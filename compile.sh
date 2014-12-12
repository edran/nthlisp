#!/bin/bash
cc -std=c99 -Wall nthlisp.c mpc/mpc.c -ledit -lm -o nthlisp
