# ptsviewer version
VERSION = 0.7.5

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

GLINC  =$(shell pkg-config --cflags gl)
GLLIB  =$(shell pkg-config --libs   gl)
GLUINC =$(shell pkg-config --cflags glu)
GLULIB =$(shell pkg-config --libs   glu)
GLUTINC=
GLUTLIB=-lglut
MLIB   =-lm

# includes and libs
INCS=-I. -I/usr/X11/include/ ${GLINC} ${GLUINC} ${GLUTINC} -I /opt/local/include/eigen3 -I /Users/tomasz/libkdtree/
LIBS=-L/usr/lib ${GLLIB} ${GLULIB} ${GLUTLIB} ${MLIB}

# dirs for source and object files
OBJDIR   = obj
SRCDIR   = src

# compiler and additional flags
COMPILER = g++-mp-4.3
FLAGS    = -Wall -DVERSION=\"${VERSION}\" ${INCS} ${LIBS}
RFLAGS   = ${FLAGS} -O3 -std=c++0x
DFLAGS   = ${FLAGS} -g
