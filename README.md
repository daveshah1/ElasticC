# ElasticC - lightweight open HLS for rapid prototyping of simple FPGA projects

**ElasticC is currently in early development. It is NOT finished or working for
any useful purpose. At the moment it is little more than a parser, simple 
evaluator and VHDL generator. It contains no timing or latency handling code
or any other transformations that would give it purpose.**

## What is ElasticC?
ElasticC is designed to make designing streaming data processing systems on
FPGAs as easy as possible, by using a C/C++ style language - while avoiding 
features of C++ or strict compatibility that makes things harder to code or 
read.

Its primary target applications are those involving a continuous stream of 
data and primarily single-cycle processing (but with few constraints on pipeline
latency), in particular real time image and signal processing.

Originally the ElasticC language was developed for augmented reality
prototyping, as demonstrated in [this video](https://www.youtube.com/watch?v=7kkcUyn5BdU).
Note that the language codebase shown in the video is a prototype version that 
is very different to what is in this repository - do not expect language features
in the video to work yet, or indeed ever in some cases.

The code examples for this older version are published on [my website](https://ds0.me/fpgasynth/index.html).
Once again, they are for a different codebase to this repository and will not work here.

You can see examples of the what the current codebase can handle in the
[tests](tests/) folder. One of the next priorities is to develop a framework to
automatically build and verify these.

## What will it be able to do?
The following features are in development or planned:
 - Support for almost all C language features (except pointers), and C++ features
   that make sense for hardware development (including templates, lambda functions,
   limited struct support, operator overloading for structs).
 - Full C++ template support (WIP: argument deduction and SFINAE not yet implemented)
 - Built-in data types for streams, including 2D moving windows (`stream2d`)
 - Variable width integer types and powerful automatic casting for optimal hardware generation
 - Support for generating a range of IO interfaces, including AXI4-Stream,
   Avalon-ST, parallel video, generic FIFO and generic parallel memory.
 - Automatic pipeline register insertion based on a timing model of the target
   FPGA, and to balance latencies of inputs or dedicated blocks such as memories.
 - Standard library containing commonly used blocks such as convolution, edge detection,
   video format conversion, signal filtering, signal generation, etc
 - VHDL and Verilog RTL back-ends (currently only VHDL supported)
 - C++ simulation output

## When will it be usable?
I am only able to work on ElasticC in my spare time. I would estimate that enough
features for it to be useful will be completed by the end of 2018, but it will
be several years before all the planned features are completed.

## Building ElasticC
Currently ElasticC is only being tested on Linux, but there is no reason it shouldn't also work 
on Windows or Mac OS X with a suitable build environment. 

ElasticC requires a modern compiler with C++17 support (Clang is recommended but g++ is also
supported), and the boost libraries. Once these are installed, you can simply run `make` to
build ElasticC. 

ElasticC includes some automated functional tests. These require a recent version of ghdl
and Python 3.5 or newer. You will need to ensure that the `PYTHON`environment variable points to
Python 3 if your distribution defaults to Python 2. Run `make test` to run the tests. 

## Contributions
Contributions are always appreciated, send an email (see my GitHub profile),
ping me (_daveshah_) on Freenode ##openfpga, or open an issue
if you would like to get involved or have any questions.

ElasticC is licensed under a MIT License.
