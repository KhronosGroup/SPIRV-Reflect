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

//! \file spirv_reflect.h
#ifndef SPIRV_REFLECT_H
#define SPIRV_REFLECT_H

#include <stdint.h>
#include <vulkan/spirv.h>
#include <vulkan/vulkan.h>

//! \enum SpvReflectResult
//!
//!
typedef enum SpvReflectResult {
  SPV_REFLECT_RESULT_SUCCESS,
  SPV_REFLECT_RESULT_NOT_READY,
  SPV_REFLECT_RESULT_ERROR_PARSE_FAILED,
  SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED,
  SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED,
  SPV_REFLECT_RESULT_ERROR_NULL_POINTER,
  SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND,
  SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE,
  SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_OF,
  SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE
} SpvReflectResult;

//! \enum SpvReflectTypeFlagBits
//!
//!
typedef enum SpvReflectTypeFlagBits {
  SPV_REFLECT_TYPE_FLAG_UNDEFINED               = 0x00000000,
  SPV_REFLECT_TYPE_FLAG_VOID                    = 0x00000001,
  SPV_REFLECT_TYPE_FLAG_BOOL                    = 0x00000002,
  SPV_REFLECT_TYPE_FLAG_INT                     = 0x00000004,
  SPV_REFLECT_TYPE_FLAG_FLOAT                   = 0x00000008,
  SPV_REFLECT_TYPE_FLAG_VECTOR                  = 0x00000100,
  SPV_REFLECT_TYPE_FLAG_MATRIX                  = 0x00000200,
  SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE          = 0x00010000,
  SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLER        = 0x00020000,
  SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE  = 0x00040000,
  SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK          = 0x00080000,
  SPV_REFLECT_TYPE_FLAG_EXTERNAL_MASK           = 0x000F0000,
  SPV_REFLECT_TYPE_FLAG_STRUCT                  = 0x10000000,
  SPV_REFLECT_TYPE_FLAG_ARRAY                   = 0x20000000,
} SpvReflectTypeFlagBits;

typedef uint32_t SpvReflectTypeFlags;

//! \enum SpvReflectDecorationBits
//!
//!
typedef enum SpvReflectDecorationBits {
  SPV_REFLECT_DECORATION_NONE                   = 0x00000000,
  SPV_REFLECT_DECORATION_BLOCK                  = 0x00000001,
  SPV_REFLECT_DECORATION_BUFFER_BLOCK           = 0x00000002,
  SPV_REFLECT_DECORATION_ROW_MAJOR              = 0x00000004,
  SPV_REFLECT_DECORATION_COLUMN_MAJOR           = 0x00000008,
  SPV_REFLECT_DECORATION_BUILT_IN               = 0x00000010,
  SPV_REFLECT_DECORATION_NOPERSPECTIVE          = 0x00000020,
  SPV_REFLECT_DECORATION_FLAT                   = 0x00000040,
} SpvReflectDecorationBits;

typedef uint32_t SpvReflectDecorations;

//! \enum SpvReflectResourceType
//!
//!
typedef enum SpvReflectResourceType {
  SPV_REFLECT_RESOURCE_FLAG_UNDEFINED           = 0x00000000,
  SPV_REFLECT_RESOURCE_FLAG_SAMPLER             = 0x00000001,
  SPV_REFLECT_RESOURCE_FLAG_CBV                 = 0x00000002,
  SPV_REFLECT_RESOURCE_FLAG_SRV                 = 0x00000004,
  SPV_REFLECT_RESOURCE_FLAG_UAV                 = 0x00000008,
} SpvReflectResourceType;

enum {
  SPV_REFLECT_MAX_ARRAY_DIMS                    = 32,
};

typedef struct SpvReflectNumericTraits {
  struct Scalar {
    uint32_t                        width;
    uint32_t                        signedness;
  } scalar;

  struct Vector {
    uint32_t                        component_count;
  } vector;

  struct Matrix {
    uint32_t                        column_count;
    uint32_t                        row_count;
    uint32_t                        stride; // Measured in bytes
  } matrix;
} SpvReflectNumericTraits;

typedef struct SpvReflectImageTraits {
  SpvDim                            dim;
  uint32_t                          depth;
  uint32_t                          arrayed;
  uint32_t                          ms;
  uint32_t                          sampled;
  SpvImageFormat                    image_format;
} SpvReflectImageTraits;

typedef struct SpvReflectArrayTraits {
  uint32_t                          dims_count;
  uint32_t                          dims[SPV_REFLECT_MAX_ARRAY_DIMS];
  uint32_t                          stride; // Measured in bytes
} SpvReflectArrayTraits;

typedef struct SpvReflectBindingArrayTraits {
  uint32_t                          dims_count;
  uint32_t                          dims[SPV_REFLECT_MAX_ARRAY_DIMS];
} SpvReflectBindingArrayTraits;

//! \struct SpvReflectType
//!
//!
typedef struct SpvReflectTypeDescription SpvReflectTypeDescription;

typedef struct SpvReflectTypeDescription {
  uint32_t                          id;
  SpvOp                             op;
  const char*                       type_name;
  const char*                       struct_member_name;
  SpvStorageClass                   storage_class;
  SpvReflectTypeFlags               flags;
  SpvReflectDecorations             decorations;

  struct Traits {
    SpvReflectNumericTraits         numeric;
    SpvReflectImageTraits           image;
    SpvReflectArrayTraits           array;
  } traits;

  uint32_t                          member_count;
  SpvReflectTypeDescription*        members;
} SpvReflectTypeDescription;


//! \struct SpvReflectInterfaceVariable
//!
//!
typedef struct SpvReflectInterfaceVariable SpvReflectInterfaceVariable;

typedef struct SpvReflectInterfaceVariable {
  const char*                       name;
  uint32_t                          location;
  SpvStorageClass                   storage_class;
  const char*                       smenatic_name;
  uint32_t                          semantic_index;
  SpvReflectDecorations             decorations;
  SpvReflectNumericTraits           numeric;
  SpvReflectArrayTraits             array;
  uint32_t                          member_count;
  SpvReflectInterfaceVariable*      members;

  SpvReflectTypeDescription*        type_description;
} SpvReflectInterfaceVariable;

//! \struct SpvReflectBlockVariable
//!
//!
typedef struct SpvReflectBlockVariable SpvReflectBlockVariable;

typedef struct SpvReflectBlockVariable {
  const char*                       name;
  uint32_t                          offset;       // Measured in bytes
  uint32_t                          size;         // Measured in bytes
  uint32_t                          padded_size;  // Measured in bytes
  SpvReflectDecorations             decorations;
  SpvReflectNumericTraits           numeric;
  SpvReflectArrayTraits             array;
  uint32_t                          member_count;
  SpvReflectBlockVariable*          members;

  SpvReflectTypeDescription*        type_description;
} SpvReflectBlockVariable;

//! \struct SpvReflectDescriptorBinding
//!
//!
typedef struct SpvReflectDescriptorBinding {
  const char*                       name;
  uint32_t                          binding;
  uint32_t                          set;
  VkDescriptorType                  descriptor_type;
  SpvReflectResourceType            resource_type;
  SpvReflectImageTraits             image;
  SpvReflectBlockVariable           block;
  SpvReflectBindingArrayTraits      array;

  SpvReflectTypeDescription*        type_description;
} SpvReflectDescriptorBinding;

//! \struct SpvReflectDescriptorSet
//!
//!
typedef struct SpvReflectDescriptorSet {
  uint32_t                          set;
  uint32_t                          binding_count;
  SpvReflectDescriptorBinding**     bindings;
} SpvReflectDescriptorSet;

//! \struct SpvReflectShaderReflection
//!
//!
typedef struct SpvReflectShaderReflection {
  const char*                       entry_point_name;
  uint32_t                          entry_point_id;
  SpvSourceLanguage                 source_language;
  uint32_t                          source_language_version;
  SpvExecutionModel                 spirv_execution_model;
  VkShaderStageFlagBits             vulkan_shader_stage;
  uint32_t                          descriptor_binding_count;
  SpvReflectDescriptorBinding*      descriptor_bindings;
  uint32_t                          descriptor_set_count;
  SpvReflectDescriptorSet*          descriptor_sets;
  uint32_t                          input_variable_count;
  SpvReflectInterfaceVariable*      input_variables;
  uint32_t                          output_variable_count;
  SpvReflectInterfaceVariable*      output_variables;
  uint32_t                          push_constant_count;
  SpvReflectBlockVariable*          push_constants;

  struct Internal {
    size_t                          spirv_word_count;
    uint32_t*                       spirv_code;

    size_t                          type_description_count;
    SpvReflectTypeDescription*      type_descriptions;

    struct DescriptorBindingInfo {
      SpvReflectDescriptorBinding*  descriptor;
      uint32_t                      binding_word_offset;
      uint32_t                      set_word_offset;
    } * descriptor_binding_infos;  

    struct InterfaceVariableInfo {
      SpvReflectInterfaceVariable*  variable;
      uint32_t                      word_offset;
    } * interface_variable_infos;

    uint32_t                        set_numbers_count;
    uint32_t*                       set_numbers;
  } * _internal;
    
} SpvReflectShaderReflection;

#if defined(__cplusplus)
extern "C" {
#endif 

//! \fn spvReflectGetShaderReflection
//!
//! \param size      Size in bytes of SPIR-V code.
//! \param p_code    Pointer to SPIR-V code.
//! \param p_module  Pointer to an instance of SpvReflectShaderReflection.
//! \return          SPV_REFLECT_RESULT_SUCCESS on success.
//!
SpvReflectResult spvReflectGetShaderReflection(size_t                       size, 
                                               void*                        p_code, 
                                               SpvReflectShaderReflection*  p_module);

//! \fn spvReflectDestroyShaderReflection
//!
//! \param p_module  Pointer to an instance of SpvReflectShaderReflection.
//!
void spvReflectDestroyShaderReflection(SpvReflectShaderReflection* p_module);

//! \fn spvReflectGetDescriptorBinding
//!
//! \param p_module  Pointer to an instance of SpvReflectShaderReflection.
//! \param index     Index into SpvReflectShaderReflection.descriptor_bindings, relative to SpvReflectShaderReflection.descriptor_binding_count.
//! \param p_result  Optional. 
//!                  Returns SPV_REFLECT_RESULT_SUCCESS on success. 
//!                  Returns SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND if \a index is out of bounds.
//!                  Returns SPV_REFLECT_RESULT_ERROR_NULL_POINTER if \a p_module is null.
//! \return          Returns a pointer to the descriptor binding at \index.
//!
SpvReflectDescriptorBinding* spvReflectGetDescriptorBinding(const SpvReflectShaderReflection* p_module, 
                                                            uint32_t                          index, 
                                                            SpvReflectResult*                 p_result);

//! \fn spvReflectGetDescriptorSet
//!
//! \param p_module  Pointer to an instance of SpvReflectShaderReflection.
//! \param index     Index into SpvReflectShaderReflection.descriptor_sets, relative to SpvReflectShaderReflection.descriptor_set_count.
//! \param p_result  Optional. 
//!                  Returns SPV_REFLECT_RESULT_SUCCESS on success. 
//!                  Returns SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND if \a index is out of bounds.
//!                  Returns SPV_REFLECT_RESULT_ERROR_NULL_POINTER if \a p_module is null.
//! \return          Returns a pointer to the descriptor set at \index.
//!
SpvReflectDescriptorSet* spvReflectGetDescriptorSet(const SpvReflectShaderReflection* p_module, 
                                                    uint32_t                          index, 
                                                    SpvReflectResult*                 p_result);

//! \fn spvReflectGetInputVariable
//!
//! \param p_module  Pointer to an instance of SpvReflectShaderReflection.
//! \param index     Index into SpvReflectShaderReflection.input_variables, relative to SpvReflectShaderReflection.input_variable_count.
//! \param p_result  Optional. 
//!                  Returns SPV_REFLECT_RESULT_SUCCESS on success. 
//!                  Returns SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND if \a index is out of bounds.
//!                  Returns SPV_REFLECT_RESULT_ERROR_NULL_POINTER if \a p_module is null.
//! \return          Returns a pointer to the input variable set at \index.
//!
SpvReflectInterfaceVariable* spvReflectGetInputVariable(const SpvReflectShaderReflection* p_module, 
                                                        uint32_t                          index, 
                                                        SpvReflectResult*                 p_result);

//! \fn spvReflectGetOutputVariable
//!
//! \param p_module  Pointer to an instance of SpvReflectShaderReflection.
//! \param index     Index into SpvReflectShaderReflection.output_variables, relative to SpvReflectShaderReflection.output_variable_count.
//! \param p_result  Optional. 
//!                  Returns SPV_REFLECT_RESULT_SUCCESS on success. 
//!                  Returns SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND if \a index is out of bounds.
//!                  Returns SPV_REFLECT_RESULT_ERROR_NULL_POINTER if \a p_module is null.
//! \return          Returns a pointer to the output variable set at \index.
//!
SpvReflectInterfaceVariable* spvReflectGetOutputVariable(const SpvReflectShaderReflection*  p_module, 
                                                         uint32_t                           index, 
                                                         SpvReflectResult*                  p_result);

//! \fn spvReflectGetOutputVariable
//!
//! \param p_module  Pointer to an instance of SpvReflectShaderReflection.
//! \param index     Index into SpvReflectShaderReflection.push_constants, relative to SpvReflectShaderReflection.push_constant_count.
//! \param p_result  Optional. 
//!                  Returns SPV_REFLECT_RESULT_SUCCESS on success. 
//!                  Returns SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND if \a index is out of bounds.
//!                  Returns SPV_REFLECT_RESULT_ERROR_NULL_POINTER if \a p_module is null.
//! \return          Returns a pointer to the push constant at \index.
//!
SpvReflectBlockVariable* spvReflectGetPushConstant(const SpvReflectShaderReflection*  p_module, 
                                                   uint32_t                           index, 
                                                   SpvReflectResult*                  p_result);

//! \fn spvReflectSourceLanguage
//! \return  Returns string of source language specified in \a source_lang.
//!
const char* spvReflectSourceLanguage(SpvSourceLanguage source_lang);

#if defined(__cplusplus)
};
#endif

#if defined(__cplusplus)
#include <cstdlib>

namespace spv_reflect {

//! \class ShaderReflection
//!
//!
class ShaderReflection {
public:
  ShaderReflection() {}

  ShaderReflection(size_t size, void* p_code, bool copy = false) : m_size(size), m_code(static_cast<uint8_t*>(p_code)), m_copied_code(copy) {
    if (m_copied_code) {    
      m_code = new uint8_t[size];
      m_result = (m_code != nullptr) ? m_result : SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
      if (m_result == SPV_REFLECT_RESULT_SUCCESS) {
        std::memcpy(m_code, p_code, m_size);
      }
    }

    if (m_result == SPV_REFLECT_RESULT_NOT_READY) {
      m_result = spvReflectGetShaderReflection(m_size, m_code, &m_reflection);
    }
  }

  ~ShaderReflection() {
    spvReflectDestroyShaderReflection(&m_reflection);
    if (m_copied_code && (m_code != nullptr)) {
      delete m_code;
      m_code = nullptr;
      m_size = 0;
      m_copied_code = false;
    }
  }

  const SpvReflectShaderReflection& GetShaderRelection() const {
    return m_reflection;
  }

  uint32_t GetDescriptorBindingCount() const {
    return m_reflection.descriptor_binding_count;
  }

  uint32_t GetDescriptorSetCount() const {
    return m_reflection.descriptor_set_count;
  }

  uint32_t GetInputVariableCount() const {
    return m_reflection.input_variable_count;
  }

  uint32_t GetOuptutVariableCount() const {
    return m_reflection.output_variable_count;
  }

  uint32_t GetPushConstantCount() const {
    return m_reflection.push_constant_count;
  }
  
  SpvReflectDescriptorBinding* GetDescriptorBinding(uint32_t index, SpvReflectResult* p_result = nullptr) {
    return spvReflectGetDescriptorBinding(&m_reflection, index, p_result);
  }

  SpvReflectDescriptorSet* GetDescriptorSet(uint32_t index, SpvReflectResult* p_result = nullptr) {
    return spvReflectGetDescriptorSet(&m_reflection, index, p_result);
  }

  SpvReflectInterfaceVariable* GetInputVariable(uint32_t index, SpvReflectResult* p_result = nullptr) {
    return spvReflectGetInputVariable(&m_reflection, index, p_result);
  }

  SpvReflectInterfaceVariable* GetOutputVariable(uint32_t index, SpvReflectResult* p_result = nullptr) {
    return spvReflectGetOutputVariable(&m_reflection, index, p_result);
  }

  SpvReflectBlockVariable* GetPushConstant(uint32_t index, SpvReflectResult* p_result = nullptr) {
    return spvReflectGetPushConstant(&m_reflection, index, p_result);
  }

private:
  SpvReflectResult            m_result = SPV_REFLECT_RESULT_NOT_READY;
  size_t                      m_size = 0;
  uint8_t*                    m_code = nullptr;
  bool                        m_copied_code = false;
  SpvReflectShaderReflection  m_reflection = {};
};

} // namespace spv_reflect
#endif // defined(__cplusplus)
#endif // SPIRV_REFLECT_H