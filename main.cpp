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
std::string ToStringVkDescriptorType(VkDescriptorType value) {
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

std::string ToStringBuiltIn(SpvBuiltIn value)
{
  switch (value) {
    case SpvBuiltInPosition : return "Position"; break;
    case SpvBuiltInPointSize : return "PointSize"; break;
    case SpvBuiltInClipDistance : return "ClipDistance"; break;
    case SpvBuiltInCullDistance : return "CullDistance"; break;
    case SpvBuiltInVertexId : return "VertexId"; break;
    case SpvBuiltInInstanceId : return "InstanceId"; break;
    case SpvBuiltInPrimitiveId : return "PrimitiveId"; break;
    case SpvBuiltInInvocationId : return "InvocationId"; break;
    case SpvBuiltInLayer : return "Layer"; break;
    case SpvBuiltInViewportIndex : return "ViewportIndex"; break;
    case SpvBuiltInTessLevelOuter : return "TessLevelOuter"; break;
    case SpvBuiltInTessLevelInner : return "TessLevelInner"; break;
    case SpvBuiltInTessCoord : return "TessCoord"; break;
    case SpvBuiltInPatchVertices : return "PatchVertices"; break;
    case SpvBuiltInFragCoord : return "FragCoord"; break;
    case SpvBuiltInPointCoord : return "PointCoord"; break;
    case SpvBuiltInFrontFacing : return "FrontFacing"; break;
    case SpvBuiltInSampleId : return "SampleId"; break;
    case SpvBuiltInSamplePosition : return "SamplePosition"; break;
    case SpvBuiltInSampleMask : return "SampleMask"; break;
    case SpvBuiltInFragDepth : return "FragDepth"; break;
    case SpvBuiltInHelperInvocation : return "HelperInvocation"; break;
    case SpvBuiltInNumWorkgroups : return "NumWorkgroups"; break;
    case SpvBuiltInWorkgroupSize : return "WorkgroupSize"; break;
    case SpvBuiltInWorkgroupId : return "WorkgroupId"; break;
    case SpvBuiltInLocalInvocationId : return "LocalInvocationId"; break;
    case SpvBuiltInGlobalInvocationId : return "GlobalInvocationId"; break;
    case SpvBuiltInLocalInvocationIndex : return "LocalInvocationIndex"; break;
    case SpvBuiltInWorkDim : return "WorkDim"; break;
    case SpvBuiltInGlobalSize : return "GlobalSize"; break;
    case SpvBuiltInEnqueuedWorkgroupSize : return "EnqueuedWorkgroupSize"; break;
    case SpvBuiltInGlobalOffset : return "GlobalOffset"; break;
    case SpvBuiltInGlobalLinearId : return "GlobalLinearId"; break;
    case SpvBuiltInSubgroupSize : return "SubgroupSize"; break;
    case SpvBuiltInSubgroupMaxSize : return "SubgroupMaxSize"; break;
    case SpvBuiltInNumSubgroups : return "NumSubgroups"; break;
    case SpvBuiltInNumEnqueuedSubgroups : return "NumEnqueuedSubgroups"; break;
    case SpvBuiltInSubgroupId : return "SubgroupId"; break;
    case SpvBuiltInSubgroupLocalInvocationId : return "SubgroupLocalInvocationId"; break;
    case SpvBuiltInVertexIndex : return "VertexIndex"; break;
    case SpvBuiltInInstanceIndex : return "InstanceIndex"; break;
    case SpvBuiltInSubgroupEqMaskKHR : return "SubgroupEqMaskKHR"; break;
    case SpvBuiltInSubgroupGeMaskKHR : return "SubgroupGeMaskKHR"; break;
    case SpvBuiltInSubgroupGtMaskKHR : return "SubgroupGtMaskKHR"; break;
    case SpvBuiltInSubgroupLeMaskKHR : return "SubgroupLeMaskKHR"; break;
    case SpvBuiltInSubgroupLtMaskKHR : return "SubgroupLtMaskKHR"; break;
    case SpvBuiltInBaseVertex : return "BaseVertex"; break;
    case SpvBuiltInBaseInstance : return "BaseInstance"; break;
    case SpvBuiltInDrawIndex : return "DrawIndex"; break;
    case SpvBuiltInDeviceIndex : return "DeviceIndex"; break;
    case SpvBuiltInViewIndex : return "ViewIndex"; break;
    case SpvBuiltInBaryCoordNoPerspAMD : return "BaryCoordNoPerspAMD"; break;
    case SpvBuiltInBaryCoordNoPerspCentroidAMD : return "BaryCoordNoPerspCentroidAMD"; break;
    case SpvBuiltInBaryCoordNoPerspSampleAMD : return "BaryCoordNoPerspSampleAMD"; break;
    case SpvBuiltInBaryCoordSmoothAMD : return "BaryCoordSmoothAMD"; break;
    case SpvBuiltInBaryCoordSmoothCentroidAMD : return "BaryCoordSmoothCentroidAMD"; break;
    case SpvBuiltInBaryCoordSmoothSampleAMD : return "BaryCoordSmoothSampleAMD"; break;
    case SpvBuiltInBaryCoordPullModelAMD : return "BaryCoordPullModelAMD"; break;
    case SpvBuiltInFragStencilRefEXT : return "FragStencilRefEXT"; break;
    case SpvBuiltInViewportMaskNV : return "ViewportMaskNV"; break;
    case SpvBuiltInSecondaryPositionNV : return "SecondaryPositionNV"; break;
    case SpvBuiltInSecondaryViewportMaskNV : return "SecondaryViewportMaskNV"; break;
    case SpvBuiltInPositionPerViewNV : return "PositionPerViewNV"; break;
    case SpvBuiltInViewportMaskPerViewNV : return "ViewportMaskPerViewNV"; break;
  }
  return "";
}

std::string ToStringComponentType(const SpvReflectTypeDescription& type)
{
  uint32_t masked_type = type.type_flags & 0xF;
  if (masked_type == 0) {
    return "";
  }

  std::stringstream ss;
  switch (masked_type) {
    default: assert(false && "unsupported component type"); break;
    case SPV_REFLECT_TYPE_FLAG_BOOL  : ss << "bool"; break;
    case SPV_REFLECT_TYPE_FLAG_INT   : ss << (type.traits.numeric.scalar.signedness ? "int" : "uint"); break;
    case SPV_REFLECT_TYPE_FLAG_FLOAT : ss << "float"; break;
  }

  if (type.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
    ss << type.traits.numeric.matrix.row_count;
    ss << "x";
    ss << type.traits.numeric.matrix.column_count;
  }
  else if (type.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
    ss << type.traits.numeric.vector.component_count;
  }

  return ss.str();
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
      text_line.text_elements.push_back(ToStringComponentType(*member.type_description));
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
  os << t << "type      : " << ToStringComponentType(*obj.type_description) << "\n";
  os << t << "name      : " << (obj.name != NULL ? obj.name : "") << "\n";
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

std::ostream& operator<<(std::ostream& os, const SpvReflectInterfaceVariable& obj)
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
//#if defined(WIN32)
//  _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
//  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
//  _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
//  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif

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

#if defined(WIN32)
  _CrtDumpMemoryLeaks();
#endif

  return 0;
}
