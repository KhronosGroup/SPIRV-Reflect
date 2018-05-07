#include "output_stream.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

enum Generator {
  GENERATOR_KHRONOS_LLVM_SPIRV_TRANSLATOR         = 6,
  GENERATOR_KHRONOS_SPIRV_TOOLS_ASSEMBLER         = 7,
  GENERATOR_KHRONOS_GLSLANG_REFERENCE_FRONT_END   = 8,
  GENERATOR_GOOGLE_SHADERC_OVER_GLSLANG           = 13,
  GENERATOR_GOOGLE_SPIREGG                        = 14,
  GENERATOR_GOOGLE_RSPIRV                         = 15,
  GENERATOR_X_LEGEND_MESA_MESAIR_SPIRV_TRANSLATOR = 16,
  GENERATOR_KHRONOS_SPIRV_TOOLS_LINKER            = 17,
  GENERATOR_WINE_VKD3D_SHADER_COMPILER            = 18,
  GENERATOR_CLAY_CLAY_SHADER_COMPILER             = 19,
};

enum TextLineType {
  TEXT_LINE_TYPE_BLOCK_BEGIN   = 0x01,
  TEXT_LINE_TYPE_BLOCK_END     = 0x02,
  TEXT_LINE_TYPE_STRUCT_BEGIN  = 0x04,
  TEXT_LINE_TYPE_STRUCT_END    = 0x08,
  TEXT_LINE_TYPE_LINES         = 0x10,
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
  uint32_t              array_stride;
  uint32_t              flags;
  std::vector<TextLine> lines;
  std::string           formatted_line;
  std::string           formatted_offset;
  std::string           formatted_absolute_offset;
  std::string           formatted_size;
  std::string           formatted_padded_size;
  std::string           formatted_array_stride;
};

const char* ToStringGenerator(Generator generator)
{
  switch (generator) {
    case GENERATOR_KHRONOS_LLVM_SPIRV_TRANSLATOR         : return "Khronos LLVM/SPIR-V Translator"; break;
    case GENERATOR_KHRONOS_SPIRV_TOOLS_ASSEMBLER         : return "Khronos SPIR-V Tools Assembler"; break;
    case GENERATOR_KHRONOS_GLSLANG_REFERENCE_FRONT_END   : return "Khronos Glslang Reference Front End"; break;
    case GENERATOR_GOOGLE_SHADERC_OVER_GLSLANG           : return "Google Shaderc over Glslang"; break;
    case GENERATOR_GOOGLE_SPIREGG                        : return "Google spiregg"; break;
    case GENERATOR_GOOGLE_RSPIRV                         : return "Google rspirv"; break;
    case GENERATOR_X_LEGEND_MESA_MESAIR_SPIRV_TRANSLATOR : return "X-LEGEND Mesa-IR/SPIR-V Translator"; break;
    case GENERATOR_KHRONOS_SPIRV_TOOLS_LINKER            : return "Khronos SPIR-V Tools Linker"; break;
    case GENERATOR_WINE_VKD3D_SHADER_COMPILER            : return "Wine VKD3D Shader Compiler"; break;
    case GENERATOR_CLAY_CLAY_SHADER_COMPILER             : return "Clay Clay Shader Compiler"; break;
  }
  return "Unknown Generator";
}

const char* ToStringShaderStage(SpvReflectShaderStageFlagBits stage)
{
  switch (stage) {
  default: break;
    case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT                  : return "VS";
    case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT    : return "HS";
    case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT : return "DS";
    case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT                : return "GS";
    case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT                : return "PS";
    case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT                 : return "CS";
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

std::string ToStringDescriptorType(SpvReflectDescriptorType value) {
  switch (value) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER                : return "VK_DESCRIPTOR_TYPE_SAMPLER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE          : return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE          : return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   : return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   : return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER         : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER         : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       : return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
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
    default:
      break;
  }
  return "";
}

std::string ToStringComponentType(const SpvReflectTypeDescription& type, uint32_t member_decoration_flags)
{
  uint32_t masked_type = type.type_flags & 0xF;
  if (masked_type == 0) {
    return "";
  }

  std::stringstream ss;

  if (type.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
    if (member_decoration_flags & SPV_REFLECT_DECORATION_COLUMN_MAJOR) {
      ss << "column_major" << " ";
    }
    else if (member_decoration_flags & SPV_REFLECT_DECORATION_ROW_MAJOR) {
      ss << "row_major" << " ";
    }
  }

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
    bool is_struct = member.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT;
    if (is_struct) {
      // Begin struct
      TextLine tl = {};
      tl.indent = expanded_indent;
      tl.type_name = member.type_description->type_name;
      tl.offset = member.offset;
      tl.absolute_offset = member.absolute_offset;
      tl.size = member.size;
      tl.padded_size = member.padded_size;
      tl.array_stride = member.array.stride;
      tl.flags = TEXT_LINE_TYPE_STRUCT_BEGIN;
      p_text_lines->push_back(tl);

      // Members
      tl = {};
      ParseBlockMembersToTextLines(t, indent_depth + 1, member.member_count, member.members, &tl.lines);
      tl.flags = TEXT_LINE_TYPE_LINES;
      p_text_lines->push_back(tl);      

      // End struct
      tl = {};
      tl.indent = expanded_indent;
      tl.name = member.name;
      if (member.array.dims_count > 0) {
        std::stringstream ss_array;
        for (uint32_t array_dim_index = 0; array_dim_index < member.array.dims_count; ++array_dim_index) {
          uint32_t dim = member.array.dims[array_dim_index];
          ss_array << "[" << dim << "]";
        }
        tl.name += ss_array.str();
      }
      tl.offset = member.offset;
      tl.absolute_offset = member.absolute_offset;
      tl.size = member.size;
      tl.padded_size = member.padded_size;
      tl.array_stride = member.array.stride;
      tl.flags = TEXT_LINE_TYPE_STRUCT_END;
      p_text_lines->push_back(tl);      
    }
    else {
      TextLine tl = {};
      tl.indent = expanded_indent;
      tl.type_name = ToStringComponentType(*member.type_description, member.decoration_flags);
      tl.name = member.name;
      if (member.array.dims_count > 0) {
        std::stringstream ss_array;
        for (uint32_t array_dim_index = 0; array_dim_index < member.array.dims_count; ++array_dim_index) {
          uint32_t dim = member.array.dims[array_dim_index];
          ss_array << "[" << dim << "]";
        }
        tl.name += ss_array.str();
      }
      tl.offset = member.offset;
      tl.absolute_offset = member.absolute_offset;
      tl.size = member.size;
      tl.array_stride = member.array.stride;
      tl.padded_size = member.padded_size;
      p_text_lines->push_back(tl);      
    }
  }
}

void ParseBlockVariableToTextLines(const char* indent, const SpvReflectBlockVariable& block_var, std::vector<TextLine>* p_text_lines)
{
  (void)indent;
  (void)block_var;
  (void)p_text_lines;

  // Begin struct
  TextLine tl = {};
  tl.indent = indent;
  tl.type_name = block_var.type_description->type_name;
  tl.size = block_var.size;
  tl.padded_size = block_var.padded_size;
  tl.flags = TEXT_LINE_TYPE_BLOCK_BEGIN;
  p_text_lines->push_back(tl);

  // Members
  tl = {};
  ParseBlockMembersToTextLines(indent, 2, block_var.member_count, block_var.members, &tl.lines);
  tl.flags = TEXT_LINE_TYPE_LINES;
  p_text_lines->push_back(tl);    

  // End struct
  tl = {};
  tl.indent = indent;
  tl.name = block_var.name;
  tl.offset = 0;
  tl.absolute_offset = 0;
  tl.size = block_var.size;
  tl.padded_size = block_var.padded_size;
  tl.flags = TEXT_LINE_TYPE_BLOCK_END;
  p_text_lines->push_back(tl);
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
    if ((tl.flags == TEXT_LINE_TYPE_BLOCK_BEGIN) || (tl.flags == TEXT_LINE_TYPE_STRUCT_BEGIN)) {
      ss << indent;
      ss << tl.indent;
      ss << "struct ";
      ss << tl.type_name;
      ss << " {";
    }
    else if ((tl.flags == TEXT_LINE_TYPE_BLOCK_END) || (tl.flags == TEXT_LINE_TYPE_STRUCT_END)) {
      ss << indent;
      ss << tl.indent;
      ss << "} ";
      ss << tl.name;
      ss << ";";
    }
    else if (tl.flags == TEXT_LINE_TYPE_LINES) {
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
     out_tl.array_stride = tl.array_stride;
      out_tl.flags = tl.flags;
      out_tl.formatted_offset = std::to_string(tl.offset);
      out_tl.formatted_absolute_offset = std::to_string(tl.absolute_offset);
      out_tl.formatted_size = std::to_string(tl.size);
      out_tl.formatted_padded_size = std::to_string(tl.padded_size);
      out_tl.formatted_array_stride = std::to_string(tl.array_stride);
      p_formatted_lines->push_back(out_tl);
    }
  }
}

void StreamWriteTextLines(std::ostream& os, const char* indent, const std::vector<TextLine>& text_lines)
{
  std::vector<TextLine> formatted_lines;
  FormatTextLines(text_lines, indent, &formatted_lines);

  size_t line_width = 0;
  size_t offset_width = 0;
  size_t absolute_offset_width = 0;
  size_t size_width = 0;
  size_t padded_size_width = 0;
  size_t array_stride_width = 0;

  // Width
  for (auto& tl : formatted_lines) {
    if (tl.flags != 0) {
      continue;
    }
    line_width = std::max<size_t>(line_width, tl.formatted_line.length());
    offset_width = std::max<size_t>(offset_width, tl.formatted_offset.length());
    absolute_offset_width = std::max<size_t>(absolute_offset_width, tl.formatted_absolute_offset.length());
    size_width = std::max<size_t>(size_width, tl.formatted_size.length());
    padded_size_width = std::max<size_t>(padded_size_width, tl.formatted_padded_size.length());
    array_stride_width = std::max<size_t>(array_stride_width, tl.formatted_array_stride.length());
  }

  size_t n = formatted_lines.size();
  for (size_t i = 0; i < n; ++i) {
    auto& tl = formatted_lines[i];


    if (tl.flags == TEXT_LINE_TYPE_BLOCK_BEGIN) {
      if (i > 0) {
        os << "\n";
      }

      size_t pos = tl.formatted_line.find_first_not_of(' ');
      if (pos != std::string::npos) {
        std::string s(pos, ' ');
        os << s << "//" << " ";
        os << "size = " << tl.formatted_size << ", ";
        os << "padded size = " << tl.formatted_padded_size;
        os << "\n";
      }

      os << std::setw(line_width) << std::left << tl.formatted_line;
    }
    else if (tl.flags == TEXT_LINE_TYPE_BLOCK_END) {
      os << std::setw(line_width) << std::left << tl.formatted_line;
      if (i < (n - 1)) {
        os << "\n";
      }
    }
    else if (tl.flags == TEXT_LINE_TYPE_STRUCT_BEGIN) {
      if (i > 0) {
        os << "\n";
      }

      size_t pos = tl.formatted_line.find_first_not_of(' ');
      if (pos != std::string::npos) {
        std::string s(pos, ' ');
        os << s << "//" << " ";
        os << "rel offset = " << tl.formatted_offset << ", ";
        os << "abs offset = " << tl.formatted_absolute_offset << ", ";
        os << "size = " << tl.formatted_size << ", ";
        os << "padded size = " << tl.formatted_padded_size;
        if (tl.array_stride > 0) {
          os << ", ";
          os << "array stride = " << tl.formatted_array_stride;
        }
        os << "\n";
      }

      os << std::setw(line_width) << std::left << tl.formatted_line;
    }
    else if (tl.flags == TEXT_LINE_TYPE_STRUCT_END) {
      os << std::setw(line_width) << std::left << tl.formatted_line;
      if (i < (n - 1)) {
        os << "\n";
      }
    }
    else {
      os << std::setw(line_width) << std::left << tl.formatted_line;
      os << " " << "//" << " ";
      os << "rel offset = " << std::setw(offset_width) << std::right << tl.formatted_offset << ", ";
      os << "abs offset = " << std::setw(absolute_offset_width) << std::right << tl.formatted_absolute_offset << ", ";
      os << "size = " << std::setw(size_width) << std::right << tl.formatted_size << ", ";
      os << "padded size = " << std::setw(padded_size_width) << std::right << tl.formatted_padded_size;
      if (tl.array_stride > 0) {
        os << ", ";
        os << "array stride = " << std::setw(array_stride_width) << tl.formatted_array_stride;
      }
    }

    if (i < (n - 1)) {
      os << "\n";
    }
  }
}

void StreamWriteDescriptorBinding(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent = "")
{
  const char* t = indent;
  if (write_set) {
    os << t << "set     : " << obj.set << "\n";
  }
  os << t << "binding : " << obj.binding << "\n";
  os << t << "type    : " << ToStringDescriptorType(obj.descriptor_type);
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

  if (obj.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
      obj.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
    std::vector<TextLine> text_lines;
    ParseBlockVariableToTextLines("    ",  obj.block, &text_lines);
    if (!text_lines.empty()) {
      os << "\n";
      StreamWriteTextLines(os, t, text_lines);
      os << "\n";
    }
  }
}

void StreamWriteInterfaceVariable(std::ostream& os, const SpvReflectInterfaceVariable& obj, const char* indent = "")
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
  os << t << "type      : " << ToStringComponentType(*obj.type_description, 0) << "\n";
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

void StreamWriteShaderModule(std::ostream& os, const SpvReflectShaderModule& obj, const char* /*indent*/ = "")
{
  os << "generator       : " << ToStringGenerator(static_cast<Generator>(obj.generator)) << "\n";
  os << "entry point     : " << obj.entry_point_name << "\n";
  os << "source lang     : " << spvReflectSourceLanguage(obj.source_language) << "\n";
  os << "source lang ver : " << obj.source_language_version;
}

std::ostream& operator<<(std::ostream& os, const SpvReflectDescriptorBinding& obj)
{
  StreamWriteDescriptorBinding(os, obj, true, "  ");
  return os;
}

//std::ostream& operator<<(std::ostream& os, const SpvReflectDescriptorSet& obj)
//{
//  PrintDescriptorSet(os, obj, "  ");
//  os << "\n";
//  for (uint32_t i = 0; i < obj.binding_count; ++i) {
//    const auto p_binding = obj.bindings[i];
//    os << "   " << i << ":"  << "\n";
//    StreamWriteDescriptorBinding(os, *p_binding, false, "    ");
//    if (i < (obj.binding_count - 1)) {
//      os << "\n";
//    }
//  }
//  return os;
//}

std::ostream& operator<<(std::ostream& os, const spv_reflect::ShaderModule& obj)
{
  const char* t     = "  ";
  const char* tt    = "    ";
  const char* ttt   = "      ";

  StreamWriteShaderModule(os, obj.GetShaderModule(), "");

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
    os << "\n";
    os << t << "Input variables: " << count << "\n\n";
    for (size_t i = 0; i < variables.size(); ++i) {
      auto p_var = variables[i];
      assert(result == SPV_REFLECT_RESULT_SUCCESS);
      os << tt << i << ":" << "\n";
      StreamWriteInterfaceVariable(os, *p_var, ttt);
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
    os << "\n";
    os << t << "Output variables: " << count << "\n\n";
    for (size_t i = 0; i < variables.size(); ++i) {
      auto p_var = variables[i];
      assert(result == SPV_REFLECT_RESULT_SUCCESS);
      os << tt << i << ":" << "\n";
      StreamWriteInterfaceVariable(os, *p_var, ttt);
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
    os << "\n";
    os << t << "Descriptor bindings: " << count << "\n\n";
    for (size_t i = 0; i < bindings.size(); ++i) {
      auto p_binding = bindings[i];
      assert(result == SPV_REFLECT_RESULT_SUCCESS);
      os << tt << "Binding" << " " << p_binding->set << "." << p_binding->binding << "" << "\n";
      StreamWriteDescriptorBinding(os, *p_binding, true, ttt);
      if (i < (count - 1)) {
        os << "\n\n";
      }  
    }
  }

  /*
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
  */

  return os;
}