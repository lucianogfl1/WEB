# Project configuration

# 
# CUSTOMIZE FOR YOUR PROJECT
#

# Give your project a name, e.g.
# PROJECT = FooBar

PROJECT = runcmd

# Name your project's binaires, eg.
# bin = foo bar

# For each item in $(bin), name the objects needed to build it, e.g.
# foo_obj = main_foo.o 
# bar_obj = main_bar.o 

# For each item in $(bin), name the project's libraries to which it shall be linked, e.g.
# foo_lib = libfoobar
# bar_lib = libfoobar libm

# For each item in $(bin), name the other libraries to which it shall be linked, e.g.
# foo_lib_other = libfoobar
# bar_lib_other = libfoobar libm


# Name your project's libraries (to be installed), e.g.
# lib = libfoobar
lib = libruncmd

# For each item in $(lib), name the objects needed to build it, e.g.
# libfoobar_obj = foo.o bar.o
libruncmd_obj = runcmd.o

# For each item in $(lib), name the headers your project installs, e.g.
#libfoobar_h = foos.h
libruncmd_h = runcmd.h

# For each item in $(noinst_bin), name the libraries to which it's to be linked, e.g.
# example_lib = libfoobar
test-001_lib = libruncmd
test-002_lib = libruncmd
test-003_lib = libruncmd
test-004_lib = libruncmd

# If any targets (either binary or object) needs specific compiler flags to be
# built, list them here, e.g.
# foo_cflags = -pthread
# bar.o_cppflags = -D_POSIX_C_SOURCE
test-002_cppflags = -pthread
test-002_ldflags = -pthread


# Extra files to go in the distribution (add as needed)

EXTRA_DIST = runcmd.txt

# List the automatically generated, distributed source files of any kind.
# These files are generated in the development environment but should
# be already available in the building enviroment. E.g. config scripts.

gendist_files = debug.h Makefile

# Installation prefix

PREFIX=./local

# Customize the building environemnt (modify as desired)

CC = gcc
CPP_FLAGS = -Wall --ansi --pedantic-errors -D_POSIX_C_SOURCE=200809L 
C_FLAGS = 
LD_FLAGS = 
MAKE = make
AR = ar

# Housekeeping (name temporary files to clean)
# EXTRA_GARBAGE = *~
EXTRA_GARBAGE = *~ \#*


