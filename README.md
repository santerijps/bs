# bs - a build system for C/C++ projects

`bs` is a zero-dependency "build system" which only requires a C/C++ compiler to work.
First you compile the build tool and then use the build tool to build your program.

## Getting started

1. Download [bs.h](bs.h) and [bs.c](bs.c) into your project directory
2. Configure [bs.h](bs.h)
    - The macros that start with `BS_` can be edited (don't edit the ones starting with `_BS_`)
    - At least check that the compiler is correct (defaults to `gcc`)
    - For cross compatible file paths, use the `_BS_SEP` macro (e.g. `"path" _BS_SEP "to" _BS_SEP "file"`)
    - **Note:** The output executable name shouldn't include a file extension!
3. Compile the build program
    - For example: `gcc -O3 -s -Wall -Wextra -Werror -pedantic -march=native bs.c -o bs.exe`
4. Run the build program to build and run your program

## Usage

Build your program by simply running `bs`:

```shell
$ bs
$ bs -v     # verbose
$ bs -R     # release build
$ bs -vR    # flags can be combined
```

To run the program after building it successfully:

```shell
$ bs -r         # run after build
$ bs -Rr        # release build and run
$ bs -r foo bar # passing arguments to the program
```

Read the full usage description by running `bs -h`.
