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

// =================================================================================================
// Stream Output
// =================================================================================================
const char* ToStringVkDescriptorType(VkDescriptorType value) {
  switch (value) {
    default: return ""; break;
    case VK_DESCRIPTOR_TYPE_SAMPLER                : return "VK_DESCRIPTOR_TYPE_SAMPLER"; break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"; break;
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE          : return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE"; break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE          : return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE"; break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   : return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER"; break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   : return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER"; break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER"; break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER"; break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC"; break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC"; break;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       : return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT"; break;
  }
  return "";
}

void StreamWrite(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent = "")
{
  const char* t = indent;

  os << " " << obj.name;
  os << "\n";
  os << t << ToStringVkDescriptorType(obj.descriptor_type);
  if ((obj.descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (obj.descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) || 
      (obj.descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) || (obj.descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)) {
    os << "\n";
    os << t;
    os << "dim=" << obj.image.dim << ", ";
    os << "depth=" << obj.image.depth << ", ";
    os << "arrayed=" << obj.image.arrayed << ", ";
    os << "ms=" << obj.image.ms << ", ";
    os << "sampled=" << obj.image.sampled;
  }

  //if ((obj.type_description->type_name != nullptr) && (strlen(obj.type_description->type_name) > 0)) {
  //  os << " " << "(" << obj.type_description->type_name << ")";
  //}

/*
  const char* t = indent;
  os << t << "binding=" << obj.binding << ", ";
  if (write_set) {
    os << "set=" << obj.set;
  }
  os << "\n";
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
*/
}

void StreamWrite(std::ostream& os, const SpvReflectShaderModule& obj, const char* indent = "")
{
  os << "entry point     : " << obj.entry_point_name << "\n";
  os << "source lang     : " << spvReflectSourceLanguage(obj.source_language) << "\n";
  os << "source lang ver : " << obj.source_language_version;
}

std::ostream& operator<<(std::ostream& os, const SpvReflectDescriptorBinding& obj)
{
  StreamWrite(os, obj, true, "  ");
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
      os << tt << i << ":";
      StreamWrite(os, *p_binding, true, ttt);
      if (i < (count - 1)) {
        os << "\n\n";
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
  std::cout << "Usage: spirv-reflect [OPTIONS] path/to/SPIR-V/bytecode.spv" << std::endl
            << "Options:" << std::endl
            << " --help:               Display this message" << std::endl
            << std::endl;
}

// =================================================================================================
// main()
// =================================================================================================
int main(int argn, char** argv)
{
  if (argn != 2) {
    PrintUsage();
    return EXIT_FAILURE;
  } else if (std::string(argv[1]) == "--help") {
    PrintUsage();
    return EXIT_SUCCESS;
  }
  std::string input_spv_path = argv[1];

  std::ifstream spv_ifstream(input_spv_path.c_str(), std::ios::binary);
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

    std::cout << reflection << std::endl;
    std::cout << std::endl;
  }


  return 0;
}
