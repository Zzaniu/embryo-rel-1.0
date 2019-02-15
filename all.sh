#!bin/bash

rm -rf /tmp/gzrom.bin

make clean

make

cp embryo.bin /tmp/gzrom.bin


