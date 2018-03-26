#include "common.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>

static std::string AsHexString(uint32_t n) {
  // std::iomanip can die in a fire.
  char out_word[11];
  int len = snprintf(out_word, 11, "0x%08X", n);
  assert(len == 10);
  (void)len;
  return std::string(out_word);
}

static const char* ToStringSpvSourceLanguage(SpvSourceLanguage lang) {
  switch(lang) {
    case SpvSourceLanguageUnknown: return "Unknown";
    case SpvSourceLanguageESSL: return "ESSL";
    case SpvSourceLanguageGLSL: return "GLSL";
    case SpvSourceLanguageOpenCL_C: return "OpenCL_C";
    case SpvSourceLanguageOpenCL_CPP: return "OpenCL_CPP";
    case SpvSourceLanguageHLSL: return "HLSL";

    case SpvSourceLanguageMax:
      break;
  }
  // unhandled SpvSourceLanguage enum value
  return "???";
}

static const char* ToStringSpvExecutionModel(SpvExecutionModel model) {
  switch(model) {
    case SpvExecutionModelVertex: return "Vertex";
    case SpvExecutionModelTessellationControl: return "TessellationControl";
    case SpvExecutionModelTessellationEvaluation: return "TessellationEvaluation";
    case SpvExecutionModelGeometry: return "Geometry";
    case SpvExecutionModelFragment: return "Fragment";
    case SpvExecutionModelGLCompute: return "GLCompute";
    case SpvExecutionModelKernel: return "Kernel";
    case SpvExecutionModelMax:
      break;
  }
  // unhandled SpvExecutionModel enum value
  return "???";
}

static const char* ToStringSpvStorageClass(SpvStorageClass storage_class) {
  switch(storage_class) {
    case SpvStorageClassUniformConstant: return "UniformConstant";
    case SpvStorageClassInput: return "Input";
    case SpvStorageClassUniform: return "Uniform";
    case SpvStorageClassOutput: return "Output";
    case SpvStorageClassWorkgroup: return "Workgroup";
    case SpvStorageClassCrossWorkgroup: return "CrossWorkgroup";
    case SpvStorageClassPrivate: return "Private";
    case SpvStorageClassFunction: return "Function";
    case SpvStorageClassGeneric: return "Generic";
    case SpvStorageClassPushConstant: return "PushConstant";
    case SpvStorageClassAtomicCounter: return "AtomicCounter";
    case SpvStorageClassImage: return "Image";
    case SpvStorageClassStorageBuffer: return "StorageBuffer";

    case SpvStorageClassMax:
      break;
  }

  // Special case: this specific "unhandled" value does actually seem to show up.
  if (storage_class == (SpvStorageClass)-1) {
    return "NOT APPLICABLE";
  }

  // unhandled SpvStorageClass enum value
  return "???";
}

static const char* ToStringSpvDim(SpvDim dim) {
  switch(dim) {
    case SpvDim1D: return "1D";
    case SpvDim2D: return "2D";
    case SpvDim3D: return "3D";
    case SpvDimCube: return "Cube";
    case SpvDimRect: return "Rect";
    case SpvDimBuffer: return "Buffer";
    case SpvDimSubpassData: return "SubpassData";

    case SpvDimMax:
      break;
  }
  // unhandled SpvDim enum value
  return "???";
}

static const char* ToStringSpvBuiltIn(SpvBuiltIn built_in) {
  switch (built_in) {
      case SpvBuiltInPosition: return "Position";
      case SpvBuiltInPointSize: return "PointSize";
      case SpvBuiltInClipDistance: return "ClipDistance";
      case SpvBuiltInCullDistance: return "CullDistance";
      case SpvBuiltInVertexId: return "VertexId";
      case SpvBuiltInInstanceId: return "InstanceId";
      case SpvBuiltInPrimitiveId: return "PrimitiveId";
      case SpvBuiltInInvocationId: return "InvocationId";
      case SpvBuiltInLayer: return "Layer";
      case SpvBuiltInViewportIndex: return "ViewportIndex";
      case SpvBuiltInTessLevelOuter: return "TessLevelOuter";
      case SpvBuiltInTessLevelInner: return "TessLevelInner";
      case SpvBuiltInTessCoord: return "TessCoord";
      case SpvBuiltInPatchVertices: return "PatchVertices";
      case SpvBuiltInFragCoord: return "FragCoord";
      case SpvBuiltInPointCoord: return "PointCoord";
      case SpvBuiltInFrontFacing: return "FrontFacing";
      case SpvBuiltInSampleId: return "SampleId";
      case SpvBuiltInSamplePosition: return "SamplePosition";
      case SpvBuiltInSampleMask: return "SampleMask";
      case SpvBuiltInFragDepth: return "FragDepth";
      case SpvBuiltInHelperInvocation: return "HelperInvocation";
      case SpvBuiltInNumWorkgroups: return "NumWorkgroups";
      case SpvBuiltInWorkgroupSize: return "WorkgroupSize";
      case SpvBuiltInWorkgroupId: return "WorkgroupId";
      case SpvBuiltInLocalInvocationId: return "LocalInvocationId";
      case SpvBuiltInGlobalInvocationId: return "GlobalInvocationId";
      case SpvBuiltInLocalInvocationIndex: return "LocalInvocationIndex";
      case SpvBuiltInWorkDim: return "WorkDim";
      case SpvBuiltInGlobalSize: return "GlobalSize";
      case SpvBuiltInEnqueuedWorkgroupSize: return "EnqueuedWorkgroupSize";
      case SpvBuiltInGlobalOffset: return "GlobalOffset";
      case SpvBuiltInGlobalLinearId: return "GlobalLinearId";
      case SpvBuiltInSubgroupSize: return "SubgroupSize";
      case SpvBuiltInSubgroupMaxSize: return "SubgroupMaxSize";
      case SpvBuiltInNumSubgroups: return "NumSubgroups";
      case SpvBuiltInNumEnqueuedSubgroups: return "NumEnqueuedSubgroups";
      case SpvBuiltInSubgroupId: return "SubgroupId";
      case SpvBuiltInSubgroupLocalInvocationId: return "SubgroupLocalInvocationId";
      case SpvBuiltInVertexIndex: return "VertexIndex";
      case SpvBuiltInInstanceIndex: return "InstanceIndex";
      case SpvBuiltInSubgroupEqMaskKHR: return "SubgroupEqMaskKHR";
      case SpvBuiltInSubgroupGeMaskKHR: return "SubgroupGeMaskKHR";
      case SpvBuiltInSubgroupGtMaskKHR: return "SubgroupGtMaskKHR";
      case SpvBuiltInSubgroupLeMaskKHR: return "SubgroupLeMaskKHR";
      case SpvBuiltInSubgroupLtMaskKHR: return "SubgroupLtMaskKHR";
      case SpvBuiltInBaseVertex: return "BaseVertex";
      case SpvBuiltInBaseInstance: return "BaseInstance";
      case SpvBuiltInDrawIndex: return "DrawIndex";
      case SpvBuiltInDeviceIndex: return "DeviceIndex";
      case SpvBuiltInViewIndex: return "ViewIndex";
      case SpvBuiltInBaryCoordNoPerspAMD: return "BaryCoordNoPerspAMD";
      case SpvBuiltInBaryCoordNoPerspCentroidAMD: return "BaryCoordNoPerspCentroidAMD";
      case SpvBuiltInBaryCoordNoPerspSampleAMD: return "BaryCoordNoPerspSampleAMD";
      case SpvBuiltInBaryCoordSmoothAMD: return "BaryCoordSmoothAMD";
      case SpvBuiltInBaryCoordSmoothCentroidAMD: return "BaryCoordSmoothCentroidAMD";
      case SpvBuiltInBaryCoordSmoothSampleAMD: return "BaryCoordSmoothSampleAMD";
      case SpvBuiltInBaryCoordPullModelAMD: return "BaryCoordPullModelAMD";
      case SpvBuiltInFragStencilRefEXT: return "FragStencilRefEXT";
      case SpvBuiltInViewportMaskNV: return "ViewportMaskNV";
      case SpvBuiltInSecondaryPositionNV: return "SecondaryPositionNV";
      case SpvBuiltInSecondaryViewportMaskNV: return "SecondaryViewportMaskNV";
      case SpvBuiltInPositionPerViewNV: return "PositionPerViewNV";
      case SpvBuiltInViewportMaskPerViewNV: return "ViewportMaskPerViewNV";

      case SpvBuiltInMax:
        break;
  }
  // unhandled SpvBuiltIn enum value
  return "???";
}

static const char* ToStringSpvImageFormat(SpvImageFormat fmt) {
  switch(fmt) {
    case SpvImageFormatUnknown: return "Unknown";
    case SpvImageFormatRgba32f: return "Rgba32f";
    case SpvImageFormatRgba16f: return "Rgba16f";
    case SpvImageFormatR32f: return "R32f";
    case SpvImageFormatRgba8: return "Rgba8";
    case SpvImageFormatRgba8Snorm: return "Rgba8Snorm";
    case SpvImageFormatRg32f: return "Rg32f";
    case SpvImageFormatRg16f: return "Rg16f";
    case SpvImageFormatR11fG11fB10f: return "R11fG11fB10f";
    case SpvImageFormatR16f: return "R16f";
    case SpvImageFormatRgba16: return "Rgba16";
    case SpvImageFormatRgb10A2: return "Rgb10A2";
    case SpvImageFormatRg16: return "Rg16";
    case SpvImageFormatRg8: return "Rg8";
    case SpvImageFormatR16: return "R16";
    case SpvImageFormatR8: return "R8";
    case SpvImageFormatRgba16Snorm: return "Rgba16Snorm";
    case SpvImageFormatRg16Snorm: return "Rg16Snorm";
    case SpvImageFormatRg8Snorm: return "Rg8Snorm";
    case SpvImageFormatR16Snorm: return "R16Snorm";
    case SpvImageFormatR8Snorm: return "R8Snorm";
    case SpvImageFormatRgba32i: return "Rgba32i";
    case SpvImageFormatRgba16i: return "Rgba16i";
    case SpvImageFormatRgba8i: return "Rgba8i";
    case SpvImageFormatR32i: return "R32i";
    case SpvImageFormatRg32i: return "Rg32i";
    case SpvImageFormatRg16i: return "Rg16i";
    case SpvImageFormatRg8i: return "Rg8i";
    case SpvImageFormatR16i: return "R16i";
    case SpvImageFormatR8i: return "R8i";
    case SpvImageFormatRgba32ui: return "Rgba32ui";
    case SpvImageFormatRgba16ui: return "Rgba16ui";
    case SpvImageFormatRgba8ui: return "Rgba8ui";
    case SpvImageFormatR32ui: return "R32ui";
    case SpvImageFormatRgb10a2ui: return "Rgb10a2ui";
    case SpvImageFormatRg32ui: return "Rg32ui";
    case SpvImageFormatRg16ui: return "Rg16ui";
    case SpvImageFormatRg8ui: return "Rg8ui";
    case SpvImageFormatR16ui: return "R16ui";
    case SpvImageFormatR8ui: return "R8ui";

    case SpvImageFormatMax:
      break;
  }
  // unhandled SpvImageFormat enum value
  return "???";
}

static const char* ToStringSpvReflectResourceType(SpvReflectResourceType res_type) {
  switch(res_type) {
  case SPV_REFLECT_RESOURCE_FLAG_UNDEFINED: return "UNDEFINED";
  case SPV_REFLECT_RESOURCE_FLAG_SAMPLER: return "SAMPLER";
  case SPV_REFLECT_RESOURCE_FLAG_CBV: return "CBV";
  case SPV_REFLECT_RESOURCE_FLAG_SRV: return "SRV";
  case SPV_REFLECT_RESOURCE_FLAG_UAV: return "UAV";
  }
  // unhandled SpvReflectResourceType enum value
  return "???";
}

static std::string ToStringSpvReflectTypeFlags(SpvReflectTypeFlags type_flags) {
  if (type_flags == SPV_REFLECT_TYPE_FLAG_UNDEFINED) {
    return "UNDEFINED";
  }

#define PRINT_AND_CLEAR_TYPE_FLAG(stream, flags, bit) \
  if (( (flags) & (SPV_REFLECT_TYPE_FLAG_##bit) ) == (SPV_REFLECT_TYPE_FLAG_##bit)) { \
    stream << #bit << " "; \
    flags ^= SPV_REFLECT_TYPE_FLAG_##bit; \
  }
  std::stringstream sstream;
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, ARRAY);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, STRUCT);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, EXTERNAL_MASK);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, EXTERNAL_BLOCK);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, EXTERNAL_SAMPLED_IMAGE);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, EXTERNAL_SAMPLER);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, EXTERNAL_IMAGE);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, MATRIX);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, VECTOR);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, FLOAT);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, INT);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, BOOL);
  PRINT_AND_CLEAR_TYPE_FLAG(sstream, type_flags, VOID);
#undef PRINT_AND_CLEAR_TYPE_FLAG
  if (type_flags != 0) {
    // Unhandled SpvReflectTypeFlags bit
    sstream << "???";
  }
  return sstream.str();
}

static std::string ToStringSpvReflectDecorationFlags(SpvReflectDecorationFlags decoration_flags) {
  if (decoration_flags == SPV_REFLECT_DECORATION_NONE) {
    return "NONE";
  }

#define PRINT_AND_CLEAR_DECORATION_FLAG(stream, flags, bit) \
  if (( (flags) & (SPV_REFLECT_DECORATION_##bit) ) == (SPV_REFLECT_DECORATION_##bit)) { \
    stream << #bit << " "; \
    flags ^= SPV_REFLECT_DECORATION_##bit; \
  }
  std::stringstream sstream;
  PRINT_AND_CLEAR_DECORATION_FLAG(sstream, decoration_flags, NON_WRITABLE);
  PRINT_AND_CLEAR_DECORATION_FLAG(sstream, decoration_flags, FLAT);
  PRINT_AND_CLEAR_DECORATION_FLAG(sstream, decoration_flags, NOPERSPECTIVE);
  PRINT_AND_CLEAR_DECORATION_FLAG(sstream, decoration_flags, BUILT_IN);
  PRINT_AND_CLEAR_DECORATION_FLAG(sstream, decoration_flags, COLUMN_MAJOR);
  PRINT_AND_CLEAR_DECORATION_FLAG(sstream, decoration_flags, ROW_MAJOR);
  PRINT_AND_CLEAR_DECORATION_FLAG(sstream, decoration_flags, BUFFER_BLOCK);
  PRINT_AND_CLEAR_DECORATION_FLAG(sstream, decoration_flags, BLOCK);
#undef PRINT_AND_CLEAR_DECORATION_FLAG
  if (decoration_flags != 0) {
    // Unhandled SpvReflectDecorationFlags bit
    sstream << "???";
  }
  return sstream.str();
}

static const char* ToStringVkShaderStageFlagBits(VkShaderStageFlagBits stage) {
  switch (stage) {
  case VK_SHADER_STAGE_VERTEX_BIT: return "VERTEX";
  case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return "TESSELLATION_CONTROL";
  case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return "TESSELLATION_EVALUATION";
  case VK_SHADER_STAGE_GEOMETRY_BIT: return "GEOMETRY";
  case VK_SHADER_STAGE_FRAGMENT_BIT: return "FRAGMENT";
  case VK_SHADER_STAGE_COMPUTE_BIT: return "COMPUTE";
  case VK_SHADER_STAGE_ALL_GRAPHICS: return "ALL_GRAPHICS";
  case VK_SHADER_STAGE_ALL: return "ALL";
  }
  // Unhandled VkShaderStageFlagBits enum value
  return "???";
}

static const char* ToStringVkDescriptorType(VkDescriptorType value) {
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
  // unhandled VkDescriptorType enum value
  return "VK_DESCRIPTOR_TYPE_???";
}

static const char* ToStringVkFormat(VkFormat fmt) {
  switch(fmt) {
      case VK_FORMAT_UNDEFINED: return "VK_FORMAT_UNDEFINED";
      case VK_FORMAT_R4G4_UNORM_PACK8: return "VK_FORMAT_R4G4_UNORM_PACK8";
      case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
      case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
      case VK_FORMAT_R5G6B5_UNORM_PACK16: return "VK_FORMAT_R5G6B5_UNORM_PACK16";
      case VK_FORMAT_B5G6R5_UNORM_PACK16: return "VK_FORMAT_B5G6R5_UNORM_PACK16";
      case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
      case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
      case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
      case VK_FORMAT_R8_UNORM: return "VK_FORMAT_R8_UNORM";
      case VK_FORMAT_R8_SNORM: return "VK_FORMAT_R8_SNORM";
      case VK_FORMAT_R8_USCALED: return "VK_FORMAT_R8_USCALED";
      case VK_FORMAT_R8_SSCALED: return "VK_FORMAT_R8_SSCALED";
      case VK_FORMAT_R8_UINT: return "VK_FORMAT_R8_UINT";
      case VK_FORMAT_R8_SINT: return "VK_FORMAT_R8_SINT";
      case VK_FORMAT_R8_SRGB: return "VK_FORMAT_R8_SRGB";
      case VK_FORMAT_R8G8_UNORM: return "VK_FORMAT_R8G8_UNORM";
      case VK_FORMAT_R8G8_SNORM: return "VK_FORMAT_R8G8_SNORM";
      case VK_FORMAT_R8G8_USCALED: return "VK_FORMAT_R8G8_USCALED";
      case VK_FORMAT_R8G8_SSCALED: return "VK_FORMAT_R8G8_SSCALED";
      case VK_FORMAT_R8G8_UINT: return "VK_FORMAT_R8G8_UINT";
      case VK_FORMAT_R8G8_SINT: return "VK_FORMAT_R8G8_SINT";
      case VK_FORMAT_R8G8_SRGB: return "VK_FORMAT_R8G8_SRGB";
      case VK_FORMAT_R8G8B8_UNORM: return "VK_FORMAT_R8G8B8_UNORM";
      case VK_FORMAT_R8G8B8_SNORM: return "VK_FORMAT_R8G8B8_SNORM";
      case VK_FORMAT_R8G8B8_USCALED: return "VK_FORMAT_R8G8B8_USCALED";
      case VK_FORMAT_R8G8B8_SSCALED: return "VK_FORMAT_R8G8B8_SSCALED";
      case VK_FORMAT_R8G8B8_UINT: return "VK_FORMAT_R8G8B8_UINT";
      case VK_FORMAT_R8G8B8_SINT: return "VK_FORMAT_R8G8B8_SINT";
      case VK_FORMAT_R8G8B8_SRGB: return "VK_FORMAT_R8G8B8_SRGB";
      case VK_FORMAT_B8G8R8_UNORM: return "VK_FORMAT_B8G8R8_UNORM";
      case VK_FORMAT_B8G8R8_SNORM: return "VK_FORMAT_B8G8R8_SNORM";
      case VK_FORMAT_B8G8R8_USCALED: return "VK_FORMAT_B8G8R8_USCALED";
      case VK_FORMAT_B8G8R8_SSCALED: return "VK_FORMAT_B8G8R8_SSCALED";
      case VK_FORMAT_B8G8R8_UINT: return "VK_FORMAT_B8G8R8_UINT";
      case VK_FORMAT_B8G8R8_SINT: return "VK_FORMAT_B8G8R8_SINT";
      case VK_FORMAT_B8G8R8_SRGB: return "VK_FORMAT_B8G8R8_SRGB";
      case VK_FORMAT_R8G8B8A8_UNORM: return "VK_FORMAT_R8G8B8A8_UNORM";
      case VK_FORMAT_R8G8B8A8_SNORM: return "VK_FORMAT_R8G8B8A8_SNORM";
      case VK_FORMAT_R8G8B8A8_USCALED: return "VK_FORMAT_R8G8B8A8_USCALED";
      case VK_FORMAT_R8G8B8A8_SSCALED: return "VK_FORMAT_R8G8B8A8_SSCALED";
      case VK_FORMAT_R8G8B8A8_UINT: return "VK_FORMAT_R8G8B8A8_UINT";
      case VK_FORMAT_R8G8B8A8_SINT: return "VK_FORMAT_R8G8B8A8_SINT";
      case VK_FORMAT_R8G8B8A8_SRGB: return "VK_FORMAT_R8G8B8A8_SRGB";
      case VK_FORMAT_B8G8R8A8_UNORM: return "VK_FORMAT_B8G8R8A8_UNORM";
      case VK_FORMAT_B8G8R8A8_SNORM: return "VK_FORMAT_B8G8R8A8_SNORM";
      case VK_FORMAT_B8G8R8A8_USCALED: return "VK_FORMAT_B8G8R8A8_USCALED";
      case VK_FORMAT_B8G8R8A8_SSCALED: return "VK_FORMAT_B8G8R8A8_SSCALED";
      case VK_FORMAT_B8G8R8A8_UINT: return "VK_FORMAT_B8G8R8A8_UINT";
      case VK_FORMAT_B8G8R8A8_SINT: return "VK_FORMAT_B8G8R8A8_SINT";
      case VK_FORMAT_B8G8R8A8_SRGB: return "VK_FORMAT_B8G8R8A8_SRGB";
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
      case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
      case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
      case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
      case VK_FORMAT_A8B8G8R8_UINT_PACK32: return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
      case VK_FORMAT_A8B8G8R8_SINT_PACK32: return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
      case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
      case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
      case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
      case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
      case VK_FORMAT_A2R10G10B10_UINT_PACK32: return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
      case VK_FORMAT_A2R10G10B10_SINT_PACK32: return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
      case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
      case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
      case VK_FORMAT_A2B10G10R10_UINT_PACK32: return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
      case VK_FORMAT_A2B10G10R10_SINT_PACK32: return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
      case VK_FORMAT_R16_UNORM: return "VK_FORMAT_R16_UNORM";
      case VK_FORMAT_R16_SNORM: return "VK_FORMAT_R16_SNORM";
      case VK_FORMAT_R16_USCALED: return "VK_FORMAT_R16_USCALED";
      case VK_FORMAT_R16_SSCALED: return "VK_FORMAT_R16_SSCALED";
      case VK_FORMAT_R16_UINT: return "VK_FORMAT_R16_UINT";
      case VK_FORMAT_R16_SINT: return "VK_FORMAT_R16_SINT";
      case VK_FORMAT_R16_SFLOAT: return "VK_FORMAT_R16_SFLOAT";
      case VK_FORMAT_R16G16_UNORM: return "VK_FORMAT_R16G16_UNORM";
      case VK_FORMAT_R16G16_SNORM: return "VK_FORMAT_R16G16_SNORM";
      case VK_FORMAT_R16G16_USCALED: return "VK_FORMAT_R16G16_USCALED";
      case VK_FORMAT_R16G16_SSCALED: return "VK_FORMAT_R16G16_SSCALED";
      case VK_FORMAT_R16G16_UINT: return "VK_FORMAT_R16G16_UINT";
      case VK_FORMAT_R16G16_SINT: return "VK_FORMAT_R16G16_SINT";
      case VK_FORMAT_R16G16_SFLOAT: return "VK_FORMAT_R16G16_SFLOAT";
      case VK_FORMAT_R16G16B16_UNORM: return "VK_FORMAT_R16G16B16_UNORM";
      case VK_FORMAT_R16G16B16_SNORM: return "VK_FORMAT_R16G16B16_SNORM";
      case VK_FORMAT_R16G16B16_USCALED: return "VK_FORMAT_R16G16B16_USCALED";
      case VK_FORMAT_R16G16B16_SSCALED: return "VK_FORMAT_R16G16B16_SSCALED";
      case VK_FORMAT_R16G16B16_UINT: return "VK_FORMAT_R16G16B16_UINT";
      case VK_FORMAT_R16G16B16_SINT: return "VK_FORMAT_R16G16B16_SINT";
      case VK_FORMAT_R16G16B16_SFLOAT: return "VK_FORMAT_R16G16B16_SFLOAT";
      case VK_FORMAT_R16G16B16A16_UNORM: return "VK_FORMAT_R16G16B16A16_UNORM";
      case VK_FORMAT_R16G16B16A16_SNORM: return "VK_FORMAT_R16G16B16A16_SNORM";
      case VK_FORMAT_R16G16B16A16_USCALED: return "VK_FORMAT_R16G16B16A16_USCALED";
      case VK_FORMAT_R16G16B16A16_SSCALED: return "VK_FORMAT_R16G16B16A16_SSCALED";
      case VK_FORMAT_R16G16B16A16_UINT: return "VK_FORMAT_R16G16B16A16_UINT";
      case VK_FORMAT_R16G16B16A16_SINT: return "VK_FORMAT_R16G16B16A16_SINT";
      case VK_FORMAT_R16G16B16A16_SFLOAT: return "VK_FORMAT_R16G16B16A16_SFLOAT";
      case VK_FORMAT_R32_UINT: return "VK_FORMAT_R32_UINT";
      case VK_FORMAT_R32_SINT: return "VK_FORMAT_R32_SINT";
      case VK_FORMAT_R32_SFLOAT: return "VK_FORMAT_R32_SFLOAT";
      case VK_FORMAT_R32G32_UINT: return "VK_FORMAT_R32G32_UINT";
      case VK_FORMAT_R32G32_SINT: return "VK_FORMAT_R32G32_SINT";
      case VK_FORMAT_R32G32_SFLOAT: return "VK_FORMAT_R32G32_SFLOAT";
      case VK_FORMAT_R32G32B32_UINT: return "VK_FORMAT_R32G32B32_UINT";
      case VK_FORMAT_R32G32B32_SINT: return "VK_FORMAT_R32G32B32_SINT";
      case VK_FORMAT_R32G32B32_SFLOAT: return "VK_FORMAT_R32G32B32_SFLOAT";
      case VK_FORMAT_R32G32B32A32_UINT: return "VK_FORMAT_R32G32B32A32_UINT";
      case VK_FORMAT_R32G32B32A32_SINT: return "VK_FORMAT_R32G32B32A32_SINT";
      case VK_FORMAT_R32G32B32A32_SFLOAT: return "VK_FORMAT_R32G32B32A32_SFLOAT";
      case VK_FORMAT_R64_UINT: return "VK_FORMAT_R64_UINT";
      case VK_FORMAT_R64_SINT: return "VK_FORMAT_R64_SINT";
      case VK_FORMAT_R64_SFLOAT: return "VK_FORMAT_R64_SFLOAT";
      case VK_FORMAT_R64G64_UINT: return "VK_FORMAT_R64G64_UINT";
      case VK_FORMAT_R64G64_SINT: return "VK_FORMAT_R64G64_SINT";
      case VK_FORMAT_R64G64_SFLOAT: return "VK_FORMAT_R64G64_SFLOAT";
      case VK_FORMAT_R64G64B64_UINT: return "VK_FORMAT_R64G64B64_UINT";
      case VK_FORMAT_R64G64B64_SINT: return "VK_FORMAT_R64G64B64_SINT";
      case VK_FORMAT_R64G64B64_SFLOAT: return "VK_FORMAT_R64G64B64_SFLOAT";
      case VK_FORMAT_R64G64B64A64_UINT: return "VK_FORMAT_R64G64B64A64_UINT";
      case VK_FORMAT_R64G64B64A64_SINT: return "VK_FORMAT_R64G64B64A64_SINT";
      case VK_FORMAT_R64G64B64A64_SFLOAT: return "VK_FORMAT_R64G64B64A64_SFLOAT";
      case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
      case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
      case VK_FORMAT_D16_UNORM: return "VK_FORMAT_D16_UNORM";
      case VK_FORMAT_X8_D24_UNORM_PACK32: return "VK_FORMAT_X8_D24_UNORM_PACK32";
      case VK_FORMAT_D32_SFLOAT: return "VK_FORMAT_D32_SFLOAT";
      case VK_FORMAT_S8_UINT: return "VK_FORMAT_S8_UINT";
      case VK_FORMAT_D16_UNORM_S8_UINT: return "VK_FORMAT_D16_UNORM_S8_UINT";
      case VK_FORMAT_D24_UNORM_S8_UINT: return "VK_FORMAT_D24_UNORM_S8_UINT";
      case VK_FORMAT_D32_SFLOAT_S8_UINT: return "VK_FORMAT_D32_SFLOAT_S8_UINT";
      case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
      case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
      case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
      case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
      case VK_FORMAT_BC2_UNORM_BLOCK: return "VK_FORMAT_BC2_UNORM_BLOCK";
      case VK_FORMAT_BC2_SRGB_BLOCK: return "VK_FORMAT_BC2_SRGB_BLOCK";
      case VK_FORMAT_BC3_UNORM_BLOCK: return "VK_FORMAT_BC3_UNORM_BLOCK";
      case VK_FORMAT_BC3_SRGB_BLOCK: return "VK_FORMAT_BC3_SRGB_BLOCK";
      case VK_FORMAT_BC4_UNORM_BLOCK: return "VK_FORMAT_BC4_UNORM_BLOCK";
      case VK_FORMAT_BC4_SNORM_BLOCK: return "VK_FORMAT_BC4_SNORM_BLOCK";
      case VK_FORMAT_BC5_UNORM_BLOCK: return "VK_FORMAT_BC5_UNORM_BLOCK";
      case VK_FORMAT_BC5_SNORM_BLOCK: return "VK_FORMAT_BC5_SNORM_BLOCK";
      case VK_FORMAT_BC6H_UFLOAT_BLOCK: return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
      case VK_FORMAT_BC6H_SFLOAT_BLOCK: return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
      case VK_FORMAT_BC7_UNORM_BLOCK: return "VK_FORMAT_BC7_UNORM_BLOCK";
      case VK_FORMAT_BC7_SRGB_BLOCK: return "VK_FORMAT_BC7_SRGB_BLOCK";
      case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
      case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
      case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
      case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
      case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
      case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
      case VK_FORMAT_EAC_R11_UNORM_BLOCK: return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
      case VK_FORMAT_EAC_R11_SNORM_BLOCK: return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
      case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
      case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
      case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
      case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
      case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
      case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
      case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
      case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
      case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
      case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
      case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
      case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
      case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
      case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
      case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
      case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
      case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
      case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
      case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
      case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
      case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
      case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
      case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
      case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
      case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
      case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
      case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
      case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
      case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
      case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
      case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
      case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
      case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
      case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
      case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
      case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
      case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
      case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
      case VK_FORMAT_G8B8G8R8_422_UNORM_KHR: return "VK_FORMAT_G8B8G8R8_422_UNORM_KHR";
      case VK_FORMAT_B8G8R8G8_422_UNORM_KHR: return "VK_FORMAT_B8G8R8G8_422_UNORM_KHR";
      case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR: return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR";
      case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR: return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR";
      case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR: return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR";
      case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR: return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR";
      case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR: return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR";
      case VK_FORMAT_R10X6_UNORM_PACK16_KHR: return "VK_FORMAT_R10X6_UNORM_PACK16_KHR";
      case VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR: return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR";
      case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR: return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR";
      case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR: return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR";
      case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR: return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR";
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR: return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR";
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR: return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR";
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR: return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR";
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR: return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR";
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR: return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR";
      case VK_FORMAT_R12X4_UNORM_PACK16_KHR: return "VK_FORMAT_R12X4_UNORM_PACK16_KHR";
      case VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR: return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR";
      case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR: return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR";
      case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR: return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR";
      case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR: return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR";
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR: return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR";
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR: return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR";
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR: return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR";
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR: return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR";
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR: return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR";
      case VK_FORMAT_G16B16G16R16_422_UNORM_KHR: return "VK_FORMAT_G16B16G16R16_422_UNORM_KHR";
      case VK_FORMAT_B16G16R16G16_422_UNORM_KHR: return "VK_FORMAT_B16G16R16G16_422_UNORM_KHR";
      case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR: return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR";
      case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR: return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR";
      case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR: return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR";
      case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR: return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR";
      case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR: return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR";

      case VK_FORMAT_RANGE_SIZE:
      case VK_FORMAT_MAX_ENUM:
        break;
  }
  // unhandled VkFormat enum value
  return "VK_FORMAT_???";
}

static const char* ToStringGlslType(const SpvReflectTypeDescription& type)
{
  switch (type.op) {
    case SpvOpTypeVector: {
      switch (type.traits.numeric.scalar.width) {
        case 32: {
          switch (type.traits.numeric.vector.component_count) {
            case 2: return "vec2";
            case 3: return "vec3";
            case 4: return "vec4";
          }
        }
        break;

        case 64: {
          switch (type.traits.numeric.vector.component_count) {
            case 2: return "dvec2";
            case 3: return "dvec3";
            case 4: return "dvec4";
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
            case 2: return "float2";
            case 3: return "float3";
            case 4: return "float4";
          }
        }
        break;

        case 64: {
          switch (type.traits.numeric.vector.component_count) {
            case 2: return "double2";
            case 3: return "double3";
            case 4: return "double4";
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

static const char* ToStringGlslBuiltIn(SpvBuiltIn built_in)
{
  switch (built_in) {
    case SpvBuiltInPosition                     : break;
    case SpvBuiltInPointSize                    : break;
    case SpvBuiltInClipDistance                 : break;
    case SpvBuiltInCullDistance                 : break;
    case SpvBuiltInVertexId                     : break;
    case SpvBuiltInInstanceId                   : break;
    case SpvBuiltInPrimitiveId                  : break;
    case SpvBuiltInInvocationId                 : break;
    case SpvBuiltInLayer                        : break;
    case SpvBuiltInViewportIndex                : break;
    case SpvBuiltInTessLevelOuter               : break;
    case SpvBuiltInTessLevelInner               : break;
    case SpvBuiltInTessCoord                    : break;
    case SpvBuiltInPatchVertices                : break;
    case SpvBuiltInFragCoord                    : break;
    case SpvBuiltInPointCoord                   : break;
    case SpvBuiltInFrontFacing                  : break;
    case SpvBuiltInSampleId                     : break;
    case SpvBuiltInSamplePosition               : break;
    case SpvBuiltInSampleMask                   : break;
    case SpvBuiltInFragDepth                    : break;
    case SpvBuiltInHelperInvocation             : break;
    case SpvBuiltInNumWorkgroups                : break;
    case SpvBuiltInWorkgroupSize                : break;
    case SpvBuiltInWorkgroupId                  : break;
    case SpvBuiltInLocalInvocationId            : break;
    case SpvBuiltInGlobalInvocationId           : break;
    case SpvBuiltInLocalInvocationIndex         : break;
    case SpvBuiltInWorkDim                      : break;
    case SpvBuiltInGlobalSize                   : break;
    case SpvBuiltInEnqueuedWorkgroupSize        : break;
    case SpvBuiltInGlobalOffset                 : break;
    case SpvBuiltInGlobalLinearId               : break;
    case SpvBuiltInSubgroupSize                 : break;
    case SpvBuiltInSubgroupMaxSize              : break;
    case SpvBuiltInNumSubgroups                 : break;
    case SpvBuiltInNumEnqueuedSubgroups         : break;
    case SpvBuiltInSubgroupId                   : break;
    case SpvBuiltInSubgroupLocalInvocationId    : break;
    case SpvBuiltInVertexIndex                  : break;
    case SpvBuiltInInstanceIndex                : break;
    case SpvBuiltInSubgroupEqMaskKHR            : break;
    case SpvBuiltInSubgroupGeMaskKHR            : break;
    case SpvBuiltInSubgroupGtMaskKHR            : break;
    case SpvBuiltInSubgroupLeMaskKHR            : break;
    case SpvBuiltInSubgroupLtMaskKHR            : break;
    case SpvBuiltInBaseVertex                   : break;
    case SpvBuiltInBaseInstance                 : break;
    case SpvBuiltInDrawIndex                    : break;
    case SpvBuiltInDeviceIndex                  : break;
    case SpvBuiltInViewIndex                    : break;
    case SpvBuiltInBaryCoordNoPerspAMD          : break;
    case SpvBuiltInBaryCoordNoPerspCentroidAMD  : break;
    case SpvBuiltInBaryCoordNoPerspSampleAMD    : break;
    case SpvBuiltInBaryCoordSmoothAMD           : break;
    case SpvBuiltInBaryCoordSmoothCentroidAMD   : break;
    case SpvBuiltInBaryCoordSmoothSampleAMD     : break;
    case SpvBuiltInBaryCoordPullModelAMD        : break;
    case SpvBuiltInFragStencilRefEXT            : break;
    case SpvBuiltInViewportMaskNV               : break;
    case SpvBuiltInSecondaryPositionNV          : break;
    case SpvBuiltInSecondaryViewportMaskNV      : break;
    case SpvBuiltInPositionPerViewNV            : break;
    case SpvBuiltInViewportMaskPerViewNV        : break;
    default: break;
  }
  return "";
}

static const char* ToStringHlslBuiltIn(SpvBuiltIn built_in)
{
  switch (built_in) {
    case SpvBuiltInPosition                     : return "SV_POSITION";
    case SpvBuiltInPointSize                    : break;
    case SpvBuiltInClipDistance                 : break;
    case SpvBuiltInCullDistance                 : break;
    case SpvBuiltInVertexId                     : break;
    case SpvBuiltInInstanceId                   : break;
    case SpvBuiltInPrimitiveId                  : break;
    case SpvBuiltInInvocationId                 : break;
    case SpvBuiltInLayer                        : break;
    case SpvBuiltInViewportIndex                : break;
    case SpvBuiltInTessLevelOuter               : break;
    case SpvBuiltInTessLevelInner               : break;
    case SpvBuiltInTessCoord                    : break;
    case SpvBuiltInPatchVertices                : break;
    case SpvBuiltInFragCoord                    : return "SV_POSITION"; break;
    case SpvBuiltInPointCoord                   : break;
    case SpvBuiltInFrontFacing                  : break;
    case SpvBuiltInSampleId                     : break;
    case SpvBuiltInSamplePosition               : break;
    case SpvBuiltInSampleMask                   : break;
    case SpvBuiltInFragDepth                    : break;
    case SpvBuiltInHelperInvocation             : break;
    case SpvBuiltInNumWorkgroups                : break;
    case SpvBuiltInWorkgroupSize                : break;
    case SpvBuiltInWorkgroupId                  : break;
    case SpvBuiltInLocalInvocationId            : break;
    case SpvBuiltInGlobalInvocationId           : break;
    case SpvBuiltInLocalInvocationIndex         : break;
    case SpvBuiltInWorkDim                      : break;
    case SpvBuiltInGlobalSize                   : break;
    case SpvBuiltInEnqueuedWorkgroupSize        : break;
    case SpvBuiltInGlobalOffset                 : break;
    case SpvBuiltInGlobalLinearId               : break;
    case SpvBuiltInSubgroupSize                 : break;
    case SpvBuiltInSubgroupMaxSize              : break;
    case SpvBuiltInNumSubgroups                 : break;
    case SpvBuiltInNumEnqueuedSubgroups         : break;
    case SpvBuiltInSubgroupId                   : break;
    case SpvBuiltInSubgroupLocalInvocationId    : break;
    case SpvBuiltInVertexIndex                  : break;
    case SpvBuiltInInstanceIndex                : break;
    case SpvBuiltInSubgroupEqMaskKHR            : break;
    case SpvBuiltInSubgroupGeMaskKHR            : break;
    case SpvBuiltInSubgroupGtMaskKHR            : break;
    case SpvBuiltInSubgroupLeMaskKHR            : break;
    case SpvBuiltInSubgroupLtMaskKHR            : break;
    case SpvBuiltInBaseVertex                   : break;
    case SpvBuiltInBaseInstance                 : break;
    case SpvBuiltInDrawIndex                    : break;
    case SpvBuiltInDeviceIndex                  : break;
    case SpvBuiltInViewIndex                    : break;
    case SpvBuiltInBaryCoordNoPerspAMD          : break;
    case SpvBuiltInBaryCoordNoPerspCentroidAMD  : break;
    case SpvBuiltInBaryCoordNoPerspSampleAMD    : break;
    case SpvBuiltInBaryCoordSmoothAMD           : break;
    case SpvBuiltInBaryCoordSmoothCentroidAMD   : break;
    case SpvBuiltInBaryCoordSmoothSampleAMD     : break;
    case SpvBuiltInBaryCoordPullModelAMD        : break;
    case SpvBuiltInFragStencilRefEXT            : break;
    case SpvBuiltInViewportMaskNV               : break;
    case SpvBuiltInSecondaryPositionNV          : break;
    case SpvBuiltInSecondaryViewportMaskNV      : break;
    case SpvBuiltInPositionPerViewNV            : break;
    case SpvBuiltInViewportMaskPerViewNV        : break;
    default: break;
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
  if (obj.semantic != nullptr) {
    os << t << "semantic  : " << obj.semantic << "\n";
  }
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

//////////////////////////////////

SpvReflectToYaml::SpvReflectToYaml(const SpvReflectShaderModule& shader_module, uint32_t verbosity) :
  sm_(shader_module), verbosity_(verbosity)
{
}

void SpvReflectToYaml::WriteTypeDescription(std::ostream& os, const SpvReflectTypeDescription& td, uint32_t indent_level) {
  // YAML anchors can only refer to points earlier in the doc, so child type descriptions must
  // be processed before the parent.
  for(uint32_t i=0; i<td.member_count; ++i) {
    WriteTypeDescription(os, td.members[i], indent_level);
  }
  const std::string t0 = Indent(indent_level);
  const std::string t1 = Indent(indent_level+1);
  const std::string t2 = Indent(indent_level+2);
  const std::string t3 = Indent(indent_level+3);
  const std::string t4 = Indent(indent_level+4);

  // Determine the index of this type within the shader module's list.
  assert(type_description_to_index_.find(&td) == type_description_to_index_.end());
  uint32_t type_description_index = static_cast<uint32_t>(type_description_to_index_.size());
  type_description_to_index_[&td] = type_description_index;

  os << t0 << "- &td" << type_description_index << std::endl;
  // typedef struct SpvReflectTypeDescription {
  //   uint32_t                          id;
  os << t1 << "id: " << td.id << std::endl;
  //   SpvOp                             op;
  os << t1 << "op: " << td.op << std::endl;
  //   const char*                       type_name;
  os << t1 << "type_name: " << SafeString(td.type_name) << std::endl;
  //   const char*                       struct_member_name;
  os << t1 << "struct_member_name: " << SafeString(td.struct_member_name) << std::endl;
  //   SpvStorageClass                   storage_class;
  os << t1 << "storage_class: " << td.storage_class << " # " << ToStringSpvStorageClass(td.storage_class) << std::endl;
  //   SpvReflectTypeFlags               type_flags;
  os << t1 << "type_flags: " << AsHexString(td.type_flags) << " # " << ToStringSpvReflectTypeFlags(td.type_flags) << std::endl;
  //   SpvReflectDecorationFlags         decoration_flags;
  os << t1 << "decoration_flags: " << AsHexString(td.decoration_flags) << " # " << ToStringSpvReflectDecorationFlags(td.decoration_flags) << std::endl;
  //   struct Traits {
  os << t1 << "traits:" << std::endl;
  //     SpvReflectNumericTraits         numeric;
  // typedef struct SpvReflectNumericTraits {
  os << t2 << "numeric:" << std::endl;
  //   struct Scalar {
  //     uint32_t                        width;
  //     uint32_t                        signedness;
  //   } scalar;
  os << t3 << "scalar: { ";
  os << "width: " << td.traits.numeric.scalar.width << ", ";
  os << "signedness: " << td.traits.numeric.scalar.signedness;
  os << " }" << std::endl;
  //   struct Vector {
  //     uint32_t                        component_count;
  //   } vector;
  os << t3 << "vector: { ";
  os << "component_count: " << td.traits.numeric.vector.component_count;
  os << " }" << std::endl;
  //   struct Matrix {
  //     uint32_t                        column_count;
  //     uint32_t                        row_count;
  //     uint32_t                        stride; // Measured in bytes
  //   } matrix;
  os << t3 << "matrix: { ";
  os << "column_count: " << td.traits.numeric.matrix.column_count << ", ";
  os << "row_count: " << td.traits.numeric.matrix.row_count << ", ";;
  os << "stride: " << td.traits.numeric.matrix.stride;
  os << " }" << std::endl;
  // } SpvReflectNumericTraits;

  //     SpvReflectImageTraits           image;
  os << t2 << "image: { ";
  // typedef struct SpvReflectImageTraits {
  //   SpvDim                            dim;
  os << "dim: " << td.traits.image.dim << ", ";
  //   uint32_t                          depth;
  os << "depth: " << td.traits.image.depth << ", ";
  //   uint32_t                          arrayed;
  os << "arrayed: " << td.traits.image.arrayed << ", ";
  //   uint32_t                          ms;
  os << "ms: " << td.traits.image.ms << ", ";
  //   uint32_t                          sampled;
  os << "sampled: " << td.traits.image.sampled << ", ";
  //   SpvImageFormat                    image_format;
  os << "image_format: " << td.traits.image.image_format;
  // } SpvReflectImageTraits;
  os << " }" << " # dim=" << ToStringSpvDim(td.traits.image.dim) << " image_format=" << ToStringSpvImageFormat(td.traits.image.image_format) << std::endl;

  //     SpvReflectArrayTraits           array;
  os << t2 << "array: { ";
  // typedef struct SpvReflectArrayTraits {
  //   uint32_t                          dims_count;
  os << "dims_count: " << td.traits.array.dims_count << ", ";
  //   uint32_t                          dims[SPV_REFLECT_MAX_ARRAY_DIMS];
  os << "dims: [";
  for(uint32_t i_dim=0; i_dim < td.traits.array.dims_count; ++i_dim) {
    os << td.traits.array.dims[i_dim] << ",";
  }
  os << "], ";
  //   uint32_t                          stride; // Measured in bytes
  os << "stride: " << td.traits.array.stride;
  // } SpvReflectArrayTraits;
  os << " }" << std::endl;
  //   } traits;

  //   uint32_t                          member_count;
  os << t1 << "member_count: " << td.member_count << std::endl;
  //   struct SpvReflectTypeDescription* members;
  os << t1 << "members:" << std::endl;
  for(uint32_t i_member=0; i_member < td.member_count; ++i_member) {
    os << t2 << "- *td" << type_description_to_index_[&(td.members[i_member])] << std::endl;
  }
  // } SpvReflectTypeDescription;
}

void SpvReflectToYaml::WriteBlockVariable(std::ostream& os, const SpvReflectBlockVariable& bv, uint32_t indent_level) {
  for(uint32_t i=0; i<bv.member_count; ++i) {
    WriteBlockVariable(os, bv.members[i], indent_level);
  }

  const std::string t0 = Indent(indent_level);
  const std::string t1 = Indent(indent_level+1);
  const std::string t2 = Indent(indent_level+2);
  const std::string t3 = Indent(indent_level+3);

  assert(block_variable_to_index_.find(&bv) == block_variable_to_index_.end());
  uint32_t block_variable_index = static_cast<uint32_t>(block_variable_to_index_.size());
  block_variable_to_index_[&bv] = block_variable_index;

  os << t0 << "- &bv" << block_variable_index << std::endl;
  // typedef struct SpvReflectBlockVariable {
  //   const char*                       name;
  os << t1 << "name: " << SafeString(bv.name) << std::endl;
  //   uint32_t                          offset;           // Measured in bytes
  os << t1 << "offset: " << bv.offset << std::endl;
  //   uint32_t                          absolute_offset;  // Measured in bytes
  os << t1 << "absolute_offset: " << bv.absolute_offset << std::endl;
  //   uint32_t                          size;             // Measured in bytes
  os << t1 << "size: " << bv.size << std::endl;
  //   uint32_t                          padded_size;      // Measured in bytes
  os << t1 << "padded_size: " << bv.padded_size << std::endl;
  //   SpvReflectDecorationFlags         decoration_flags;
  os << t1 << "decorations: " << AsHexString(bv.decoration_flags) << " # " << ToStringSpvReflectDecorationFlags(bv.decoration_flags) << std::endl;
  //   SpvReflectNumericTraits           numeric;
  // typedef struct SpvReflectNumericTraits {
  os << t1 << "numeric:" << std::endl;
  //   struct Scalar {
  //     uint32_t                        width;
  //     uint32_t                        signedness;
  //   } scalar;
  os << t2 << "scalar: { ";
  os << "width: " << bv.numeric.scalar.width << ", ";
  os << "signedness: " << bv.numeric.scalar.signedness << " }" << std::endl;
  //   struct Vector {
  //     uint32_t                        component_count;
  //   } vector;
  os << t2 << "vector: { ";
  os << "component_count: " << bv.numeric.vector.component_count << " }" << std::endl;
  //   struct Matrix {
  //     uint32_t                        column_count;
  //     uint32_t                        row_count;
  //     uint32_t                        stride; // Measured in bytes
  //   } matrix;
  os << t2 << "matrix: { ";
  os << "column_count: " << bv.numeric.matrix.column_count << ", ";
  os << "row_count: " << bv.numeric.matrix.row_count << ", ";;
  os << "stride: " << bv.numeric.matrix.stride << " }" << std::endl;
  // } SpvReflectNumericTraits;

  //     SpvReflectArrayTraits           array;
  os << t1 << "array: { ";
  // typedef struct SpvReflectArrayTraits {
  //   uint32_t                          dims_count;
  os << "dims_count: " << bv.array.dims_count << ", ";
  //   uint32_t                          dims[SPV_REFLECT_MAX_ARRAY_DIMS];
  os << "dims: [";
  for(uint32_t i_dim=0; i_dim < bv.array.dims_count; ++i_dim) {
    os << bv.array.dims[i_dim] << ",";
  }
  os << "], ";
  //   uint32_t                          stride; // Measured in bytes
  os << "stride: " << bv.array.stride;
  // } SpvReflectArrayTraits;
  os << " }" << std::endl;

  //   uint32_t                          member_count;
  os << t1 << "member_count: " << bv.member_count << std::endl;
  //   struct SpvReflectBlockVariable*   members;
  os << t1 << "members:" << std::endl;
  for(uint32_t i=0; i<bv.member_count; ++i) {
    auto itor = block_variable_to_index_.find(&bv.members[i]);
    assert(itor != block_variable_to_index_.end());
    os << t2 << "- *bv" << itor->second << std::endl;
  }
  if (verbosity_ >= 1) {
    //   SpvReflectTypeDescription*        type_description;
    if (bv.type_description == nullptr) {
      os << t1 << "type_description:" << std::endl;
    } else {
      auto itor = type_description_to_index_.find(bv.type_description);
      assert(itor != type_description_to_index_.end());
      os << t1 << "type_description: *td" << itor->second << std::endl;
    }
  }
  // } SpvReflectBlockVariable;
}

void SpvReflectToYaml::WriteDescriptorBinding(std::ostream& os, const SpvReflectDescriptorBinding& db, uint32_t indent_level) {
  if (db.uav_counter_binding != nullptr) {
    auto itor = descriptor_binding_to_index_.find(db.uav_counter_binding);
    if (itor == descriptor_binding_to_index_.end()) {
      WriteDescriptorBinding(os, *(db.uav_counter_binding), indent_level);
    }
  }

  const std::string t0 = Indent(indent_level);
  const std::string t1 = Indent(indent_level+1);
  const std::string t2 = Indent(indent_level+2);
  const std::string t3 = Indent(indent_level+3);

  // A binding's UAV binding later may appear later in the table than the binding itself,
  // in which case we've already output entries for both bindings, and can just write another
  // reference here.
  {
    auto itor = descriptor_binding_to_index_.find(&db);
    if (itor != descriptor_binding_to_index_.end()) {
      os << t0 << "- *db" << itor->second << std::endl;
      return;
    }
  }

  uint32_t descriptor_binding_index = static_cast<uint32_t>(descriptor_binding_to_index_.size());
  descriptor_binding_to_index_[&db] = descriptor_binding_index;

  os << t0 << "- &db" << descriptor_binding_index << std::endl;
  // typedef struct SpvReflectDescriptorBinding {
  //   uint32_t                            spirv_id;
  os << t1 << "spirv_id: " << db.spirv_id << std::endl;
  //   const char*                         name;
  os << t1 << "name: " << SafeString(db.name) << std::endl;
  //   uint32_t                            binding;
  os << t1 << "binding: " << db.binding << std::endl;
  //   uint32_t                            input_attachment_index;
  os << t1 << "input_attachment_index: " << db.input_attachment_index << std::endl;
  //   uint32_t                            set;
  os << t1 << "set: " << db.set << std::endl;
  //   VkDescriptorType                    descriptor_type;
  os << t1 << "descriptor_type: " << db.descriptor_type << " # " << ToStringVkDescriptorType(db.descriptor_type) << std::endl;
  //   SpvReflectResourceType              resource_type;
  os << t1 << "resource_type: " << db.resource_type << " # " << ToStringSpvReflectResourceType(db.resource_type) << std::endl;
  //   SpvReflectImageTraits           image;
  os << t1 << "image: { ";
  // typedef struct SpvReflectImageTraits {
  //   SpvDim                            dim;
  os << "dim: " << db.image.dim << ", ";
  //   uint32_t                          depth;
  os << "depth: " << db.image.depth << ", ";
  //   uint32_t                          arrayed;
  os << "arrayed: " << db.image.arrayed << ", ";
  //   uint32_t                          ms;
  os << "ms: " << db.image.ms << ", ";
  //   uint32_t                          sampled;
  os << "sampled: " << db.image.sampled << ", ";
  //   SpvImageFormat                    image_format;
  os << "image_format: " << db.image.image_format;
  // } SpvReflectImageTraits;
  os << " }" << " # dim=" << ToStringSpvDim(db.image.dim) << " image_format=" << ToStringSpvImageFormat(db.image.image_format) << std::endl;

  //   SpvReflectBlockVariable             block;
  {
    auto itor = block_variable_to_index_.find(&db.block);
    assert(itor != block_variable_to_index_.end());
    os << t1 << "block: *bv" << itor->second << " # " << SafeString(db.block.name) << std::endl;
  }
  //   SpvReflectBindingArrayTraits        array;
  os << t1 << "array: { ";
  // typedef struct SpvReflectBindingArrayTraits {
  //   uint32_t                          dims_count;
  os << "dims_count: " << db.array.dims_count << ", ";
  //   uint32_t                          dims[SPV_REFLECT_MAX_ARRAY_DIMS];
  os << "dims: [";
  for(uint32_t i_dim=0; i_dim < db.array.dims_count; ++i_dim) {
    os << db.array.dims[i_dim] << ",";
  }
  // } SpvReflectBindingArrayTraits;
  os << "] }" << std::endl;

  //   uint32_t                            uav_counter_id;
  os << t1 << "uav_counter_id: " << db.uav_counter_id << std::endl;
  //   struct SpvReflectDescriptorBinding* uav_counter_binding;
  if (db.uav_counter_binding == nullptr) {
    os << t1 << "uav_counter_binding:" << std::endl;
  } else {
    auto itor = descriptor_binding_to_index_.find(db.uav_counter_binding);
    assert(itor != descriptor_binding_to_index_.end());
    os << t1 << "uav_counter_binding: *db" << itor->second << " # " << SafeString(db.uav_counter_binding->name) << std::endl;
  }
  if (verbosity_ >= 1) {
    //   SpvReflectTypeDescription*        type_description;
    if (db.type_description == nullptr) {
      os << t1 << "type_description:" << std::endl;
    } else {
      auto itor = type_description_to_index_.find(db.type_description);
      assert(itor != type_description_to_index_.end());
      os << t1 << "type_description: *td" << itor->second << std::endl;
    }
  }
  //   struct {
  //     uint32_t                        binding;
  //     uint32_t                        set;
  //   } word_offset;
  os << t1 << "word_offset: { binding: " << db.word_offset.binding;
  os << ", set: " << db.word_offset.set << " }" << std::endl;
  // } SpvReflectDescriptorBinding;
}

void SpvReflectToYaml::WriteInterfaceVariable(std::ostream& os, const SpvReflectInterfaceVariable& iv, uint32_t indent_level) {
  for(uint32_t i=0; i<iv.member_count; ++i) {
    assert(interface_variable_to_index_.find(&iv.members[i]) == interface_variable_to_index_.end());
    WriteInterfaceVariable(os, iv.members[i], indent_level);
  }

  const std::string t0 = Indent(indent_level);
  const std::string t1 = Indent(indent_level+1);
  const std::string t2 = Indent(indent_level+2);
  const std::string t3 = Indent(indent_level+3);

  uint32_t interface_variable_index = static_cast<uint32_t>(interface_variable_to_index_.size());
  interface_variable_to_index_[&iv] = interface_variable_index;

  // typedef struct SpvReflectInterfaceVariable {
  os << t0 << "- &iv" << interface_variable_index << std::endl;
  //   uint32_t                            spirv_id;
  os << t1 << "spirv_id: " << iv.spirv_id << std::endl;
  //   const char*                         name;
  os << t1 << "name: " << SafeString(iv.name) << std::endl;
  //   uint32_t                            location;
  os << t1 << "location: " << iv.location << std::endl;
  //   SpvStorageClass                     storage_class;
  os << t1 << "storage_class: " << iv.storage_class << " # " << ToStringSpvStorageClass(iv.storage_class) << std::endl;
  //   const char*                         semantic;
  os << t1 << "semantic: " << SafeString(iv.semantic) << std::endl;
  //   SpvReflectDecorationFlags           decoration_flags;
  os << t1 << "decoration_flags: " << AsHexString(iv.decoration_flags) << " # " << ToStringSpvReflectDecorationFlags(iv.decoration_flags)  << std::endl;
  //   SpvBuiltIn                          built_in;
  os << t1 << "built_in: " << iv.built_in << " # " << ToStringSpvBuiltIn(iv.built_in) << std::endl;
  //   SpvReflectNumericTraits             numeric;
  // typedef struct SpvReflectNumericTraits {
  os << t1 << "numeric:" << std::endl;
  //   struct Scalar {
  //     uint32_t                        width;
  //     uint32_t                        signedness;
  //   } scalar;
  os << t2 << "scalar: { ";
  os << "width: " << iv.numeric.scalar.width << ", ";
  os << "signedness: " << iv.numeric.scalar.signedness << " }" << std::endl;
  //   struct Vector {
  //     uint32_t                        component_count;
  //   } vector;
  os << t2 << "vector: { ";
  os << "component_count: " << iv.numeric.vector.component_count << " }" << std::endl;
  //   struct Matrix {
  //     uint32_t                        column_count;
  //     uint32_t                        row_count;
  //     uint32_t                        stride; // Measured in bytes
  //   } matrix;
  os << t2 << "matrix: { ";
  os << "column_count: " << iv.numeric.matrix.column_count << ", ";
  os << "row_count: " << iv.numeric.matrix.row_count << ", ";;
  os << "stride: " << iv.numeric.matrix.stride << " }" << std::endl;
  // } SpvReflectNumericTraits;

  //     SpvReflectArrayTraits           array;
  os << t1 << "array: { ";
  // typedef struct SpvReflectArrayTraits {
  //   uint32_t                          dims_count;
  os << "dims_count: " << iv.array.dims_count << ", ";
  //   uint32_t                          dims[SPV_REFLECT_MAX_ARRAY_DIMS];
  os << "dims: [";
  for(uint32_t i_dim=0; i_dim < iv.array.dims_count; ++i_dim) {
    os << iv.array.dims[i_dim] << ",";
  }
  os << "], ";
  //   uint32_t                          stride; // Measured in bytes
  os << "stride: " << iv.array.stride;
  // } SpvReflectArrayTraits;
  os << " }" << std::endl;

  //   uint32_t                            member_count;
  os << t1 << "member_count: " << iv.member_count << std::endl;
  //   struct SpvReflectInterfaceVariable* members;
  os << t1 << "members:" << std::endl;
  for(uint32_t i=0; i<iv.member_count; ++i) {
    auto itor = interface_variable_to_index_.find(&iv.members[i]);
    assert(itor != interface_variable_to_index_.end());
    os << t2 << "- *iv" << itor->second << " # " << SafeString(iv.members[i].name) << std::endl;
  }

  //   VkFormat                          format;
  os << t1 << "format: " << iv.format << " # " << ToStringVkFormat(iv.format) << std::endl;

  if (verbosity_ >= 1) {
    //   SpvReflectTypeDescription*        type_description;
    if (!iv.type_description) {
      os << t1 << "type_description:" << std::endl;
    } else {
      auto itor = type_description_to_index_.find(iv.type_description);
      assert(itor != type_description_to_index_.end());
      os << t1 << "type_description: *td" << itor->second << std::endl;
    }
  }

  //   struct {
  //     uint32_t                        location;
  //   } word_offset;
  os << t1 << "word_offset: { location: " << iv.word_offset.location << " }" << std::endl;

  // } SpvReflectInterfaceVariable;
}

void SpvReflectToYaml::WriteBlockVariableTypes(std::ostream& os, const SpvReflectBlockVariable& bv, uint32_t indent_level) {
  const auto* td = bv.type_description;
  if (td && type_description_to_index_.find(td) == type_description_to_index_.end()) {
    WriteTypeDescription(os, *td, indent_level);
  }

  for(uint32_t i=0; i<bv.member_count; ++i) {
    WriteBlockVariableTypes(os, bv.members[i], indent_level);
  }
}
void SpvReflectToYaml::WriteDescriptorBindingTypes(std::ostream& os, const SpvReflectDescriptorBinding& db, uint32_t indent_level) {
  WriteBlockVariableTypes(os, db.block, indent_level);

  if (db.uav_counter_binding) {
    WriteDescriptorBindingTypes(os, *(db.uav_counter_binding), indent_level);
  }

  const auto* td = db.type_description;
  if (td && type_description_to_index_.find(td) == type_description_to_index_.end()) {
    WriteTypeDescription(os, *td, indent_level);
  }
}
void SpvReflectToYaml::WriteInterfaceVariableTypes(std::ostream& os, const SpvReflectInterfaceVariable& iv, uint32_t indent_level) {
  const auto* td = iv.type_description;
  if (td && type_description_to_index_.find(td) == type_description_to_index_.end()) {
    WriteTypeDescription(os, *td, indent_level);
  }

  for(uint32_t i=0; i<iv.member_count; ++i) {
    WriteInterfaceVariableTypes(os, iv.members[i], indent_level);
  }
}


void SpvReflectToYaml::Write(std::ostream& os)
{
  if (!sm_._internal) {
    return;
  }

  uint32_t indent_level = 0;
  const std::string t0 = Indent(indent_level);
  const std::string t1 = Indent(indent_level+1);
  const std::string t2 = Indent(indent_level+2);
  const std::string t3 = Indent(indent_level+3);

  os << "%YAML 1.0" << std::endl;
  os << "---" << std::endl;

  type_description_to_index_.clear();
  if (verbosity_ >= 2) {
    os << t0 << "all_type_descriptions:" << std::endl;
    // Write the entire internal type_description table; all type descriptions are
    // reachable from there, though most of them are purely internal & not referenced
    // by any of the public-facing structures.
    for(size_t i=0; i<sm_._internal->type_description_count; ++i) {
      WriteTypeDescription(os, sm_._internal->type_descriptions[i], indent_level+1);
    }
  } else if (verbosity_ >= 1) {
    os << t0 << "all_type_descriptions:" << std::endl;
    // Iterate through all public-facing structures and write any type descriptions
    // we find (and their children).
    for(uint32_t i=0; i<sm_.descriptor_binding_count; ++i) {
      WriteDescriptorBindingTypes(os, sm_.descriptor_bindings[i], indent_level+1);
    }
    for(uint32_t i=0; i<sm_.push_constant_block_count; ++i) {
      WriteBlockVariableTypes(os, sm_.push_constant_blocks[i], indent_level+1);
    }
    for(uint32_t i=0; i<sm_.input_variable_count; ++i) {
      WriteInterfaceVariableTypes(os, sm_.input_variables[i], indent_level+1);
    }
    for(uint32_t i=0; i<sm_.output_variable_count; ++i) {
      WriteInterfaceVariableTypes(os, sm_.output_variables[i], indent_level+1);
    }
  }

  block_variable_to_index_.clear();
  os << t0 << "all_block_variables:" << std::endl;
  for(uint32_t i=0; i<sm_.descriptor_binding_count; ++i) {
    WriteBlockVariable(os, sm_.descriptor_bindings[i].block, indent_level+1);
  }
  for(uint32_t i=0; i<sm_.push_constant_block_count; ++i) {
    WriteBlockVariable(os, sm_.push_constant_blocks[i], indent_level+1);
  }

  descriptor_binding_to_index_.clear();
  os << t0 << "all_descriptor_bindings:" << std::endl;
  for(uint32_t i=0; i<sm_.descriptor_binding_count; ++i) {
    WriteDescriptorBinding(os, sm_.descriptor_bindings[i], indent_level+1);
  }

  interface_variable_to_index_.clear();
  os << t0 << "all_interface_variables:" << std::endl;
  for(uint32_t i=0; i<sm_.input_variable_count; ++i) {
    WriteInterfaceVariable(os, sm_.input_variables[i], indent_level+1);
  }
  for(uint32_t i=0; i<sm_.output_variable_count; ++i) {
    WriteInterfaceVariable(os, sm_.output_variables[i], indent_level+1);
  }

  // struct SpvReflectShaderModule {
  os << t0 << "module:" << std::endl;
  // const char*                       entry_point_name;
  os << t1 << "entry_point_name: " << SafeString(sm_.entry_point_name) << std::endl;
  // uint32_t                          entry_point_id;
  os << t1 << "entry_point_id: " << sm_.entry_point_id << std::endl;
  // SpvSourceLanguage                 source_language;
  os << t1 << "source_language: " << sm_.source_language << " # " << ToStringSpvSourceLanguage(sm_.source_language) << std::endl;
  // uint32_t                          source_language_version;
  os << t1 << "source_language_version: " << sm_.source_language_version << std::endl;
  // SpvExecutionModel                 spirv_execution_model;
  os << t1 << "spirv_execution_model: " << sm_.spirv_execution_model << " # " << ToStringSpvExecutionModel(sm_.spirv_execution_model) << std::endl;
  // VkShaderStageFlagBits             vulkan_shader_stage;
  os << t1 << "vulkan_shader_stage: " << AsHexString(sm_.vulkan_shader_stage) << " # " << ToStringVkShaderStageFlagBits(sm_.vulkan_shader_stage) << std::endl;
  // uint32_t                          descriptor_binding_count;
  os << t1 << "descriptor_binding_count: " << sm_.descriptor_binding_count << std::endl;
  // SpvReflectDescriptorBinding*      descriptor_bindings;
  os << t1 << "descriptor_bindings:" << std::endl;
  for(uint32_t i=0; i<sm_.descriptor_binding_count; ++i) {
    auto itor = descriptor_binding_to_index_.find(&sm_.descriptor_bindings[i]);
    assert(itor != descriptor_binding_to_index_.end());
    os << t2 << "- *db" << itor->second << " # " << SafeString(sm_.descriptor_bindings[i].name) << std::endl;
  }
  // uint32_t                          descriptor_set_count;
  os << t1 << "descriptor_set_count: " << sm_.descriptor_set_count << std::endl;
  // SpvReflectDescriptorSet descriptor_sets[SPV_REFLECT_MAX_DESCRIPTOR_SETS];
  os << t1 << "descriptor_sets:" << std::endl;
  for(uint32_t i_set=0; i_set<sm_.descriptor_set_count; ++i_set) {
    // typedef struct SpvReflectDescriptorSet {
    const auto& dset = sm_.descriptor_sets[i_set];
    //   uint32_t                          set;
    os << t1 << "- " << "set: " << dset.set << std::endl;
    //   uint32_t                          binding_count;
    os << t2 << "binding_count: " << dset.binding_count << std::endl;
    //   SpvReflectDescriptorBinding**     bindings;
    os << t2 << "bindings:" << std::endl;
    for(uint32_t i_binding=0; i_binding < dset.binding_count; ++i_binding) {
      auto itor = descriptor_binding_to_index_.find(dset.bindings[i_binding]);
      assert(itor != descriptor_binding_to_index_.end());
      os << t3 << "- *db" << itor->second << " # " << SafeString(dset.bindings[i_binding]->name) << std::endl;
    }
    // } SpvReflectDescriptorSet;

  }
  // uint32_t                          input_variable_count;
  os << t1 << "input_variable_count: " << sm_.input_variable_count << ",\n";
  // SpvReflectInterfaceVariable*      input_variables;
  os << t1 << "input_variables:" << std::endl;
  for(uint32_t i=0; i < sm_.input_variable_count; ++i) {
    auto itor = interface_variable_to_index_.find(&sm_.input_variables[i]);
    assert(itor != interface_variable_to_index_.end());
    os << t2 << "- *iv" << itor->second << " # " << SafeString(sm_.input_variables[i].name) << std::endl;
  }
  // uint32_t                          output_variable_count;
  os << t1 << "output_variable_count: " << sm_.output_variable_count << ",\n";
  // SpvReflectInterfaceVariable*      output_variables;
  os << t1 << "output_variables:" << std::endl;
  for(uint32_t i=0; i < sm_.output_variable_count; ++i) {
    auto itor = interface_variable_to_index_.find(&sm_.output_variables[i]);
    assert(itor != interface_variable_to_index_.end());
    os << t2 << "- *iv" << itor->second << " # " << SafeString(sm_.output_variables[i].name) << std::endl;
  }
  // uint32_t                          push_constant_count;
  os << t1 << "push_constant_count: " << sm_.push_constant_block_count << ",\n";
  // SpvReflectBlockVariable*          push_constants;
  os << t1 << "push_constants:" << std::endl;
  for(uint32_t i=0; i<sm_.push_constant_block_count; ++i) {
    auto itor = block_variable_to_index_.find(&sm_.push_constant_blocks[i]);
    assert(itor != block_variable_to_index_.end());
    os << t2 << "- *bv" << itor->second << " # " << SafeString(sm_.push_constant_blocks[i].name) << std::endl;
  }

  if (verbosity_ >= 2) {
    // struct Internal {
    os << t1 << "_internal:" << std::endl;
    if (sm_._internal) {
      //   size_t                          spirv_size;
      os << t2 << "spirv_size: " << sm_._internal->spirv_size << std::endl;
      //   uint32_t*                       spirv_code;
      os << t2 << "spirv_code: [";
      for(size_t i=0; i < sm_._internal->spirv_word_count; ++i) {
        if ((i % 6) == 0) {
          os << std::endl << t3;
        }
        os << AsHexString(sm_._internal->spirv_code[i]) << ",";
      }
      os << "]" << std::endl;
      //   uint32_t                        spirv_word_count;
      os << t2 << "spirv_word_count: " << sm_._internal->spirv_word_count << std::endl;
      //   size_t                          type_description_count;
      os << t2 << "type_description_count: " << sm_._internal->type_description_count << std::endl;
      //   SpvReflectTypeDescription*      type_descriptions;
      os << t2 << "type_descriptions:" << std::endl;
      for(uint32_t i=0; i<sm_._internal->type_description_count; ++i) {
        auto itor = type_description_to_index_.find(&sm_._internal->type_descriptions[i]);
        assert(itor != type_description_to_index_.end());
        os << t3 << "- *td" << itor->second << std::endl;
      }
    }
    // } * _internal;
  }

  os << "..." << std::endl;
}
