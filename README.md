LLVM Pass for `printf()` Call Logging
=====================================

This pass was created as a project for the [Type Systems and Static Analysis](https://www.csd.uoc.gr/~hy546) course and is no longer maintained.

In case you are interested, you can take a look at the [project report](doc/ProjectReport.pdf).

---

**Built and tested against LLVM 7.0.1 on macOS Mojave 10.14.3 and Ubuntu 18.04 LTS.**

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Instructions](#instructions)
    - [Configuring](#configuring)
    - [Building LLVM](#building-llvm)
    - [Building Pass](#building-pass)
    - [Building & Running Tests](#building-&-running-tests)
    - [Cleaning Up](#cleaning-up)
    - [Building & Running Everything](#building-&-running-everything)
- [Adding a Test](#adding-a-test)
- [Getting the Results](#getting-the-results)
- [License](#license)
- [Contact](#contact)


<a name="prerequisites"></a>
## Prerequisites

- LLVM source code
- Clang
- CMake
- Zsh (for executing the scripts)


<a name="instructions"></a>
## Instructions

<a name="configuring"></a>
### Configuring

In `scripts/configuration.zsh`:

1. Point `LLVM_SOURCE_DIR` to the LLVM codebase.
2. Point `LLVM_BUILD_DIR` to the directory LLVM should be built in.
3. Point `CLANG` to the Clang executable that should be used to compile the test files.
4. Set `GENERATOR` to your generator of choice, e.g. `Ninja` or `Xcode`.


<a name="building-llvm"></a>
### Building LLVM

    $ cd scripts
    $ ./build_llvm.zsh


<a name="building-pass"></a>
### Building Pass

    $ cd scripts
    $ ./build_pass.zsh


<a name="building-&-running-tests"></a>
### Building & Running Tests

    $ cd scripts
    $ ./run_test.zsh


<a name="cleaning-up"></a>
### Cleaning Up

    $ cd scripts
    $ ./clean_up.zsh


<a name="building-&-running-everything"></a>
### Building & Running Everything

You can run all of the above mentioned steps at once using the following script:

    $ ./build_and_run.zsh


<a name="adding-a-test"></a>
## Adding a Test

Create a test file `<TEST_NAME>.cpp` and place it under `test`. Build and run the test using either:

    $ cd scripts
    $ ./run_test.zsh <TEST_NAME>

or

    $ ./build_and_run.zsh <TEST_NAME>

If no test name is provided, defaults to `hello`.

**NOTE:** The working directory of the test file will be `logs`.


<a name="getting-the-results"></a>
## Getting the Results

All logging happens in `logs/log.txt`.


<a name="license"></a>
## License

Licensed under the [Mozilla Public License 2.0](LICENSE).


<a name="contact"></a>
## Contact

Vangelis Tsiatsianas - [contact@vangelists.com](mailto:contact@vangelists.com?subject=[GitHub]%20LLVM%20Pass%20for%20printf()%20Call%20Logging)
