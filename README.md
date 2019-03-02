#  LMD (LaTeX-Markdown) Compiler
All the people who have used LaTeX know it can be cumbersome and long to write; similarly, everyone who's used markdown knows that it is very easy to read and fast to type.
The objective of this project was to transform files formatted in markdown into LaTeX files (and then possibly run `pdflatex` to get a pdf).

To be exact, the name of this repo should rather be *lmdConverter* than *lmdCompiler*.

Note that transforming markdown files into pdf files has already been done by other people, as well as interpreting markdown within LaTeX documents. The point here was not however to have a final pdf or to use markdown inside LaTeX files but rather to have a way to quickly create a LaTeX file with some content. This file could then be used as the basis for some documents that would need more advanced LaTeX features.
In other words, this project allows to quickly create the skeleton of a LaTeX file to be filled with more content later on.

*Languages used:*
- *C*
- *makefile (to automate compilation and installation)*

## Collaborators
This is a personal project I worked on alone.

## What I learned
- Open and read files in C, and more generally deal with character arrays in C
- Automate an installation process

## Files worth checking out
- Main code of the converter: [compiler.c](https://github.com/SimGus/lmdCompiler/blob/master/compiler.c)
- A very simple implementation of a linked stack: [pile.c](https://github.com/SimGus/lmdCompiler/blob/master/pile.c)
- Entry point of the program, which interprets the arguments and runs the rest of the code: [main.c](https://github.com/SimGus/lmdCompiler/blob/master/main.c)

## Compilation and execution
Compile the project:
```sh
make exe
```

Install (if you want to use it without needing to be in this directory, and if you're using Linux):
```sh
make install
```

Run the project:
```sh
./lmd [<options>] <path-to-markdown-file>
```

Simply run `./lmd` to have the list of options printed.
