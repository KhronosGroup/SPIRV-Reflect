#!/bin/sh

glslangValidator.exe -V -D -S frag -e main -o sample.spv sample.hlsl
bin2c -i sample.spv -o tmp_sample_spv_h -a k_sample_spv -l 16

echo -e "#ifndef SAMPLE_SPV_H" >  sample_spv.h
echo -e "#define SAMPLE_SPV_H" >> sample_spv.h

echo -e "" >> sample_spv.h
echo -e "/* Source from sample.hlsl" >> sample_spv.h
echo -e "" >> sample_spv.h
cat sample.hlsl >> sample_spv.h
echo -e "\n" >> sample_spv.h
echo -e "*/" >> sample_spv.h

echo -e "" >> sample_spv.h
cat tmp_sample_spv_h >> sample_spv.h
echo -e "" >> sample_spv.h

echo -e "/* SPIRV Disassembly" >> sample_spv.h
echo -e "" >> sample_spv.h
spirv-dis --raw-id sample.spv >> sample_spv.h 
echo -e "" >> sample_spv.h
echo -e "*/" >> sample_spv.h

echo -e "" >> sample_spv.h
echo -e "#endif // SAMPLE_SPV_H" >> sample_spv.h

dos2unix sample_spv.h

rm -f tmp_sample_spv_h