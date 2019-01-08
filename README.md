# mitiGate: quick benchmark to test mitigations.

Mitigations often stop an attacker from gaining useful primitives such as
arbitrary code execution. This benchmark tests common mitigations and compares
their power.

The current version focuses on forward-edge CFI and enables quick evaluation of
LLVM-CFI and RAP.

The forward edge is tested for indirect calls in C, focusing on function
pointers stored in arrays or struct and compromised through several angles.


## LLVM-CFI

LLVM-CFI is a function-prototype-based CFI that protects programs against
corruption of function pointers. LLVM-CFI restricts targets to address-taken
functions of the same prototype.

Compile the benchmark with `make -f Makefile.llvmcfi`. Run it with
`./test.llvmcfi`.


## RAP

See the [note in the RAP directory](./RAP/README.md) on how to compile RAP or
use the precompiled shared library for GCC 6.3.0. RAP restricts targets to
functions of the same prototype.

Compile the benchmark with `make -f Makefile.gccrap`. Run it with
`./test.gccrap`.


## Evaluation

We have evaluated LLVM-CFI and RAP for C programs in a 
[blog post](https://nebelwelt.net/blog/20181226-CFIeval.html) with some more
details about the two CFI implementations.

