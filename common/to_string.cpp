#include "to_string.h"
#include <cassert>
#include <sstream>


std::string ToStringShaderStage(SpvReflectShaderStageFlagBits stage)
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

std::string ToStringHlslResourceType(SpvReflectResourceType type)
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

std::string ToStringDescriptorType(SpvReflectDescriptorType value) 
{
  switch (value) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER                : return "SAMPLER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : return "COMBINED_IMAGE_SAMPLER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE          : return "SAMPLED_IMAGE";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE          : return "STORAGE_IMAGE";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   : return "UNIFORM_TEXEL_BUFFER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   : return "STORAGE_TEXEL_BUFFER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER         : return "UNIFORM_BUFFER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER         : return "STORAGE_BUFFER";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : return "UNIFORM_BUFFER_DYNAMIC";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : return "STORAGE_BUFFER_DYNAMIC";
    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       : return "INPUT_ATTACHMENT";
  }
  return "";
}

std::string ToStringFormat(SpvReflectFormat format)
{
  switch (format) {
    case SPV_REFLECT_FORMAT_UNDEFINED           : return "UNDEFINED"; break;
    case SPV_REFLECT_FORMAT_R32_UINT            : return "R32_UINT"; break;
    case SPV_REFLECT_FORMAT_R32_SINT            : return "R32_SINT"; break;
    case SPV_REFLECT_FORMAT_R32_SFLOAT          : return "R32_SFLOAT"; break;
    case SPV_REFLECT_FORMAT_R32G32_UINT         : return "R32G32_UINT"; break;
    case SPV_REFLECT_FORMAT_R32G32_SINT         : return "R32G32_SINT"; break;
    case SPV_REFLECT_FORMAT_R32G32_SFLOAT       : return "R32G32_SFLOAT"; break;
    case SPV_REFLECT_FORMAT_R32G32B32_UINT      : return "R32G32B32_UINT"; break;
    case SPV_REFLECT_FORMAT_R32G32B32_SINT      : return "R32G32B32_SINT"; break;
    case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT    : return "R32G32B32_SFLOAT"; break;
    case SPV_REFLECT_FORMAT_R32G32B32A32_UINT   : return "R32G32B32A32_UINT"; break;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SINT   : return "R32G32B32A32_SINT"; break;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT : return "R32G32B32A32_SFLOAT"; break;
  }
  return "UNKNOWN";
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

std::string ToStringStorageClass(SpvStorageClass value)
{
  switch (value)
  {
    case SpvStorageClassUniformConstant: return "UniformConstant"; break;
    case SpvStorageClassInput: return "Input"; break;
    case SpvStorageClassUniform: return "Uniform"; break;
    case SpvStorageClassOutput: return "Output"; break;
    case SpvStorageClassWorkgroup: return "Workgroup"; break;
    case SpvStorageClassCrossWorkgroup: return "CrossWorkgroup"; break;
    case SpvStorageClassPrivate: return "Private"; break;
    case SpvStorageClassFunction: return "Function"; break;
    case SpvStorageClassGeneric: return "Generic"; break;
    case SpvStorageClassPushConstant: return "PushConstant"; break;
    case SpvStorageClassAtomicCounter: return "AtomicCounter"; break;
    case SpvStorageClassImage: return "Image"; break;
    case SpvStorageClassStorageBuffer: return "StorageBuffer"; break;
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