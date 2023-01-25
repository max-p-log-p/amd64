# CS6332: Lab 4 - AMD64 Binary Translator

**NOTE:** You will connect to and use `ctf-vm1.utdallas.edu` for this assignment. You can either directly connect to the host from UTD network or `ssh <netid>@syssec.run -p 2201` from the public network. 

In this homework you will port and extend the [ARM Binary Translator][binary translator] to run from Intel architecture. The main focus is in three folds. We want to learn (1) how to apply the same technique to different (or more complex) platform to apply the same principles of software virtualization and instrumentation techniques, (2) the challenges in disassembling machine code written in Intel architecture (i386/AMD64), (3) inline code transformation leveraging the binary translator. 

Again, the homework provides [skeleton codes] to implement a minimal binary translator for Intel architecture. In summary, your implementation will (1) Intel instruction decoder decode instruction, (2) patch the branch (or control) instruction to switch into callout context, (3) profile program execution at runtime, and (4) apply an optimization.

Although the assignment is tested and verified for GCC compilers in ctf-vm1 (version 5.4), the other environmental factors may affect compilers to generate different machine instruction sequences, resulting in runtime failures. Please feel free to consult the instructor or TA regarding such issues.

## Preliminaries

Again, you will begin the lab by building and running the provided skeleton code. Then, you will incrementally extend it in the course of completing each part. Two recursive functions with no [side effects] ([Fibonacci] and [isPrime][isPrime.c]), written in plain C, are provided to test your binary translator. Fibonacci comes in with two different versions, which implement recursion (fib.c) and iteration (fibp.c). 

Both functions have signatures that take a single integer argument, return an integer with no side effects. Inside the main driver code ([lab4.c]), all functions are alias as `int64_t user_prog(void *)`. While all functions take a single argument, we need to support two different ways to pass the argument(s). [fib.c] takes an integer argument and returns an integer value. [fibp.c] takes input in pointer type to support variable-length arguments. We use the `void*` type to indicate generic input types.


To test each function, first, copy the template code located in `/home/lab4/lab4.zip` from your ctf-vm1 and run the following.

```bash
# Fibonacci functions.
$ make fib
$ ./lab4_fib

$ make fibp
$ ./lab4_fibp

$ make prime
$ ./lab4_prime
```

### Submission guideline.

This assignment will be due at 11:59 PM, ~~Nov 27~~ Dec 5. To submit the assignment, you will extend the provided skeleton codes to implement a solution for each part. Please do not forget to write a README.md document to explain your code and
solution.

```
<netid>-lab04/
├── Lab4-1/
├── Lab4-2/
├── Lab4-3/
├── Lab4-4/
└── README.md
```


### Academic honesty

The submission and code should be your own. Although strongly encouraging discussions among students and with the instructor or TA, the class will not tolerate students' failure to comply with [the Student code of conduct] or any unethical behaviors. The submitted code should be of your own. We will run code similarity measurement tools against all student submissions.
    
---

## Lab4-1: i386 Instruction Decode (30 pt)

The goal of this step is to use *[StartProfiling()]* to decode a block of instructions. You need only to decode enough of each instruction to determine its length. By doing this you should be able to decode a block of instructions of arbitrary length. *[StartProfiling()]* should print the address, opcode, and length of instructions for *[user_prog()]* until it encounters `ret` (0xc9) instruction.

Due to the complexity of Intel ISA being CISC, the core challenge for the part is to get the right length for each instruction. On the other hand, it is simpler than ARM architecture as it has limited set of instructions which make control (branch) operations. To help you on this, we provide IA32 opcode map in *[ia32DecodeTable]*. Use it as you see fit. Or if you find any instruction not covered by the map, please feel free to update the table.  
    
The following is the sample output. 

```
input number: 10
addr 0x8049310, opcode: 55, len: 1, isCFlow: false
addr 0x8049311, opcode: 89, len: 2, isCFlow: false
addr 0x8049313, opcode: 57, len: 1, isCFlow: false
addr 0x8049314, opcode: 56, len: 1, isCFlow: false
addr 0x8049315, opcode: 53, len: 1, isCFlow: false
addr 0x8049316, opcode: 51, len: 1, isCFlow: false
addr 0x8049317, opcode: 8d, len: 3, isCFlow: false
addr 0x804931a, opcode: 89, len: 2, isCFlow: false
addr 0x804931c, opcode: 83, len: 3, isCFlow: false
addr 0x804931f, opcode: 77, len: 2, isCFlow: true
addr 0x8049321, opcode: 8b, len: 2, isCFlow: false
addr 0x8049323, opcode: ba, len: 5, isCFlow: false
addr 0x8049328, opcode: eb, len: 2, isCFlow: true
addr 0x804932a, opcode: 8b, len: 2, isCFlow: false
addr 0x804932c, opcode: 83, len: 3, isCFlow: false
addr 0x804932f, opcode: 50, len: 1, isCFlow: false
addr 0x8049330, opcode: e8, len: 5, isCFlow: true
addr 0x8049335, opcode: 83, len: 3, isCFlow: false
addr 0x8049338, opcode: 89, len: 2, isCFlow: false
addr 0x804933a, opcode: 89, len: 2, isCFlow: false
addr 0x804933c, opcode: 8b, len: 2, isCFlow: false
addr 0x804933e, opcode: 83, len: 3, isCFlow: false
addr 0x8049341, opcode: 50, len: 1, isCFlow: false
addr 0x8049342, opcode: e8, len: 5, isCFlow: true
addr 0x8049347, opcode: 83, len: 3, isCFlow: false
addr 0x804934a, opcode: 1, len: 2, isCFlow: false
addr 0x804934c, opcode: 11, len: 2, isCFlow: false
addr 0x804934e, opcode: 8d, len: 3, isCFlow: false
addr 0x8049351, opcode: 59, len: 1, isCFlow: false
addr 0x8049352, opcode: 5b, len: 1, isCFlow: false
addr 0x8049353, opcode: 5e, len: 1, isCFlow: false
addr 0x8049354, opcode: 5f, len: 1, isCFlow: false
addr 0x8049355, opcode: 5d, len: 1, isCFlow: false
addr 0x8049356, opcode: c3, len: 1, isCFlow: true
```

## Lab4-2: Control flow following and program profiling (40 pt)
    
    
You should now have the tools to identify the control (or branch) instructions and follow the control flow of IA32 architecture. With this, you will extend [lab4.c] to implement the same binary patching / unpatching operations you did for the previous lab. Again, decode the instructions to be executed until you hit a control flow instruction. Binary patch that instruction to call you instead of doing the control flow. You can then return to the code knowing that you will be called before execution passes that point. When your handler is called, unpatch the
instruction, emulate its behavior, and binary patch the end of the following basic block. For each basic block you encounter, dump the instructions in that block in the same format as Lab3-3. You should stop this process when you hit the StopProfiling() function.
Create a data structure to capture the start address of each basic block executed and the number of instructions. Run target program  (user_prog()) with different inputs and check the number of instructions (and basic blocks) executions.

## Lab4-3: Memoizer (30 pt)

As you have seen from the profiling result (Lab4-2), the runtime cost of the recursive program (fib.c) exponentially grows as the input increase. To improve the runtime performance, you will optimize the runtime performance by extending the binary patcher by reducing the number of overlapping computations. We will implement the [memoization] by keeping track of the input argument and return value pairs.

To implement, you need to extend both Call and Ret handlers to observe input and ret values and associate them. For each function call, you will first check the global cache area for the stored result from the previous runs. If found, you will immediately return the cached value, or you will proceed to run the function body.

**NOTE**: This assignment is for [fib.c] and [isPrime.c] not for [fibp.c].

## Lab4-4: (Extra Credit - 30pt) AMD64 Binary Translator

As for an extra assignment,  you will extend the binary translator implementation (Lab4-3) to run code to AMD64 functions. If you have finished all previous and are ready for the extra credit assignment, please contact the instructor. 

**NOTE:** This part is for [fib.c] and [isPrime.c], not for [fibp.c].

## Resources

You may find this reference helpful for PC assembly language programming. You will need the [Intel IA32 manuals] for exact instruction formats and decoding rules. You can find them here:
* [Volume 1]
* [Volume 2]

    
[Lab3]:https://gitlab.syssec.org/utd-classes/CS6332.001-f21-Lab3
[Lab4]:https://gitlab.syssec.org/utd-classes/CS6332.001-f21-Lab4

[binary translator]:https://dl.acm.org/doi/10.1145/3321705.3329819
[skeleton codes]:https://files.syssec.org/lab4.zip
[side effects]:https://en.wikipedia.org/wiki/Side_effect_(computer_science)
[Fibonacci]:https://en.wikipedia.org/wiki/Fibonacci_number
[code]:https://theory.stanford.edu/~aiken/moss/
[similarity]:https://en.wikipedia.org/wiki/Content_similarity_detection
[measurement]:https://github.com/genchang1234/How-to-cheat-in-computer-science-101
[tools]:https://ieeexplore.ieee.org/document/5286623
[optimized]:https://www.sciencedirect.com/topics/computer-science/conditional-execution
[inline monitors]:https://files.syssec.org/0907-mm.pdf

[the Student code of conduct]:https://policy.utdallas.edu/utdsp5003

[Intel IA32 manuals]:https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
[Volume 1]:https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-1-manual.pdf
[Volume 2]:https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf

[lab4.c]:https://gitlab.syssec.org/utd-classes/cs6332.001-f21-lab4/-/blob/main/lab4.c
[fibp.c]:https://gitlab.syssec.org/utd-classes/cs6332.001-f21-lab4/-/blob/main/fibp.c
[isPrime.c]:https://gitlab.syssec.org/utd-classes/cs6332.001-f21-lab4/-/blob/main/isPrime.c
[fib.c]:https://gitlab.syssec.org/utd-classes/cs6332.001-f21-lab4/-/blob/main/fib.c
[user_prog()]:https://gitlab.syssec.org/utd-classes/cs6332.001-f21-lab4/-/blob/main/lab4.c#L21
[StartProfiling()]:https://gitlab.syssec.org/utd-classes/cs6332.001-f21-lab4/-/blob/main/lab4.c#L84
[ia32DecodeTable]:https://gitlab.syssec.org/utd-classes/cs6332.001-f21-lab4/-/blob/main/ia32_disas.c
[memoization]:https://en.wikipedia.org/wiki/Memoization
