#ifndef __SPIRV_REFLECT_TO_STRING_H__
#define __SPIRV_REFLECT_TO_STRING_H__

#include "spirv_reflect.h"

std::string ToStringShaderStage(SpvReflectShaderStageFlagBits stage);
std::string ToStringHlslResourceType(SpvReflectResourceType type);
std::string ToStringDescriptorType(SpvReflectDescriptorType value);
std::string ToStringFormat(SpvReflectFormat format);
std::string ToStringBuiltIn(SpvBuiltIn value);
std::string ToStringStorageClass(SpvStorageClass value);
std::string ToStringComponentType(const SpvReflectTypeDescription& type, uint32_t member_decoration_flags);

#endif // __SPIRV_REFLECT_TO_STRING_H__