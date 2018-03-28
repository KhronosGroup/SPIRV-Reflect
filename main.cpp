/*
 Copyright 2017-2018 Google Inc.
 
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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "spirv_reflect.h"
#include "examples/arg_parser.h"
#include "examples/common.h"

enum TextLineFlags {
  TEXT_LINE_FLAGS_STRUCT_BEGIN  = 0x01,
  TEXT_LINE_FLAGS_STRUCT_END    = 0x02,
  TEXT_LINE_FLAGS_LINES         = 0x04,
};

struct TextLine {
  std::string           indent;
  std::string           modifier;
  std::string           type_name;
  std::string           name;
  uint32_t              offset;
  uint32_t              absolute_offset;
  uint32_t              size;
  uint32_t              padded_size;
  uint32_t              flags;
  std::vector<TextLine> lines;
  std::string           formatted_line;
  std::string           formatted_offset;
  std::string           formatted_absolute_offset;
  std::string           formatted_size;
  std::string           formatted_padded_size;
};

const char* ToStringVulkanShaderStage(VkShaderStageFlagBits stage)
{
  switch (stage) {
  default: break;
    case VK_SHADER_STAGE_VERTEX_BIT                  : return "VS";
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT    : return "HS";
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT : return "DS";
    case VK_SHADER_STAGE_GEOMETRY_BIT                : return "GS";
    case VK_SHADER_STAGE_FRAGMENT_BIT                : return "PS";
    case VK_SHADER_STAGE_COMPUTE_BIT                 : return "CS";
  }
  return "";
}

const char* ToStringHlslResourceType(SpvReflectResourceType type)
{
  switch (type) {
    case SPV_REFLECT_RESOURCE_FLAG_UNDEFINED : return "UNDEFINED";
    case SPV_REFLECT_RESOURCE_FLAG_SAMPLER   : return "SAMPLER";
    case SPV_REFLECT_RESOURCE_FLAG_CBV       : return "CBV";
    case SPV_REFLECT_RESOURCE_FLAG_SRV       : return "SRV";
    case SPV_REFLECT_RESOURCE_FLAG_UAV       : return "UAV";
  }
  return "";
}

std::string ToStringVkDescriptorType(VkDescriptorType value) {
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

std::string ToStringBuiltIn(SpvBuiltIn value)
{
  switch (value) {
    case SpvBuiltInPosition : return "Position";
    case SpvBuiltInPointSize : return "PointSize";
    case SpvBuiltInClipDistance : return "ClipDistance";
    case SpvBuiltInCullDistance : return "CullDistance";
    case SpvBuiltInVertexId : return "VertexId";
    case SpvBuiltInInstanceId : return "InstanceId";
    case SpvBuiltInPrimitiveId : return "PrimitiveId";
    case SpvBuiltInInvocationId : return "InvocationId";
    case SpvBuiltInLayer : return "Layer";
    case SpvBuiltInViewportIndex : return "ViewportIndex";
    case SpvBuiltInTessLevelOuter : return "TessLevelOuter";
    case SpvBuiltInTessLevelInner : return "TessLevelInner";
    case SpvBuiltInTessCoord : return "TessCoord";
    case SpvBuiltInPatchVertices : return "PatchVertices";
    case SpvBuiltInFragCoord : return "FragCoord";
    case SpvBuiltInPointCoord : return "PointCoord";
    case SpvBuiltInFrontFacing : return "FrontFacing";
    case SpvBuiltInSampleId : return "SampleId";
    case SpvBuiltInSamplePosition : return "SamplePosition";
    case SpvBuiltInSampleMask : return "SampleMask";
    case SpvBuiltInFragDepth : return "FragDepth";
    case SpvBuiltInHelperInvocation : return "HelperInvocation";
    case SpvBuiltInNumWorkgroups : return "NumWorkgroups";
    case SpvBuiltInWorkgroupSize : return "WorkgroupSize";
    case SpvBuiltInWorkgroupId : return "WorkgroupId";
    case SpvBuiltInLocalInvocationId : return "LocalInvocationId";
    case SpvBuiltInGlobalInvocationId : return "GlobalInvocationId";
    case SpvBuiltInLocalInvocationIndex : return "LocalInvocationIndex";
    case SpvBuiltInWorkDim : return "WorkDim";
    case SpvBuiltInGlobalSize : return "GlobalSize";
    case SpvBuiltInEnqueuedWorkgroupSize : return "EnqueuedWorkgroupSize";
    case SpvBuiltInGlobalOffset : return "GlobalOffset";
    case SpvBuiltInGlobalLinearId : return "GlobalLinearId";
    case SpvBuiltInSubgroupSize : return "SubgroupSize";
    case SpvBuiltInSubgroupMaxSize : return "SubgroupMaxSize";
    case SpvBuiltInNumSubgroups : return "NumSubgroups";
    case SpvBuiltInNumEnqueuedSubgroups : return "NumEnqueuedSubgroups";
    case SpvBuiltInSubgroupId : return "SubgroupId";
    case SpvBuiltInSubgroupLocalInvocationId : return "SubgroupLocalInvocationId";
    case SpvBuiltInVertexIndex : return "VertexIndex";
    case SpvBuiltInInstanceIndex : return "InstanceIndex";
    case SpvBuiltInSubgroupEqMaskKHR : return "SubgroupEqMaskKHR";
    case SpvBuiltInSubgroupGeMaskKHR : return "SubgroupGeMaskKHR";
    case SpvBuiltInSubgroupGtMaskKHR : return "SubgroupGtMaskKHR";
    case SpvBuiltInSubgroupLeMaskKHR : return "SubgroupLeMaskKHR";
    case SpvBuiltInSubgroupLtMaskKHR : return "SubgroupLtMaskKHR";
    case SpvBuiltInBaseVertex : return "BaseVertex";
    case SpvBuiltInBaseInstance : return "BaseInstance";
    case SpvBuiltInDrawIndex : return "DrawIndex";
    case SpvBuiltInDeviceIndex : return "DeviceIndex";
    case SpvBuiltInViewIndex : return "ViewIndex";
    case SpvBuiltInBaryCoordNoPerspAMD : return "BaryCoordNoPerspAMD";
    case SpvBuiltInBaryCoordNoPerspCentroidAMD : return "BaryCoordNoPerspCentroidAMD";
    case SpvBuiltInBaryCoordNoPerspSampleAMD : return "BaryCoordNoPerspSampleAMD";
    case SpvBuiltInBaryCoordSmoothAMD : return "BaryCoordSmoothAMD";
    case SpvBuiltInBaryCoordSmoothCentroidAMD : return "BaryCoordSmoothCentroidAMD";
    case SpvBuiltInBaryCoordSmoothSampleAMD : return "BaryCoordSmoothSampleAMD";
    case SpvBuiltInBaryCoordPullModelAMD : return "BaryCoordPullModelAMD";
    case SpvBuiltInFragStencilRefEXT : return "FragStencilRefEXT";
    case SpvBuiltInViewportMaskNV : return "ViewportMaskNV";
    case SpvBuiltInSecondaryPositionNV : return "SecondaryPositionNV";
    case SpvBuiltInSecondaryViewportMaskNV : return "SecondaryViewportMaskNV";
    case SpvBuiltInPositionPerViewNV : return "PositionPerViewNV";
    case SpvBuiltInViewportMaskPerViewNV : return "ViewportMaskPerViewNV";

    case SpvBuiltInMax:
      break;
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
    std::stringstream ss_indent;
    for (int indent_count = 0; indent_count < indent_depth; ++indent_count) {
      ss_indent << t;
    }
    std::string expanded_indent = ss_indent.str();

    const auto& member = p_members[member_index];
    if (member.member_count > 0) {
      // Begin struct
      TextLine tl = {};
      tl.indent = expanded_indent;
      tl.type_name = member.type_description->type_name;
      tl.offset = member.offset;
      tl.absolute_offset = member.absolute_offset;
      tl.size = member.size;
      tl.padded_size = member.padded_size;
      tl.flags = TEXT_LINE_FLAGS_STRUCT_BEGIN;
      p_text_lines->push_back(tl);

      // Members
      tl = {};
      ParseBlockMembersToTextLines(t, indent_depth + 1, member.member_count, member.members, &tl.lines);
      tl.flags = TEXT_LINE_FLAGS_LINES;
      p_text_lines->push_back(tl);      

      // End struct
      tl = {};
      tl.indent = expanded_indent;
      tl.name = member.name;
      tl.offset = member.offset;
      tl.absolute_offset = member.absolute_offset;
      tl.size = member.size;
      tl.padded_size = member.padded_size;
      tl.flags = TEXT_LINE_FLAGS_STRUCT_END;
      p_text_lines->push_back(tl);      
    }
    else {
      TextLine tl = {};
      tl.indent = expanded_indent;
      tl.type_name = ToStringComponentType(*member.type_description);
      tl.name = member.name;
      tl.offset = member.offset;
      tl.absolute_offset = member.absolute_offset;
      tl.size = member.size;
      tl.padded_size = member.padded_size;
      p_text_lines->push_back(tl);      
    }
  }
}

void FormatTextLines(const std::vector<TextLine>& text_lines, const char* indent, std::vector<TextLine>* p_formatted_lines)
{
  size_t modifier_width = 0;
  size_t type_name_width = 0;
  size_t name_width = 0;

  // Widths
  for (auto& tl : text_lines) {
    if (tl.flags != 0) {
      continue;
    }
    modifier_width = std::max<size_t>(modifier_width, tl.modifier.length());
    type_name_width = std::max<size_t>(type_name_width, tl.type_name.length());
    name_width = std::max<size_t>(name_width, tl.name.length());
  }

  // Output
  size_t n = text_lines.size();
  for (size_t i = 0; i < n; ++i) {
    auto& tl = text_lines[i];

    std::stringstream ss;
    if (tl.flags & TEXT_LINE_FLAGS_STRUCT_BEGIN) {
      ss << indent;
      ss << tl.indent;
      ss << "struct ";
      ss << tl.type_name;
      ss << " {";
    }
    else if (tl.flags & TEXT_LINE_FLAGS_STRUCT_END) {
      ss << indent;
      ss << tl.indent;
      ss << "} ";
      ss << tl.name;
      ss << ";";
    }
    else if (tl.flags & TEXT_LINE_FLAGS_LINES) {
      FormatTextLines(tl.lines, indent, p_formatted_lines);
    }
    else {
      ss << indent;
      ss << tl.indent;
      if (modifier_width > 0) {
        ss << std::setw(modifier_width) << std::left << tl.modifier;
        ss << " ";
      }     
      ss << std::setw(type_name_width) << std::left << tl.type_name;
      ss << " ";
      ss << std::setw(name_width) << (tl.name + ";");
    }

    // Reuse the various strings to store the formatted texts
    TextLine out_tl = {};
    out_tl.formatted_line = ss.str();
    if (out_tl.formatted_line.length() > 0) {
      out_tl.flags = tl.flags;
      out_tl.formatted_offset = std::to_string(tl.offset);
      out_tl.formatted_absolute_offset = std::to_string(tl.absolute_offset);
      out_tl.formatted_size = std::to_string(tl.size);
      out_tl.formatted_padded_size = std::to_string(tl.padded_size);
      p_formatted_lines->push_back(out_tl);
    }
  }
}

void StreamWrite(std::ostream& os, const char* indent, const std::vector<TextLine>& text_lines)
{
  std::vector<TextLine> formatted_lines;
  FormatTextLines(text_lines, indent, &formatted_lines);

  size_t line_width = 0;
  size_t offset_width = 0;
  size_t absolute_offset_width = 0;
  size_t size_width = 0;
  size_t padded_size_width = 0;

  // Width
  for (auto& tl : formatted_lines) {
    line_width = std::max<size_t>(line_width, tl.formatted_line.length());
    offset_width = std::max<size_t>(offset_width, tl.formatted_offset.length());
    absolute_offset_width = std::max<size_t>(absolute_offset_width, tl.formatted_absolute_offset.length());
    size_width = std::max<size_t>(size_width, tl.formatted_size.length());
    padded_size_width = std::max<size_t>(padded_size_width, tl.formatted_padded_size.length());
  }

  size_t n = formatted_lines.size();
  for (size_t i = 0; i < n; ++i) {
    auto& tl = formatted_lines[i];


    if (tl.flags == TEXT_LINE_FLAGS_STRUCT_BEGIN) {
      if (i > 0) {
        os << "\n";
      }

      size_t pos = tl.formatted_line.find_first_not_of(' ');
      if (pos != std::string::npos) {
        std::string s(pos, ' ');
        os << s << "//" << " ";
        os << "offset = " << tl.formatted_offset << ", ";
        os << "abs offset = " << tl.formatted_absolute_offset << ", ";
        os << "size = " << tl.formatted_size << ", ";
        os << "padded size = " << tl.formatted_padded_size;
        os << "\n";
      }

      os << std::setw(line_width) << std::left << tl.formatted_line;
    }
    else if (tl.flags == TEXT_LINE_FLAGS_STRUCT_END) {
      os << std::setw(line_width) << std::left << tl.formatted_line;
      if (i < (n - 1)) {
        os << "\n";
      }
    }
    else {
      os << std::setw(line_width) << std::left << tl.formatted_line;
      os << " " << "//" << " ";
      os << "offset = " << std::setw(offset_width) << std::right << tl.formatted_offset << ", ";
      os << "abs offset = " << std::setw(absolute_offset_width) << std::right << tl.formatted_absolute_offset << ", ";
      os << "size = " << std::setw(size_width) << std::right << tl.formatted_size << ", ";
      os << "padded size = " << std::setw(padded_size_width) << std::right << tl.formatted_padded_size;
    }

    if (i < (n - 1)) {
      os << "\n";
    }
  }
}

void StreamWrite(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent = "")
{
  const char* t = indent;
  os << t << "binding : " << obj.binding << "\n";
  if (write_set) {
    os << t << "set     : " << obj.set << "\n";
  }
  os << t << "type    : " << ToStringVkDescriptorType(obj.descriptor_type);
  os << " " << "(" << ToStringHlslResourceType(obj.resource_type) << ")" << "\n";
  
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
    ParseBlockMembersToTextLines("    ", 1, 1, &obj.block, &text_lines);
    if (!text_lines.empty()) {
      os << "\n";
      StreamWrite(os, t, text_lines);
      os << "\n";
    }
  }
}

void StreamWrite(std::ostream& os, const SpvReflectInterfaceVariable& obj, const char* indent = "")
{
  const char* t = indent;
  os << t << "location  : ";
  if (obj.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
    os << "(built-in)" << " ";
    os << ToStringBuiltIn(obj.built_in);
  }
  else {
    os << obj.location;
  }
  os << "\n";
  os << t << "type      : " << ToStringComponentType(*obj.type_description) << "\n";
  os << t << "semantic  : " << (obj.semantic != NULL ? obj.semantic : "") << "\n";
  os << t << "name      : " << (obj.name != NULL ? obj.name : "") << "\n";
  os << t << "qualifier : ";
  if (obj.decoration_flags & SPV_REFLECT_DECORATION_FLAT) {
    os << "flat";
  }
  else   if (obj.decoration_flags & SPV_REFLECT_DECORATION_NOPERSPECTIVE) {
    os << "noperspective";
  }
}

void StreamWrite(std::ostream& os, const SpvReflectShaderModule& obj, const char* /*indent*/ = "")
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

std::ostream& operator<<(std::ostream& os, const SpvReflectInterfaceVariable& /*obj*/)
{
  return os;
}

std::ostream& operator<<(std::ostream& os, const spv_reflect::ShaderModule& obj)
{
  const char* t     = "  ";
  const char* tt    = "    ";
  const char* ttt   = "      ";

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
    << "Prints a summary of the reflection data extracted from SPIR-V bytecode." << std::endl
    << "Options:" << std::endl
    << " --help         Display this message" << std::endl
    << " -y,--yaml      Format output as YAML. [default: disabled]" << std::endl
    << " -v VERBOSITY   Specify output verbosity (YAML output only):" << std::endl
    << "                0: shader info, block variables, interface variables," << std::endl
    << "                   descriptor bindings. No type descriptions. [default]" << std::endl
    << "                1: Everything above, plus type descriptions." << std::endl
    << "                2: Everything above, plus SPIR-V bytecode and all internal" << std::endl
    << "                   type descriptions. If you're not working on SPIRV-Reflect" << std::endl
    << "                   itself, you probably don't want this." << std::endl
    << "-e,--entrypoint Prints the entry point found in shader module." << std::endl
    << "-s,--stage      Prints the Vulkan shader stage found in shader module." << std::endl;
}

// =================================================================================================
// main()
// =================================================================================================
int main(int argn, char** argv)
{
  ArgParser arg_parser;
  arg_parser.AddFlag("h", "help", "");
  arg_parser.AddFlag("y", "yaml", "");
  arg_parser.AddOptionInt("v", "verbosity", "", 0);
  arg_parser.AddFlag("e", "entrypoint", "");
  arg_parser.AddFlag("s", "stage", "");
  if (!arg_parser.Parse(argn, argv, std::cerr)) {
    PrintUsage();
    return EXIT_FAILURE;
  }

  bool output_as_yaml = arg_parser.GetFlag("y", "yaml");

  int yaml_verbosity = 0;
  arg_parser.GetInt("v", "verbosity", &yaml_verbosity);

	bool print_entry_point = arg_parser.GetFlag("e", "entrypoint");
  bool print_shader_stage = arg_parser.GetFlag("s", "stage");

	std::string input_spv_path;
	if (!arg_parser.GetArg(0, &input_spv_path)) {
    std::cerr << "ERROR: no SPIR-V file specified" << std::endl;
    return EXIT_FAILURE;
	}
			
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

		if (print_entry_point || print_shader_stage) {
      size_t printed_count = 0;
      if (print_entry_point) {
        std::cout<< reflection.GetEntryPointName();
        ++printed_count;
      }

      if (print_shader_stage) {
        if (printed_count > 0) {
          std::cout << ";";
        }
        std::cout << ToStringVulkanShaderStage(reflection.GetVulkanShaderStage());
      }

      std::cout << std::endl;
		}
		else {
			if (output_as_yaml) {
				SpvReflectToYaml yamlizer(reflection.GetShaderModule(), yaml_verbosity);
				std::cout << yamlizer;
			} else {
				std::cout << reflection << std::endl;
				std::cout << std::endl;
			}
		}
  }

  return EXIT_SUCCESS;
}
