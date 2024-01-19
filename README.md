# Tomasolo's Algorithm with Speculation

    This project is an implementation of Tomasulo's Algorithm with speculation. It assesses the performance of an out-of-order 16-bit processor which has 11 instructions. The language used for this project is C++.
    
    This algorithm is used for dynamic scheduling of instructions that is designed for out-of-order execution.

    This is a university project that was done by Ali Khattab and Menna Abdalla under the supervision of Professor Cherif Salama.
    
    
## Project Assumptions

These are the assumptions made while working on this project:
    - There is no virtual memory.
    - No input or output instructions are supported.
    - No interrupts or exceptions are to be handled.
    - There are 4 stages: _issue, execute, write back and commit_.
    - The processor has 8 general purpose registers, R0-R7, and R0 always contains the value 0 and cannot be changed.
    - The memory capacity is 128KB and it is word-addressable, where a word is 16 bits. 
    - Branch instructions have an always taken predictor.
    - The instructions are already in the instruction queue and have been fetched and decoded.
    - Each reservation station has a functional unit dedicated to it.
    
    
## What the simulator does

    - You can write the assmebly instruction that you want to try in the simulation in a text file. Then write the pathname of the text file in the variable called instructionsFilename. Write the contents of the data memory in another text file where the data is written followed by a comma and its address. Include its pathname in the variable called dataFilename. 
    - Once you run the program, you will get an output showing you at which cycle each instruction was issued, executed, written and committed. 
    - You also get an output of the IPC and the branch misprediction percentage.
    
    
## Issues

    Due to being short on time, the following was not done as we would've liked:
    - The return function 'ret' works in some cases and sometimes it does not.
    - In special cases when there are many instructions, not all instructions at the end of the list get executed properly.
    - Did not implement that the branch prediction would depend on offset.
    - Did not stall branch instruction properly when the register it needs is not ready.

