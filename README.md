# bs - a build system

**bs** is a "*build system*" that only requires a C compiler to work.
Thus, it's a replacement for **Make**.

The idea is simple: first you build the build system, then you build your program.

## Getting started

Below is one way you may want to use **bs**:

```sh
# 1. Go to your project repository.
cd my-repository

# 2. Copy the needed files: bs.c, config.h, and an appropriate install script.
# Alternatively, clone the bs repository.
git clone https://github.com/santerijps/bs.git

# 3. Edit bs/config.h to match your needs.
edit bs/config.h

# 4. Install bs.
bs/install

# 5. Build your program.
bs/build
```

## Usage

Below usage information can be read by running `bs/build -h`:

```text
[bs] usage: build [-hdrRv]
        -r     Run the program after compiling.
        -R     Compile a release build.
        -v     Print the generated commands to STDERR.
        -w     Watch mode.
```
