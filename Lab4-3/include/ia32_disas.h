//
// Created by Kangkook Jee on 10/15/20.
//
#include <stdint.h>

#define IA32_DATA            0       // default
#define IA32_notimpl         1
#define IA32_PREFIX          2
#define IA32_2BYTE           3
#define IA32_CFLOW           4
#define IA32_DECODE_TYPE(_d) ((_d) & 0x000f)

/*
 *  The following defines add extra information on top of the decode type.
 */

#define IA32_MODRM           0x0010
#define IA32_IMM8            0x0020   // REL8 also
#define IA32_IMM32           0x0040   // REL32 also

#define IA32_RET             0x0100
#define IA32_JCC             0x0200
#define IA32_JMP             0x0400
#define IA32_CALL            0x0800

#define ESCAPE_OPCODE	     0x0f
#define INT3		     0xcc
#define JA		     0x77
#define JLE	     	     0x7e
#define JNE	     	     0x75
#define IS_PREFIX(octet) ((octet) == 0x66 || (octet) == 0xf2 || (octet) == 0xf3)
#define IA32_OPERAND_TYPE(decoded) ((decoded) & 0x00f0)
#define HAS_SIB(modrm) ((((modrm) >> 6) != 0x3) && (((modrm) & 0x7) == 0x4))
#define IA32_CFLOW_TYPE(decoded) ((decoded) & 0x0f00)

typedef struct {
	void *saddr;
	uint16_t ninstr;
} basicblk;

extern unsigned ia32DecodeTable[]; /* see below */

/*********************************************************************
 *
 *  IA32Instr
 *
 *   Decoded information about a single ia32 instruction.
 *
 *********************************************************************/

typedef struct {
   uint16_t  opcode;
   int8_t   len;
   uint8_t  modRM;
   uint32_t  imm;
} IA32Instr;
