#define _GNU_SOURCE

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ucontext.h>

#include "amd64_context.h"
#include "amd64_disas.h"
#include "macros.h"

/* addresses of asm callout glue code */

extern void *jccCallout;
extern void *jmpCallout;
extern void *callCallout;
extern void *retCallout;

extern uint32_t amd64DecodeTable[]; /* see below */

/* instrumentation target */

extern int user_prog(void *a);

void StartProfiling(void *func);

void StopProfiling(void);

void amd64Decode(uint8_t *ptr, AMD64Instr *instr);

void patchNextCtrlInstr(void *);

void *callTarget;

/*********************************************************************
 *
 *  callout handlers
 *
 *   These get called by asm glue routines.
 *
 *********************************************************************/

void handleJccCallout(SaveRegs regs) { NOT_IMPLEMENTED(); }

void handleJmpCallout(SaveRegs regs) { NOT_IMPLEMENTED(); }

void handleCallCallout(SaveRegs regs) { NOT_IMPLEMENTED(); }

void handleRetCallout(SaveRegs regs) { NOT_IMPLEMENTED(); }

static AMD64Instr *origInstr;
static uint8_t *origAddr;
static int32_t numCalls;

void
_sa_sigaction(int32_t sig, siginfo_t *siginfo, void *ucontext)
{
	ucontext_t *uc;
	greg_t eflags;

	uc = ucontext;

	/* Unpatch instruction */
	*origAddr = origInstr->opcode; /* unsafe */

	/* Emulate instruction */
	switch (IA_CFLOW_TYPE(amd64DecodeTable[origInstr->opcode])) {
	case IA32_JCC:
		eflags = uc->uc_mcontext.gregs[REG_EFL];
		uc->uc_mcontext.gregs[REG_RIP] = (uint64_t)origAddr + origInstr->len;
		switch (origInstr->opcode) {
		case JA:
			uc->uc_mcontext.gregs[REG_RIP] += ((eflags & 0x41) == 0) * origInstr->imm;
			break;
		case JLE:
			if ((eflags & 0x40) == 0x40 || (((eflags & 0x400) >> 10) != ((eflags & 0x80) >> 6)))
				uc->uc_mcontext.gregs[REG_RIP] += origInstr->imm;
			break;
		case JNE:
			uc->uc_mcontext.gregs[REG_RIP] += ((eflags & 0x40) == 0) * origInstr->imm;
			break;
		default:
			NOT_IMPLEMENTED();
			break;
		}
		break;
	case IA32_JMP:
		uc->uc_mcontext.gregs[REG_RIP] = (uint64_t)origAddr + origInstr->len + origInstr->imm;
		break;
	case IA32_CALL:
		uc->uc_mcontext.gregs[REG_RSP] -= sizeof(int64_t);
		/* eip points one byte ahead because of 0xcc */
		*(int64_t *)uc->uc_mcontext.gregs[REG_RSP] = uc->uc_mcontext.gregs[REG_RIP] + origInstr->len - sizeof(int8_t);
		uc->uc_mcontext.gregs[REG_RIP] = (uint64_t)origAddr + origInstr->len + origInstr->imm; /* unsafe */
		++numCalls; /* int overflow */
		break;
	case IA32_RET:
		uc->uc_mcontext.gregs[REG_RIP] = *(int64_t *)uc->uc_mcontext.gregs[REG_RSP];
		uc->uc_mcontext.gregs[REG_RSP] += sizeof(int64_t);
		--numCalls; /* int underflow */
		break;
	default:
		NOT_IMPLEMENTED();
		break;
	}

	/* Patch next control instruction */
	free(origInstr);
	origInstr = NULL;

	if (numCalls > 0)
		patchNextCtrlInstr((void *)uc->uc_mcontext.gregs[REG_RIP]);
	else
		StopProfiling();
}

static inline uint32_t
getDispSize(uint8_t modrm)
{
	switch (modrm >> 6) {
	case 0x0:
		return ((modrm & 0x7) == 0x5) ? sizeof(uint32_t) : 0;
	case 0x1:
		return sizeof(uint8_t);
	case 0x2:
		return sizeof(uint32_t);
	default:
		return 0;
	}
}

/*********************************************************************
 *
 *  amd64Decode
 *
 *   Decode an AMD64/IA32 instruction.
 *
 *********************************************************************/

void 
amd64Decode(uint8_t *ptr, AMD64Instr *instr) 
{ 
	uint32_t decoded;

	instr->len = 0;

	/* Check for mandatory prefix */
	if (IS_MAND_PREFIX(ptr[instr->len]))
		++instr->len;

	/* Check for REX prefix */
	if (IS_REX_PREFIX(ptr[instr->len]))
		++instr->len;

	/* Check for 2 byte vs 1 byte opcode */
	if (ptr[instr->len] == ESCAPE_OPCODE)
		++instr->len;

	instr->opcode = ptr[instr->len];
	++instr->len;

	decoded = amd64DecodeTable[instr->opcode];

	switch (IA_OPERAND_TYPE(decoded)) {
	case IA32_MODRM:
		instr->len += sizeof(int8_t) + HAS_SIB(ptr[instr->len]) + getDispSize(ptr[instr->len]);
		break;
	case IA32_MODRM | IA32_IMM8:
		instr->len += sizeof(int8_t) + HAS_SIB(ptr[instr->len]) + getDispSize(ptr[instr->len]);
		instr->imm = ptr[instr->len];
		instr->len += sizeof(int8_t);
		break;
	case IA32_MODRM | IA32_IMM32:
		instr->len += sizeof(int8_t) + HAS_SIB(ptr[instr->len]) + getDispSize(ptr[instr->len]);
		instr->imm = *(uint32_t *)(ptr + instr->len);
		instr->len += sizeof(int32_t);
		break;
	case IA32_IMM8:
		instr->imm = (int8_t)ptr[instr->len];
		instr->len += sizeof(int8_t);
		break;
	case IA32_IMM32:
		instr->imm = *(uint32_t *)(ptr + instr->len);
		instr->len += sizeof(int32_t);
		break;
	case IA32_DATA:
		if (IA32_DECODE_TYPE(decoded) == IA32_notimpl) {
			switch (instr->opcode) {
			case 0xa1:
			case 0xa3:
				/* Assume double word operand */
				instr->imm = *(uint32_t *)(ptr + instr->len);
				instr->len += sizeof(int32_t);
				break;
			case 0xd1:
			case 0xff:
				instr->len += sizeof(int8_t) + HAS_SIB(ptr[instr->len]) + getDispSize(ptr[instr->len]);
				break;
			default:
				NOT_IMPLEMENTED();
				break;
			}
		}
		break;
	default:
		NOT_IMPLEMENTED();
		break;
	}

	printf("addr %p, opcode: %x, len: %d, isCFlow: %s\n", 
		(void *)ptr, instr->opcode, instr->len, 
		(decoded & IA32_CFLOW) ? "true" : "false");
}

void
patchNextCtrlInstr(void *addr)
{
	uint8_t *ptr;
    AMD64Instr *instr;
    basicblk b;

    b.ninstr = 0;
    b.saddr = addr;
	instr = malloc(sizeof(AMD64Instr));

	for (ptr = (uint8_t *)addr; ; ptr += instr->len) {
		amd64Decode(ptr, instr);
		++b.ninstr;
		if (IA32_DECODE_TYPE(amd64DecodeTable[instr->opcode]) == IA32_CFLOW)
			break;
	}

    printf("Block start address: %p, # of instructions: %d\n", b.saddr, b.ninstr);

	/* Save original instruction and address */
	origAddr = ptr;
	origInstr = instr;

	/* Patch instruction */
	*ptr = INT3;
}

/*********************************************************************
 *
 *  StartProfiling, StopProfiling
 *
 *   Profiling hooks. This is your place to inspect and modify the profiled
 *   function.
 *
 *********************************************************************/

void StartProfiling(void *func) 
{
	struct sigaction act;
	
	act.sa_sigaction = &_sa_sigaction;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGTRAP, &act, NULL))
		err(1, "sigaction");

	++numCalls; /* numCalls begins at 1 */
	patchNextCtrlInstr(func);
}

void StopProfiling(void) 
{
	struct sigaction act;
	
	act.sa_handler = SIG_DFL;

	if (sigaction(SIGTRAP, &act, NULL))
		err(1, "sigaction");
}

int main(int argc, char *argv[]) {
  int value;
  char *end;

  char buf[16];

  if (argc != 1) {
    fprintf(stderr, "usage: %s\n", argv[0]);
    exit(1);
  }

#ifdef __FIB__
  printf("running fib()\n");
#endif

#ifdef __FIBP__
  printf("running fibp()\n");
#endif

#ifdef __PRIME__
  printf("running isPrime()\n");
#endif

  printf("input number: ");
  scanf("%15s", buf);

  value = strtol(buf, &end, 10);

  if (((errno == ERANGE) && ((value == LONG_MAX) || (value == LONG_MIN))) ||
      ((errno != 0) && (value == 0))) {
    perror("strtol");
    exit(1);
  }

  if (end == buf) {
    fprintf(stderr, "error: %s is not an integer\n", buf);
    exit(1);
  }

  if (*end != '\0') {
    fprintf(stderr, "error: junk at end of parameter: %s\n", end);
    exit(1);
  }

  StartProfiling(user_prog);

#if defined(__FIB__) || defined(__PRIME__)
  value = user_prog((void *) value);
#else
  value = user_prog(&value);
#endif

  StopProfiling();

  printf("%d\n", value);
  exit(0);
}
