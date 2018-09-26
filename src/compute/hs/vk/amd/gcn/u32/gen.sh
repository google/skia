#!/bin/bash

##
## Copyright 2018 Google Inc.
##
## Use of this source code is governed by a BSD-style license that can
## be found in the LICENSE file.
##

##
## exit on error
##

## set -e

##
## delete the previous images
##

rm *.comp
rm *.spv
rm *.xxd

##
##
##

HS_GEN=../../../../gen/hs_gen

## --- 32-bit keys ---

$HS_GEN -v -a "glsl" -D HS_AMD_GCN -t 1 -w 64 -r 16 -s 32768 -S 32768 -b 16 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z

##
## remove trailing whitespace from generated files
##

sed -i 's/[[:space:]]*$//' hs_config.h
sed -i 's/[[:space:]]*$//' hs_modules.h

##
##
##

whereis glslangValidator

##
## FIXME -- convert this to a bash script
##
## Note that we can use xargs instead of the cmd for/do
##

for f in *.comp
do
    dos2unix $f
    clang-format -style=Mozilla -i $f
    cpp -P -I ../.. -I ../../.. $f > ${f%%.*}.pre.comp
    clang-format -style=Mozilla -i ${f%%.*}.pre.comp
    glslangValidator --target-env vulkan1.1 -o ${f%%.*}.spv ${f%%.*}.pre.comp
    spirv-opt -O ${f%%.*}.spv -o ${f%%.*}.spv
##  spirv-remap -v --do-everything --input %%~nf.spv --output remap
    xxd -i < ${f%%.*}.spv > ${f%%.*}.spv.xxd
    len=$(wc -c < ${f%%.*}.spv)
    echo ${f%%.*}.spv $len
    printf "%.8x" $len | xxd -r -p | xxd -i > ${f%%.*}.len.xxd
done

##
## dump a binary
##

cc -I ../../.. -I ../../../../.. -D=HS_DUMP -o hs_dump *.c
hs_dump

##
## delete temporary files
##

rm *.pre.comp
rm *.comp
rm *.spv
