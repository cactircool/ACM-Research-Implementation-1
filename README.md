# ACM Research Implementation 1

First install cmake and the Ninja build tool either manually or by running:
```bash
./init.sh # only works on linux/macos systems
```

Then you can run the program by running:
```bash
./run.sh [PROGRAM_TO_BREAK] (secretName secret)*
```
[] = required

()* = if included, must be included in the format inside the parenthesis

## Purpose

This is an implementation of the proposed code execution path in this research paper: https://theory.stanford.edu/~barrett/pubs/FSB+19.pdf

This implementation aims to uphold the Unique Execution Principle which follows closely from pure functional programming. Essentially, by first declaring a set of secrets that are meant to be both cryptographically secure and not leaked, this program can then fuzz test and repeatedly step through the program to analyze any micro-architectural or architectural variables that may have ended up storing any secret.