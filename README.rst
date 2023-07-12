Lock2
=====
Lock2 is an simple kernel written in C++. It is still in heavy development.

Currently, it only supports the x86_64 architecture. I don't expect to support any other arches because dealing only with AMD's architecture is already a pain in the ass.

Build instructions
==================
.. code-block::

        make

Is enough. By default, it will try to build using the LLVM toolchain (Clang, LLD...) together with NASM.

You can switch which compiler to use for the C and C++ by overriding the `CPP` and `CXX` variables, respectively. You can also change which assembler to use, given it supports the Intel syntax (You may have to change the command line syntax).

Notice that Lock2 is written assuming that the compiler supports C++17. It may or may not work with older C++ compilers or standards.

Higher build customization (Kconfig!) is planned, but not a priority right now.

Licensing
=========
Your code are belong to us. Lock2 is licensed under the BSD 3-Clause Licence.
Lock2 uses:

        * ACPICA, which is under the BSD licence.

        * Scalable Screen Font 2, which is under the MIT licence

