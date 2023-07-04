#!/bin/bash

make clean
make FORMAT=DATA_OPS
make TARGET=ia32 clean
make TARGET=ia32 FORMAT=DATA_OPS
