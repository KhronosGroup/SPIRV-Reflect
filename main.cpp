/*
 Copyright 2017 Google Inc.
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#if defined(WIN32)
  #define _CRTDBG_MAP_ALLOC  
  #include <stdlib.h>
  #include <crtdbg.h>
#endif

#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "spirv_reflect.h"
#include "examples/common.h"

struct TextLine {
  std::vector<std::string>  text_elements;
};

// =================================================================================================
// Stream Output
// =================================================================================================
const char* ToStringVkDescriptorType(VkDescriptorType value) {
  switch (value) {
    case VK_DESCRIPTOR_TYPE_SAMPLER                : return "VK_DESCRIPTOR_TYPE_SAMPLER";
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE          : return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE          : return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   : return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   : return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       : return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";

    case VK_DESCRIPTOR_TYPE_RANGE_SIZE:
    case VK_DESCRIPTOR_TYPE_MAX_ENUM:
      break;
  }
  return "";
}

const char* ToStringType(const SpvReflectTypeDescription& /*type*/)
{
/*
  uint32_t masked = type.type_flags & SPV_REFLECT_TYPE_FLAG_COMPOSITE_MASK;
  switch (masked) {
    case SPV_REFLECT_TYPE_FLAG_COMPOSITE_SCALAR: {
      switch (type.scalar_traits.component_type) {
        case SPV_REFLECT_COMPONENT_TYPE_FLOAT: return "float"; break;
        case SPV_REFLECT_COMPONENT_TYPE_INT: return (type.scalar_traits.signedness ? "int" : "uint"); break;
      }
    } break;
    case (SPV_REFLECT_TYPE_FLAG_COMPOSITE_VECTOR | SPV_REFLECT_TYPE_FLAG_COMPOSITE_SCALAR): {
      switch (type.scalar_traits.component_type) {
        case SPV_REFLECT_COMPONENT_TYPE_FLOAT: {
          switch (type.vector_traits.component_count) {
            case 2: return "vec2"; break;
            case 3: return "vec3"; break;
            case 4: return "vec4"; break;
          }
        } break;
        case SPV_REFLECT_COMPONENT_TYPE_INT: {
          switch (type.vector_traits.component_count) {
            case 2: return (type.scalar_traits.signedness ? "ivec2" : "uvec2"); break;
            case 3: return (type.scalar_traits.signedness ? "ivec3" : "uvec3"); break;
            case 4: return (type.scalar_traits.signedness ? "ivec4" : "uvec4"); break;
          }
        } break;
      }
    } break;
  }
*/
  return "";
}

void ParseBlockMembersToTextLines(const char* indent, int indent_depth, uint32_t member_count, const SpvReflectBlockVariable* p_members, std::vector<TextLine>* p_text_lines)
{
  const char* t = indent;
  for (uint32_t member_index = 0; member_index < member_count; ++member_index) {
    std::stringstream ss;
    for (int indent_count = 0; indent_count < indent_depth; ++indent_count) {
      ss << t;
    }
    std::string expanded_indent = ss.str();

    const auto& member = p_members[member_index];
    if (member.member_count > 0) {
      TextLine text_line;
      text_line.text_elements.push_back(expanded_indent);
      text_line.text_elements.push_back("struct ");
      text_line.text_elements.push_back(member.type_description->type_name);
      text_line.text_elements.push_back(" {");
      text_line.text_elements.push_back(std::to_string(member.offset));
      text_line.text_elements.push_back(std::to_string(member.size));
      p_text_lines->push_back(text_line);
      ParseBlockMembersToTextLines(t, indent_depth + 1, member.member_count, member.members, p_text_lines);
      text_line = TextLine();
      text_line.text_elements.push_back(expanded_indent);
      text_line.text_elements.push_back("} " + std::string(member.name) + ";");
      p_text_lines->push_back(text_line);
    }
    else {
      TextLine text_line;
      text_line.text_elements.push_back(expanded_indent);
      text_line.text_elements.push_back(member.name + std::string(";"));
      text_line.text_elements.push_back(std::to_string(member.offset));
      text_line.text_elements.push_back(std::to_string(member.size));
      p_text_lines->push_back(text_line);
    }
  }
}

void StreamWrite(std::ostream& os, const char* indent, const std::vector<TextLine>& text_lines)
{
  for (auto& text_line : text_lines) {
    os << indent;
    for (auto& elem : text_line.text_elements) {
      os << elem;
    }
    os << "\n";
  }
}

void StreamWrite(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent = "")
{
  const char* t = indent;
  os << t << "binding : " << obj.binding << "\n";
  if (write_set) {
    os << t << "set     : " << obj.set << "\n";
  }
  os << t << "type    : " << ToStringVkDescriptorType(obj.descriptor_type) << "\n";
  
  // array
  if (obj.array.dims_count > 0) {  
    os << t << "array   : ";
    for (uint32_t dim_index = 0; dim_index < obj.array.dims_count; ++dim_index) {
      os << "[" << obj.array.dims[dim_index] << "]";
    }
    os << "\n";
  }

  // counter
  if (obj.uav_counter_binding != nullptr) {
    os << t << "counter : ";
    os << "(";
    os << "set=" << obj.uav_counter_binding->set << ", ";
    os << "binding=" << obj.uav_counter_binding->binding << ", ";
    os << "name=" << obj.uav_counter_binding->name;
    os << ");";
    os << "\n";
  }

  os << t << "name    : " << obj.name;
  if ((obj.type_description->type_name != nullptr) && (strlen(obj.type_description->type_name) > 0)) {
    os << " " << "(" << obj.type_description->type_name << ")";
  }

  if (obj.descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
    std::vector<TextLine> text_lines;
    ParseBlockMembersToTextLines("  ", 1, obj.block.member_count, obj.block.members, &text_lines);
    if (!text_lines.empty()) {
      os << "\n";
      os << t << "struct" << " " << obj.type_description->type_name << " " << "{" << "\n";
      StreamWrite(os, t, text_lines);
      os << t << "};";
    }
  }
}

/*
void StreamWrite(std::ostream& os, const SpvReflectDescriptorSet& obj, const char* indent = "")
{
  const char* t = indent;
  os << t << "set           : " << obj.set << "\n";
  os << t << "binding count : " << obj.binding_count;
}
*/

void StreamWrite(std::ostream& os, const SpvReflectInterfaceVariable& obj, const char* indent = "")
{
  const char* t = indent;
  os << t << "location  : ";
  if (obj.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
    os << "(built-in)";
  }
  else {
    os << obj.location;
  }
  os << "\n";
  os << t << "type      : " << ToStringType(*obj.type_description) << "\n";
  os << t << "qualifier : ";
  if (obj.decoration_flags & SPV_REFLECT_DECORATION_FLAT) {
    os << "flat";
  }
  else   if (obj.decoration_flags & SPV_REFLECT_DECORATION_NOPERSPECTIVE) {
    os << "noperspective";
  }
}

void StreamWrite(std::ostream& os, const SpvReflectShaderModule& obj, const char* indent = "")
{
  (void)indent;
  os << "entry point     : " << obj.entry_point_name << "\n";
  os << "source lang     : " << spvReflectSourceLanguage(obj.source_language) << "\n";
  os << "source lang ver : " << obj.source_language_version;
}

std::ostream& operator<<(std::ostream& os, const SpvReflectDescriptorBinding& obj)
{
  StreamWrite(os, obj, true, "  ");
  return os;
}

std::ostream& operator<<(std::ostream& os, const SpvReflectDescriptorSet& obj)
{
  PrintDescriptorSet(os, obj, "  ");
  os << "\n";
  for (uint32_t i = 0; i < obj.binding_count; ++i) {
    os << "   " << i << ":"  << "\n";
    StreamWrite(os, *obj.bindings[i], false, "    ");
    if (i < (obj.binding_count - 1)) {
      os << "\n";
    }
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const SpvReflectInterfaceVariable& /*obj*/)
{
  return os;
}

std::ostream& operator<<(std::ostream& os, const spv_reflect::ShaderModule& obj)
{
  const char* t     = "  ";
  const char* tt    = "    ";
  const char* ttt   = "      ";
  const char* tttt  = "        ";
  const char* ttttt = "          ";

  StreamWrite(os, obj.GetShaderModule(), "");

  SpvReflectResult result = SPV_REFLECT_RESULT_NOT_READY;
  uint32_t count = 0;
  std::vector<SpvReflectInterfaceVariable*> variables;
  std::vector<SpvReflectDescriptorBinding*> bindings;
  std::vector<SpvReflectDescriptorSet*> sets;

  count = 0;
  result = obj.EnumerateInputVariables(&count, nullptr);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  variables.resize(count);
  result = obj.EnumerateInputVariables(&count, variables.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  if (count > 0) {
    os << "\n";
    os << "\n";
    os << t << "input variables: " << count << "\n";
    for (size_t i = 0; i < variables.size(); ++i) {
      auto p_var = variables[i];
      assert(result == SPV_REFLECT_RESULT_SUCCESS);
      os << tt << i << ":" << "\n";
      StreamWrite(os, *p_var, ttt);
      if (i < (count - 1)) {
        os << "\n";
      }  
    }
  }

  count = 0;
  result = obj.EnumerateOutputVariables(&count, nullptr);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  variables.resize(count);
  result = obj.EnumerateOutputVariables(&count, variables.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  if (count > 0) {
    os << "\n";
    os << "\n";
    os << t << "output variables: " << count << "\n";
    for (size_t i = 0; i < variables.size(); ++i) {
      auto p_var = variables[i];
      assert(result == SPV_REFLECT_RESULT_SUCCESS);
      os << tt << i << ":" << "\n";
      StreamWrite(os, *p_var, ttt);
      if (i < (count - 1)) {
        os << "\n";
      }  
    }
  }

  count = 0;
  result = obj.EnumerateDescriptorBindings(&count, nullptr);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  bindings.resize(count);
  result = obj.EnumerateDescriptorBindings(&count, bindings.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  if (count > 0) {
    os << "\n";
    os << "\n";
    os << t << "Descriptor bindings: " << count << "\n";
    for (size_t i = 0; i < bindings.size(); ++i) {
      auto p_binding = bindings[i];
      assert(result == SPV_REFLECT_RESULT_SUCCESS);
      os << tt << i << ":" << "\n";
      StreamWrite(os, *p_binding, true, ttt);
      if (i < (count - 1)) {
        os << "\n";
      }  
    }
  }

  count = 0;
  result = obj.EnumerateDescriptorSets(&count, nullptr);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  sets.resize(count);
  result = obj.EnumerateDescriptorSets(&count, sets.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);
  if (count > 0) {
    os << "\n";
    os << "\n";
    os << t << "Descriptor sets: " << count << "\n";
    for (size_t i = 0; i < sets.size(); ++i) {
      auto p_set = sets[i];
      assert(result == SPV_REFLECT_RESULT_SUCCESS);
      os << tt << i << ":" << "\n";
      PrintDescriptorSet(os, *p_set, ttt);
      if (count > 0) {
        os << "\n";
        for (uint32_t j = 0; j < p_set->binding_count; ++j) {
          const SpvReflectDescriptorBinding& binding = *p_set->bindings[j];
          os << tttt << j << ":" << "\n";
          StreamWrite(os, binding, false, ttttt);
          if (j < (p_set->binding_count - 1)) {
            os << "\n";
          }
        }
      }
      if (i < (count - 1)) {
        os << "\n";
      }  
    }
  }

  return os;
}

// =================================================================================================
// PrintUsage()
// =================================================================================================
void PrintUsage()
{
  std::cout
    << "Usage: spirv-reflect [OPTIONS] path/to/SPIR-V/bytecode.spv" << std::endl
    << "Options:" << std::endl
    << " --help         Display this message" << std::endl
    << " -v VERBOSITY   Specify output verbosity:" << std::endl
    << "                0: shader info, block variables, interface variables," << std::endl
    << "                   descriptor bindings. No type descriptions. [default]" << std::endl
    << "                1: Everything above, plus type descriptions." << std::endl
    << "                2: Everything above, plus SPIR-V bytecode and all internal" << std::endl
    << "                   type descriptions. If you're not working on SPIRV-Reflect" << std::endl
    << "                   itself, you probably don't want this." << std::endl
    << std::endl;
}

// =================================================================================================
// main()
// =================================================================================================
int main(int argn, char** argv)
{
//#if defined(WIN32)
//  _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
//  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
//  _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
//  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif

  const char* input_spv_path = nullptr;
  uint32_t verbosity = 0;
  for (int i = 1; i < argn; ++i) {
    std::string arg(argv[i]);
    if (arg == "--help") {
      PrintUsage();
      return EXIT_SUCCESS;
    } else if (arg == "-v" && i+1 < argn) {
      verbosity = static_cast<uint32_t>(strtol(argv[++i], nullptr, 10));
    } else if (i == argn - 1) {
      input_spv_path = argv[i];
    } else {
      std::cerr << "Unrecognized argument: " << arg << std::endl;
      PrintUsage();
      return EXIT_FAILURE;
    }
  }
  if (!input_spv_path) {
    PrintUsage();
    return EXIT_FAILURE;
  }

  std::ifstream spv_ifstream(input_spv_path, std::ios::binary);
  if (!spv_ifstream.is_open()) {
    std::cerr << "ERROR: could not open '" << input_spv_path << "' for reading" << std::endl;
    return EXIT_FAILURE;
  }

  spv_ifstream.seekg(0, std::ios::end);
  size_t size = spv_ifstream.tellg();
  spv_ifstream.seekg(0, std::ios::beg);

  {
    std::vector<char> spv_data(size);
    spv_ifstream.read(spv_data.data(), size);

    spv_reflect::ShaderModule reflection(spv_data.size(), spv_data.data());
    if (reflection.GetResult() != SPV_REFLECT_RESULT_SUCCESS) {
      std::cerr << "ERROR: could not process '" << input_spv_path
                << "' (is it a valid SPIR-V bytecode?)" << std::endl;
      return EXIT_FAILURE;
    }
 
#if 0 // test code to modify reflection data in-place
    //reflection.ChangeDescriptorSetNumber(reflection.GetDescriptorSet(0), 5);
    reflection.ChangeDescriptorBindingNumber(reflection.GetDescriptorBinding(0, 0), 4, 7);
#endif

    //std::cout << reflection << std::endl;
    //std::cout << std::endl;
    SpvReflectToYaml yamlizer(reflection.GetShaderModule(), verbosity);
    std::cout << yamlizer;
  }

#if defined(WIN32)
  _CrtDumpMemoryLeaks();
#endif

  return 0;
}
