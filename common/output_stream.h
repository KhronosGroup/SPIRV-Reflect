#ifndef SPIRV_REFLECT_OUTPUT_STREAM_H
#define SPIRV_REFLECT_OUTPUT_STREAM_H

#include "spirv_reflect.h"
#include <ostream>

const char* ToStringShaderStage(SpvReflectShaderStageFlagBits stage);

std::ostream& operator<<(std::ostream& os, const spv_reflect::ShaderModule& obj);

#endif