#ifndef SPIRV_REFLECT_EXAMPLES_COMMON_H
#define SPIRV_REFLECT_EXAMPLES_COMMON_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <spirv_reflect.h>

bool ReadFile(const std::string& file_path, std::vector<uint8_t>* p_buffer);

void PrintModuleInfo(std::ostream& os, const SpvReflectShaderModule& obj, const char* indent = "");
void PrintDescriptorSet(std::ostream& os, const SpvReflectDescriptorSet& obj, const char* indent = "");
void PrintDescriptorBinding(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent = "");
void PrintInterfaceVariable(std::ostream& os, SpvSourceLanguage src_lang, const SpvReflectInterfaceVariable& obj, const char* indent);

class YamlWriter {
public:
  explicit YamlWriter(const SpvReflectShaderModule& shader_module);

  void Write(std::ostream& os);
private:
  YamlWriter(const YamlWriter&) = delete;
  YamlWriter(const YamlWriter&&) = delete;
  static std::string Indent(uint32_t level) {
    return std::string(2*level, ' ');
  }
  static std::string SafeString(const char* str) { return str ? (std::string("\"") + str + "\"") : ""; }
  void WriteTypeDescription(std::ostream& os, const SpvReflectTypeDescription& td, uint32_t indent_level);
  void WriteBlockVariable(std::ostream& os, const SpvReflectBlockVariable& bv, uint32_t indent_level);
  void WriteDescriptorBinding(std::ostream& os, const SpvReflectDescriptorBinding& db, uint32_t indent_level);
  void WriteInterfaceVariable(std::ostream& os, const SpvReflectInterfaceVariable& iv, uint32_t indent_level);

  const SpvReflectShaderModule& sm_;
  std::map<const SpvReflectTypeDescription*, uint32_t> type_description_to_index_;
  std::map<const SpvReflectBlockVariable*, uint32_t> block_variable_to_index_;
  std::map<const SpvReflectDescriptorBinding*, uint32_t> descriptor_binding_to_index_;
  std::map<const SpvReflectInterfaceVariable*, uint32_t> interface_variable_to_index_;
};


#endif