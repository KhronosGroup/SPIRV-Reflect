#include "common.h"

#include <cassert>
#include <cstring>
#include <fstream>

static const char* ToStringVkDescriptorType(VkDescriptorType value) {
  switch (value) {
    default: break;
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

static const char* ToStringGlslType(const SpvReflectTypeDescription& type)
{
  switch (type.op) {
    case SpvOpTypeVector: {
      switch (type.traits.numeric.scalar.width) {
        case 32: {
          switch (type.traits.numeric.vector.component_count) {
            case 2: return "vec2"; break;
            case 3: return "vec3"; break;
            case 4: return "vec4"; break;
          }
        }
        break;

        case 64: {
          switch (type.traits.numeric.vector.component_count) {
            case 2: return "dvec2"; break;
            case 3: return "dvec3"; break;
            case 4: return "dvec4"; break;
          }
        }
        break;
      }
    }
    break;

    default:
      break;
  }
  return "";
}

static const char* ToStringHlslType(const SpvReflectTypeDescription& type)
{
  switch (type.op) {
    case SpvOpTypeVector: {
      switch (type.traits.numeric.scalar.width) {
        case 32: {
          switch (type.traits.numeric.vector.component_count) {
            case 2: return "float2"; break;
            case 3: return "float3"; break;
            case 4: return "float4"; break;
          }
        }
        break;

        case 64: {
          switch (type.traits.numeric.vector.component_count) {
            case 2: return "double2"; break;
            case 3: return "double3"; break;
            case 4: return "double4"; break;
          }
        }
        break;
      }
    }
    break;

    default:
      break;
  }
  return "";
}

static const char* ToStringType(SpvSourceLanguage src_lang, const SpvReflectTypeDescription& type)
{
  if (src_lang == SpvSourceLanguageHLSL) {
    return ToStringHlslType(type);
  }

  return ToStringGlslType(type);
}

bool ReadFile(const std::string& file_path, std::vector<uint8_t>* /*p_buffer*/)
{
  std::ifstream is(file_path.c_str(), std::ios::binary);
  if (!is.is_open()) {
    return false;
  }

  is.seekg(0, std::ios::end);
  size_t size = is.tellg();
  is.seekg(0, std::ios::beg);  

  std::vector<char> data(size);
  is.read(data.data(), size);

  is.close();

  return true;
}

static const char* ToStringGlslBuiltIn(SpvBuiltIn built_in)
{
  switch (built_in) {
    default: break;
    case SpvBuiltInPosition                     : return ""; break;
    case SpvBuiltInPointSize                    : return ""; break;
    case SpvBuiltInClipDistance                 : return ""; break;
    case SpvBuiltInCullDistance                 : return ""; break;
    case SpvBuiltInVertexId                     : return ""; break;
    case SpvBuiltInInstanceId                   : return ""; break;
    case SpvBuiltInPrimitiveId                  : return ""; break;
    case SpvBuiltInInvocationId                 : return ""; break;
    case SpvBuiltInLayer                        : return ""; break;
    case SpvBuiltInViewportIndex                : return ""; break;
    case SpvBuiltInTessLevelOuter               : return ""; break;
    case SpvBuiltInTessLevelInner               : return ""; break;
    case SpvBuiltInTessCoord                    : return ""; break;
    case SpvBuiltInPatchVertices                : return ""; break;
    case SpvBuiltInFragCoord                    : return ""; break;
    case SpvBuiltInPointCoord                   : return ""; break;
    case SpvBuiltInFrontFacing                  : return ""; break;
    case SpvBuiltInSampleId                     : return ""; break;
    case SpvBuiltInSamplePosition               : return ""; break;
    case SpvBuiltInSampleMask                   : return ""; break;
    case SpvBuiltInFragDepth                    : return ""; break;
    case SpvBuiltInHelperInvocation             : return ""; break;
    case SpvBuiltInNumWorkgroups                : return ""; break;
    case SpvBuiltInWorkgroupSize                : return ""; break;
    case SpvBuiltInWorkgroupId                  : return ""; break;
    case SpvBuiltInLocalInvocationId            : return ""; break;
    case SpvBuiltInGlobalInvocationId           : return ""; break;
    case SpvBuiltInLocalInvocationIndex         : return ""; break;
    case SpvBuiltInWorkDim                      : return ""; break;
    case SpvBuiltInGlobalSize                   : return ""; break;
    case SpvBuiltInEnqueuedWorkgroupSize        : return ""; break;
    case SpvBuiltInGlobalOffset                 : return ""; break;
    case SpvBuiltInGlobalLinearId               : return ""; break;
    case SpvBuiltInSubgroupSize                 : return ""; break;
    case SpvBuiltInSubgroupMaxSize              : return ""; break;
    case SpvBuiltInNumSubgroups                 : return ""; break;
    case SpvBuiltInNumEnqueuedSubgroups         : return ""; break;
    case SpvBuiltInSubgroupId                   : return ""; break;
    case SpvBuiltInSubgroupLocalInvocationId    : return ""; break;
    case SpvBuiltInVertexIndex                  : return ""; break;
    case SpvBuiltInInstanceIndex                : return ""; break;
    case SpvBuiltInSubgroupEqMaskKHR            : return ""; break;
    case SpvBuiltInSubgroupGeMaskKHR            : return ""; break;
    case SpvBuiltInSubgroupGtMaskKHR            : return ""; break;
    case SpvBuiltInSubgroupLeMaskKHR            : return ""; break;
    case SpvBuiltInSubgroupLtMaskKHR            : return ""; break;
    case SpvBuiltInBaseVertex                   : return ""; break;
    case SpvBuiltInBaseInstance                 : return ""; break;
    case SpvBuiltInDrawIndex                    : return ""; break;
    case SpvBuiltInDeviceIndex                  : return ""; break;
    case SpvBuiltInViewIndex                    : return ""; break;
    case SpvBuiltInBaryCoordNoPerspAMD          : return ""; break;
    case SpvBuiltInBaryCoordNoPerspCentroidAMD  : return ""; break;
    case SpvBuiltInBaryCoordNoPerspSampleAMD    : return ""; break;
    case SpvBuiltInBaryCoordSmoothAMD           : return ""; break;
    case SpvBuiltInBaryCoordSmoothCentroidAMD   : return ""; break;
    case SpvBuiltInBaryCoordSmoothSampleAMD     : return ""; break;
    case SpvBuiltInBaryCoordPullModelAMD        : return ""; break;
    case SpvBuiltInFragStencilRefEXT            : return ""; break;
    case SpvBuiltInViewportMaskNV               : return ""; break;
    case SpvBuiltInSecondaryPositionNV          : return ""; break;
    case SpvBuiltInSecondaryViewportMaskNV      : return ""; break;
    case SpvBuiltInPositionPerViewNV            : return ""; break;
    case SpvBuiltInViewportMaskPerViewNV        : return ""; break;
  }
  return "";
}

static const char* ToStringHlslBuiltIn(SpvBuiltIn built_in)
{
  switch (built_in) {
    default: break;
    case SpvBuiltInPosition                     : return "SV_POSITION"; break;
    case SpvBuiltInPointSize                    : return ""; break;
    case SpvBuiltInClipDistance                 : return ""; break;
    case SpvBuiltInCullDistance                 : return ""; break;
    case SpvBuiltInVertexId                     : return ""; break;
    case SpvBuiltInInstanceId                   : return ""; break;
    case SpvBuiltInPrimitiveId                  : return ""; break;
    case SpvBuiltInInvocationId                 : return ""; break;
    case SpvBuiltInLayer                        : return ""; break;
    case SpvBuiltInViewportIndex                : return ""; break;
    case SpvBuiltInTessLevelOuter               : return ""; break;
    case SpvBuiltInTessLevelInner               : return ""; break;
    case SpvBuiltInTessCoord                    : return ""; break;
    case SpvBuiltInPatchVertices                : return ""; break;
    case SpvBuiltInFragCoord                    : return "SV_POSITION"; break;
    case SpvBuiltInPointCoord                   : return ""; break;
    case SpvBuiltInFrontFacing                  : return ""; break;
    case SpvBuiltInSampleId                     : return ""; break;
    case SpvBuiltInSamplePosition               : return ""; break;
    case SpvBuiltInSampleMask                   : return ""; break;
    case SpvBuiltInFragDepth                    : return ""; break;
    case SpvBuiltInHelperInvocation             : return ""; break;
    case SpvBuiltInNumWorkgroups                : return ""; break;
    case SpvBuiltInWorkgroupSize                : return ""; break;
    case SpvBuiltInWorkgroupId                  : return ""; break;
    case SpvBuiltInLocalInvocationId            : return ""; break;
    case SpvBuiltInGlobalInvocationId           : return ""; break;
    case SpvBuiltInLocalInvocationIndex         : return ""; break;
    case SpvBuiltInWorkDim                      : return ""; break;
    case SpvBuiltInGlobalSize                   : return ""; break;
    case SpvBuiltInEnqueuedWorkgroupSize        : return ""; break;
    case SpvBuiltInGlobalOffset                 : return ""; break;
    case SpvBuiltInGlobalLinearId               : return ""; break;
    case SpvBuiltInSubgroupSize                 : return ""; break;
    case SpvBuiltInSubgroupMaxSize              : return ""; break;
    case SpvBuiltInNumSubgroups                 : return ""; break;
    case SpvBuiltInNumEnqueuedSubgroups         : return ""; break;
    case SpvBuiltInSubgroupId                   : return ""; break;
    case SpvBuiltInSubgroupLocalInvocationId    : return ""; break;
    case SpvBuiltInVertexIndex                  : return ""; break;
    case SpvBuiltInInstanceIndex                : return ""; break;
    case SpvBuiltInSubgroupEqMaskKHR            : return ""; break;
    case SpvBuiltInSubgroupGeMaskKHR            : return ""; break;
    case SpvBuiltInSubgroupGtMaskKHR            : return ""; break;
    case SpvBuiltInSubgroupLeMaskKHR            : return ""; break;
    case SpvBuiltInSubgroupLtMaskKHR            : return ""; break;
    case SpvBuiltInBaseVertex                   : return ""; break;
    case SpvBuiltInBaseInstance                 : return ""; break;
    case SpvBuiltInDrawIndex                    : return ""; break;
    case SpvBuiltInDeviceIndex                  : return ""; break;
    case SpvBuiltInViewIndex                    : return ""; break;
    case SpvBuiltInBaryCoordNoPerspAMD          : return ""; break;
    case SpvBuiltInBaryCoordNoPerspCentroidAMD  : return ""; break;
    case SpvBuiltInBaryCoordNoPerspSampleAMD    : return ""; break;
    case SpvBuiltInBaryCoordSmoothAMD           : return ""; break;
    case SpvBuiltInBaryCoordSmoothCentroidAMD   : return ""; break;
    case SpvBuiltInBaryCoordSmoothSampleAMD     : return ""; break;
    case SpvBuiltInBaryCoordPullModelAMD        : return ""; break;
    case SpvBuiltInFragStencilRefEXT            : return ""; break;
    case SpvBuiltInViewportMaskNV               : return ""; break;
    case SpvBuiltInSecondaryPositionNV          : return ""; break;
    case SpvBuiltInSecondaryViewportMaskNV      : return ""; break;
    case SpvBuiltInPositionPerViewNV            : return ""; break;
    case SpvBuiltInViewportMaskPerViewNV        : return ""; break;
  }
  return "";
}

static const char* ToStringBuiltIn(SpvSourceLanguage src_lang, SpvBuiltIn built_in)
{
  if (src_lang == SpvSourceLanguageHLSL) {
    return ToStringHlslBuiltIn(built_in);
  }

  return ToStringGlslBuiltIn(built_in);
}

void PrintModuleInfo(std::ostream& os, const SpvReflectShaderModule& obj, const char* /*indent*/)
{
  os << "entry point     : " << obj.entry_point_name << "\n";
  os << "source lang     : " << spvReflectSourceLanguage(obj.source_language) << "\n";
  os << "source lang ver : " << obj.source_language_version << "\n";
  if (obj.source_language == SpvSourceLanguageHLSL) {
    os << "stage           : ";
    switch (obj.vulkan_shader_stage) {
      default: break;
      case VK_SHADER_STAGE_VERTEX_BIT                   : os << "VS"; break;
      case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT     : os << "HS"; break;
      case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT  : os << "DS"; break;
      case VK_SHADER_STAGE_GEOMETRY_BIT                 : os << "GS"; break;
      case VK_SHADER_STAGE_FRAGMENT_BIT                 : os << "PS"; break;
      case VK_SHADER_STAGE_COMPUTE_BIT                  : os << "CS"; break;
    }
  }
}

void PrintDescriptorSet(std::ostream& os, const SpvReflectDescriptorSet& obj, const char* indent)
{
  const char* t     = indent;
  std::string tt    = std::string(indent) + "  ";
  std::string ttttt = std::string(indent) + "    ";

  os << t << "set           : " << obj.set << "\n";
  os << t << "binding count : " << obj.binding_count;
  os << "\n";
  for (uint32_t i = 0; i < obj.binding_count; ++i) {
    const SpvReflectDescriptorBinding& binding = *obj.bindings[i];
    os << tt << i << ":" << "\n";
    PrintDescriptorBinding(os, binding, false, ttttt.c_str());
    if (i < (obj.binding_count - 1)) {
      os << "\n";
    }
  }
}

void PrintDescriptorBinding(std::ostream& os, const SpvReflectDescriptorBinding& obj, bool write_set, const char* indent)
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
}

void PrintInterfaceVariable(std::ostream& os, SpvSourceLanguage src_lang, const SpvReflectInterfaceVariable& obj, const char* indent)
{
  const char* t = indent;
  os << t << "location  : ";
  if (obj.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
    os << ToStringBuiltIn(src_lang, obj.built_in) << " " << "(built-in)";
  }
  else {
    os << obj.location;
  }
  os << "\n";
  os << t << "type      : " << ToStringType(src_lang, *obj.type_description) << "\n";
  os << t << "qualifier : ";
  if (obj.decoration_flags & SPV_REFLECT_DECORATION_FLAT) {
    os << "flat";
  }
  else   if (obj.decoration_flags & SPV_REFLECT_DECORATION_NOPERSPECTIVE) {
    os << "noperspective";
  }
  os << "\n";

  os << t << "name      : " << obj.name;
  if ((obj.type_description->type_name != nullptr) && (strlen(obj.type_description->type_name) > 0)) {
    os << " " << "(" << obj.type_description->type_name << ")";
  }
}
