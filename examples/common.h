#ifndef SPIRV_REFLECT_EXAMPLES_COMMON_H
#define SPIRV_REFLECT_EXAMPLES_COMMON_H

#include <iostream>
#include <string>
#include <vector>
#include <spirv_reflect.h>

bool ReadFile(const std::string& file_path, std::vector<uint8_t>* p_buffer);

void PrintModuleInfo(std::ostream& os, const SpvReflectShaderModule& obj, const char* indent = "");
void PrintDescriptorSet(std::ostream& os, const SpvReflectDescriptorSet& obj, const char* indent = "");
void PrintDescriptorBinding(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent = "");
void PrintInterfaceVariable(std::ostream& os, SpvSourceLanguage src_lang, const SpvReflectInterfaceVariable& obj, const char* indent);

#endif