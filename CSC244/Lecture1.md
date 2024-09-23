# Lecture 1

## Basic functions of computers
The basic functions of a computer are inputting data, outputting data, processing data, and storing data. Data, data, data, data.

## The components of a computer
The main components of a computer include the memory, the input devices, the output devices, the datapath, and the control.

### Memory
The memory stores the instructions and data for a given program.

### Input devices
Input devices, such as computer mice, keyboards, microphones, MIDI shenannigans, and so on, write data to the memory.

## Output devices
Output devices, such as displays, printers, speakers, and other stuff like that, read data from the memory (and, presumably, share that data with the user).

## Datapath
Datapath processes data via a collection of functional units, such as ALU (arithmetic logic units), registers, internal buses, and so on.

## Control
The control sends signals to determine the operations of the datapath, memory, input devices, and output devices.


## Abstraction layers
Abstraction layers are, to put things simply, degrees of seperation between the lowest level of computing and where the user is.

## Levels of Program Code
There are three major levels of programming. Some would include a fourth, but not my professor.
1. High-level programming language - Including C (WHAT), C++, Java, Visual Basic, and so on.
2. Assembly languages - "A symbolic representation of machine instructions."
3. Machine languages - The binary representations of machine instructions.


## Important Ideas/Concepts in Computer Architecture:
1. Moore's Law: Integrated circuit resources double every 18-24 months. Computer architects must think ahead and write code for where technology will be, rather than where it is now.
2. Abstractions are useful for simplifying design. Abstractions allow for us to write and run code without needing to know the low-level details.
3. Make the "common case" more efficient. This is usually better for improving performance than improving the rare case. The common case is often simpler, so it should be easier to improve.
3. Parallelism allows performance to be drastically improved.
4. Performance can be improved by dividing instructions into sequential steps.
5. Assuming that a poor prediction wouldn't cause too much trouble (or cost too much), and your prediction is fairly accurate, it is often better to guess and start working, rather than waiting until you know for sure.
6. Each memory/storage device has its own cost, performance, and capacity. Memory hierarchy is like a pyramid, with the fastest and smallest (and most expensive) memory per bit at the top, and the slowest, largest, and cheapest per bit at the bottom.
7. Efficiency is important, but dependability (reliability) is equally, if not more important. Computers often use redundant components, so that a failure in one area won't throw the system into an unreliable state.
