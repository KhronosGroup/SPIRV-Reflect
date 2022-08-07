#!/bin/bash
# Creates multi_entrypoint.spv from multi_entrypoint.glsl and
# multi_entrypoint.spv.dis.diff
glslc -fshader-stage=comp --target-spv=spv1.5 test_orig.glsl -o test_32bit.spv
spirv-dis test_32bit.spv > test_orig.spv.dis
cp test_orig.spv.dis test_32bit.spv.dis
patch test_32bit.spv.dis test_32bit.spv.dis.patch
spirv-as --target-env spv1.5 test_32bit.spv.dis -o test_32bit.spv
cp test_orig.spv.dis test_64bit.spv.dis
patch test_64bit.spv.dis test_64bit.spv.dis.patch
spirv-as --target-env spv1.5 test_64bit.spv.dis -o test_64bit.spv
cp test_orig.spv.dis test_localsizeid.spv.dis
patch test_localsizeid.spv.dis test_localsizeid.spv.dis.patch
spirv-as --target-env spv1.5 test_localsizeid.spv.dis -o test_localsizeid.spv
glslc -fshader-stage=comp --target-spv=spv1.5 test_convert.glsl -o test_convert.spv
spirv-dis test_convert.spv > test_convert.spv.dis
patch test_convert.spv.dis test_convert.spv.dis.patch
spirv-as --target-env spv1.5 --preserve-numeric-ids test_convert.spv.dis -o test_convert.spv