Building Lock2: Tutorial
========================
First of all, make sure you've done a recursive clone of Lock2 to ensure all the submodules were correctly fetched.

.. code-block::

        $ git clone https://github.com/rodovia/lock2 --recursive


Now, you will have to build Limine. Downloading a pre-built binary also works. See their_ instructions on how to do both of these [1]_. Place the now built binaries, including the hosted tools, inside the **limine/** folder, which should be located in the project root (create a folder with that name over there).

You can now safely build Lock2. It is as simples as:

.. code-block::

   $ make


By default, it will try to build using the LLVM toolchain (Clang, LLD...) together with NASM.

You can switch which compiler to use for the C and C++ by overriding the `CPP` and `CXX` variables, respectively. You can also change which assembler to use, given it supports the Intel syntax (You may have to change the command line syntax).

Notice that Lock2 is written assuming that the compiler supports C++17 and the GCC compiler extensions. It may or may not work with older C++ compilers or standards.

Higher build customization (Kconfig!) is planned, but not a priority right now.

If something went wrong during the build, more than certainly bug inside the source code. You can wait until it gets fixed or fix it yourself.

.. their_: https://github.com/limine-bootloader/limine#binary-releases

.. [1] Despite the hyperlink currently going to the 5th major of Limine, you can use the 4th version. 4.x support is untested though.
