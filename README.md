# Hepler for Chime

This is a helper repository for [Chime](https://github.com/yatorho/CHIME) where we implement some useful functions and libraries for `Chime`.

## Environment

To use this repository, you need to install `Bazel` at first. Please refer to [Bazel](https://bazel.build/) for more information. The version of bazel we use is `3.1.0`.

## Hello World

This is a simple example to show remote repository usage. We use `Chime` remotely to implement a simple `Hello World` program.

Firstly, you should clone this repository to your local machine and enter the directory.
```bash
git clone https://github.com/yatorho/CHIME.git
```
  
Then, you can run the following command to build  and run the `Hello World` program.
```bash
bazel run //:hello_chime
```
You will see the following output:
```bash
INFO: Analyzed target //:hello_chime (0 packages loaded, 0 targets configured).
INFO: Found 1 target...
Target //:hello_chime up-to-date:
  bazel-bin/hello_chime
INFO: Elapsed time: 0.100s, Critical Path: 0.00s
INFO: 0 processes.
INFO: Build completed successfully, 1 total action
INFO: Build completed successfully, 1 total action
Hello Chime!
```
Congratulations! You have successfully run a `Hello World` program with `Chime`!
