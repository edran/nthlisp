#!/bin/bash
cc -std=c99 -Wall error_handling.c mpc/mpc.c -ledit -lm -o error_handling
