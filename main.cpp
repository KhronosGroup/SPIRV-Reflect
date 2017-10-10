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
#include <iostream>
#include <fstream>
#include <vector>

#include "spirv_reflect.h"

// =================================================================================================
// Stream Output
// =================================================================================================
inline const char* ToString(VkDescriptorType value) {
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

inline const char* ToStringSimple(const SpvReflectTypeDescription& type)
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

inline void StreamWrite(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent = "")
{
  const char* t = indent;
  os << t << "binding : " << obj.binding << "\n";
  if (write_set) {
    os << t << "set     : " << obj.set << "\n";
  }
  os << t << "type    : " << ToString(obj.descriptor_type) << "\n";
  os << t << "name    : " << obj.name;
  if ((obj.type_description->type_name != nullptr) && (strlen(obj.type_description->type_name) > 0)) {
    os << " " << "(" << obj.type_description->type_name << ")";
  }
}

inline void StreamWrite(std::ostream& os, const SpvReflectDescriptorSet& obj, const char* indent = "")
{
  const char* t = indent;
  os << t << "set           : " << obj.set << "\n";
  os << t << "binding count : " << obj.binding_count;
}

inline void StreamWrite(std::ostream& os, const SpvReflectInterfaceVariable& obj, const char* indent = "")
{
  const char* t = indent;
  os << t << "location  : ";
  if (obj.decorations & SPV_REFLECT_DECORATION_BUILT_IN) {
    os << "(built-in)";
  }
  else {
    os << obj.location;
  }
  os << "\n";
  os << t << "type      : " << ToStringSimple(*obj.type_description) << "\n";
  os << t << "qualifier : ";
  if (obj.decorations & SPV_REFLECT_DECORATION_FLAT) {
    os << "flat";
  }
  else   if (obj.decorations & SPV_REFLECT_DECORATION_NOPERSPECTIVE) {
    os << "noperspective";
  }
}

inline void StreamWrite(std::ostream& os, const SpvReflectShaderReflection& obj, const char* indent = "")
{
  os << "entry point     : " << obj.entry_point_name << "\n";
  os << "source lang     : " << spvReflectSourceLanguage(obj.source_language) << "\n";
  os << "source lang ver : " << obj.source_language_version;
}

inline std::ostream& operator<<(std::ostream& os, const SpvReflectDescriptorBinding& obj)
{
  StreamWrite(os, obj, true, "  ");
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const SpvReflectDescriptorSet& obj)
{
  StreamWrite(os, obj, "  ");
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

inline std::ostream& operator<<(std::ostream& os, const SpvReflectInterfaceVariable& obj)
{
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const SpvReflectShaderReflection& obj)
{
  const char* t     = "  ";
  const char* tt    = "    ";
  const char* ttt   = "      ";
  const char* tttt  = "        ";
  const char* ttttt = "          ";
  StreamWrite(os, obj, "");

  //if (obj.input_variable_count)  {
  //  os << "\n";
  //  os << "\n";
  //  os << t << "input variables: " << obj.input_variable_count << "\n";
  //  for (uint32_t i = 0; i < obj.input_variable_count; ++i) {
  //    os << tt << i << ":" << "\n";
  //    StreamWrite(os, obj.input_variables[i], ttt);
  //    if (i < (obj.input_variable_count - 1)) {
  //      os << "\n";
  //    }      
  //  }
  //}

  if (obj.output_variable_count)  {
    os << "\n";
    os << "\n";
    os << t << "output variables: " << obj.output_variable_count << "\n";
    for (uint32_t i = 0; i < obj.output_variable_count; ++i) {
      os << tt << i << ":" << "\n";
      StreamWrite(os, obj.output_variables[i], ttt);
      if (i < (obj.output_variable_count - 1)) {
        os << "\n";
      }      
    }
  }
  
  if (obj.descriptor_binding_count > 0) {
    os << "\n";
    os << "\n";
    os << t << "descriptor bindings: " << obj.descriptor_binding_count << "\n";
    for (uint32_t i = 0; i < obj.descriptor_binding_count; ++i) {
      os << tt << i << ":" << "\n";
      StreamWrite(os, obj.descriptor_bindings[i], true, ttt);
      if (i < (obj.descriptor_binding_count - 1)) {
        os << "\n";
      }      
    }
  }

  if (obj.descriptor_set_count > 0) {
    os << "\n";
    os << "\n";
    os << t << "descriptor sets: " << obj.descriptor_set_count << "\n";
    for (uint32_t i = 0; i < obj.descriptor_set_count; ++i) {
      os << tt << i << ":" << "\n";
      const SpvReflectDescriptorSet& set = obj.descriptor_sets[i];
      StreamWrite(os, set, ttt);
      if (set.binding_count > 0) {
        os << "\n";
        for (uint32_t j = 0; j < set.binding_count; ++j) {
          const SpvReflectDescriptorBinding& binding = *set.bindings[j];
          os << tttt << j << ":" << "\n";
          StreamWrite(os, binding, false, ttttt);
          if (j < (set.binding_count - 1)) {
            os << "\n";
          }
        }
      }
      if (i < (obj.descriptor_set_count - 1)) {
        os << "\n";
      }      
    }
  }

  return os;
}

// =================================================================================================
// main()
// =================================================================================================
int main(int argn, char** argv)
{
#if defined(WIN32)
  _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
  _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  std::string file_path = argv[1];

  std::ifstream is(file_path.c_str(), std::ios::binary);
  assert(is.is_open());

  is.seekg(0, std::ios::end);
  size_t size = is.tellg();
  is.seekg(0, std::ios::beg);  

  std::vector<char> data(size);
  is.read(data.data(), size);

  spv_reflect::ShaderReflection reflection(data.size(), data.data());
  auto re = reflection.GetShaderRelection();

  std::cout << re << std::endl;

/*
  auto count = reflection.GetDescriptorBindingCount();
  for (auto i = 0; i < count; ++i) {
    auto elem = reflection.GetDescriptorBinding(i);
    std::cout << "Descriptor Binding" << "\n";
    std::cout << *elem << "\n";
    std::cout << "\n";
  }
  std::cout << "\n";
*/

/*
  count = reflection.GetDescriptorSetCount();
  for (auto i = 0; i < count; ++i) {
    auto elem = reflection.GetDescriptorSet(i);
    std::cout << "Descriptor Set" << "\n";
    std::cout << *elem << "\n";
    std::cout << "\n";
  }
*/

  std::cout << std::endl;
  
/*
  spirv_ref::ShaderReflection reflection;
  spirv_ref::ParseShaderReflection(data.size(), data.data(), &reflection);

  //auto count = reflection.GetTypeCount();
  //for (auto i = 0; i < count; ++i) {
  //  auto elem = reflection.GetType(i);
  //  std::cout << elem->GetInfo("  ") << std::endl;
  //}

  auto count = reflection.GetInputVariableCount();
  for (auto i = 0; i < count; ++i) {
    auto elem = reflection.GetInputVariable(i);
    std::cout << elem->GetInfo("  ") << "\n";
  }

  count = reflection.GetOutputVariableCount();
  for (auto i = 0; i < count; ++i) {
    auto elem = reflection.GetOutputVariable(i);
    std::cout << elem->GetInfo("  ") << "\n";
  }

  count = reflection.GetDescriptorCount();
  for (auto i = 0; i < count; ++i) {
    auto elem = reflection.GetDescriptor(i);
    std::cout << elem->GetInfo("  ") << "\n";
  }

  //reflection.GetDescriptorSet(0)->GetBinding(0)->SetBindingNumber(2);
*/

#if defined(WIN32)
  _CrtDumpMemoryLeaks();
#endif

  return 0;
}