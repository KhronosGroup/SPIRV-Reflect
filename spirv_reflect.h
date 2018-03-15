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

/*!
 
 @file spirv_reflect.h

*/
#ifndef SPIRV_REFLECT_H
#define SPIRV_REFLECT_H

#include <vulkan/spirv.h>
#include <vulkan/vulkan.h>

#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
#define SPV_REFLECT_DEPRECATED(msg_str) __declspec(deprecated("This symbol is deprecated. Details: " msg_str))
#elif defined(__clang__)
#define SPV_REFLECT_DEPRECATED(msg_str) __attribute__((deprecated(msg_str)))
#elif defined(__GNUC__)
#if GCC_VERSION >= 40500
#define SPV_REFLECT_DEPRECATED(msg_str) __attribute__((deprecated(msg_str)))
#else
#define SPV_REFLECT_DEPRECATED(msg_str) __attribute__((deprecated))
#endif
#else
#define SPV_REFLECT_DEPRECATED(msg_str)
#endif

/*! @enum SpvReflectResult

*/
typedef enum SpvReflectResult {
  SPV_REFLECT_RESULT_SUCCESS,
  SPV_REFLECT_RESULT_NOT_READY,
  SPV_REFLECT_RESULT_ERROR_PARSE_FAILED,
  SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED,
  SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED,
  SPV_REFLECT_RESULT_ERROR_NULL_POINTER,
  SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR,
  SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH,
  SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND,
  SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE,
  SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF,
  SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE,
  SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW,
  SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS,
} SpvReflectResult;

/*! @enum SpvReflectTypeFlagBits

*/
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

/*! @enum SpvReflectDecorationBits

*/
typedef enum SpvReflectDecorationFlagBits {
  SPV_REFLECT_DECORATION_NONE                   = 0x00000000,
  SPV_REFLECT_DECORATION_BLOCK                  = 0x00000001,
  SPV_REFLECT_DECORATION_BUFFER_BLOCK           = 0x00000002,
  SPV_REFLECT_DECORATION_ROW_MAJOR              = 0x00000004,
  SPV_REFLECT_DECORATION_COLUMN_MAJOR           = 0x00000008,
  SPV_REFLECT_DECORATION_BUILT_IN               = 0x00000010,
  SPV_REFLECT_DECORATION_NOPERSPECTIVE          = 0x00000020,
  SPV_REFLECT_DECORATION_FLAT                   = 0x00000040,
  SPV_REFLECT_DECORATION_NON_WRITABLE           = 0x00000080,
} SpvReflectDecorationFlagBits;

typedef uint32_t SpvReflectDecorationFlags;

/*! @enum SpvReflectResourceType

*/
typedef enum SpvReflectResourceType {
  SPV_REFLECT_RESOURCE_FLAG_UNDEFINED           = 0x00000000,
  SPV_REFLECT_RESOURCE_FLAG_SAMPLER             = 0x00000001,
  SPV_REFLECT_RESOURCE_FLAG_CBV                 = 0x00000002,
  SPV_REFLECT_RESOURCE_FLAG_SRV                 = 0x00000004,
  SPV_REFLECT_RESOURCE_FLAG_UAV                 = 0x00000008,
} SpvReflectResourceType;

enum {
  SPV_REFLECT_MAX_ARRAY_DIMS                    = 32,
  SPV_REFLECT_MAX_DESCRIPTOR_SETS               = 64,
};

enum {
  SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE        = ~0,
  SPV_REFLECT_SET_NUMBER_DONT_CHANGE            = ~0
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
  uint32_t                          ms; // 0: single-sampled; 1: multisampled
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

/*! @struct SpvReflectTypeDescription

*/
typedef struct SpvReflectTypeDescription {
  uint32_t                          id;
  SpvOp                             op;
  const char*                       type_name;
  const char*                       struct_member_name;
  SpvStorageClass                   storage_class;
  SpvReflectTypeFlags               type_flags;
  SpvReflectDecorationFlags         decoration_flags;

  struct Traits {
    SpvReflectNumericTraits         numeric;
    SpvReflectImageTraits           image;
    SpvReflectArrayTraits           array;
  } traits;

  uint32_t                          member_count;
  struct SpvReflectTypeDescription* members;
} SpvReflectTypeDescription;


/*! @struct SpvReflectInterfaceVariable

*/
typedef struct SpvReflectInterfaceVariable {
  const char*                         name;
  uint32_t                            location;
  SpvStorageClass                     storage_class;
  const char*                         semantic_name;
  uint32_t                            semantic_index;
  SpvReflectDecorationFlags           decoration_flags;
  SpvBuiltIn                          built_in;
  SpvReflectNumericTraits             numeric;
  SpvReflectArrayTraits               array;
  uint32_t                            member_count;
  struct SpvReflectInterfaceVariable* members;

  VkFormat                            format;

  // NOTE: SPIR-V shares type references for variables
  //       that have the same underlying type. This means
  //       that the same type name will appear for multiple 
  //       variables.
  SpvReflectTypeDescription*          type_description;

  struct {
    uint32_t                          location;
  } word_offset;
} SpvReflectInterfaceVariable;

/*! @struct SpvReflectBlockVariable

*/
typedef struct SpvReflectBlockVariable {
  const char*                       name;
  uint32_t                          offset;       // Measured in bytes
  uint32_t                          size;         // Measured in bytes
  uint32_t                          padded_size;  // Measured in bytes
  SpvReflectDecorationFlags         decoration_flags;
  SpvReflectNumericTraits           numeric;
  SpvReflectArrayTraits             array;
  uint32_t                          member_count;
  struct SpvReflectBlockVariable*   members;

  SpvReflectTypeDescription*        type_description;
} SpvReflectBlockVariable;

/*! @struct SpvReflectDescriptorBinding

*/
typedef struct SpvReflectDescriptorBinding {
  const char*                         name;
  uint32_t                            binding;
  uint32_t                            input_attachment_index;
  uint32_t                            set;
  VkDescriptorType                    descriptor_type;
  SpvReflectResourceType              resource_type;
  SpvReflectImageTraits               image;
  SpvReflectBlockVariable             block;
  SpvReflectBindingArrayTraits        array;
  struct SpvReflectDescriptorBinding* uav_counter_binding;

  SpvReflectTypeDescription*          type_description;

  struct {
    uint32_t                          binding;
    uint32_t                          set;
  } word_offset;
} SpvReflectDescriptorBinding;

/*! @struct SpvReflectDescriptorSet

*/
typedef struct SpvReflectDescriptorSet {
  uint32_t                          set;
  uint32_t                          binding_count;
  SpvReflectDescriptorBinding**     bindings;
} SpvReflectDescriptorSet;

/*! @struct SpvReflectShaderModule

*/
typedef struct SpvReflectShaderModule {
  const char*                       entry_point_name;
  uint32_t                          entry_point_id;
  SpvSourceLanguage                 source_language;
  uint32_t                          source_language_version;
  SpvExecutionModel                 spirv_execution_model;
  VkShaderStageFlagBits             vulkan_shader_stage;
  uint32_t                          descriptor_binding_count;
  SpvReflectDescriptorBinding*      descriptor_bindings;
  uint32_t                          descriptor_set_count;
  SpvReflectDescriptorSet           descriptor_sets[SPV_REFLECT_MAX_DESCRIPTOR_SETS];
  uint32_t                          input_variable_count;
  SpvReflectInterfaceVariable*      input_variables;
  uint32_t                          output_variable_count;
  SpvReflectInterfaceVariable*      output_variables;
  uint32_t                          push_constant_block_count;
  SpvReflectBlockVariable*          push_constant_blocks;

  struct Internal {
    size_t                          spirv_size;
    uint32_t*                       spirv_code;
    uint32_t                        spirv_word_count;

    size_t                          type_description_count;
    SpvReflectTypeDescription*      type_descriptions;
  } * _internal;
    
} SpvReflectShaderModule;

#if defined(__cplusplus)
extern "C" {
#endif 

/*! @fn spvReflectCreateShaderModule

 @param  size      Size in bytes of SPIR-V code.
 @param  p_code    Pointer to SPIR-V code.
 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @return           SPV_REFLECT_RESULT_SUCCESS on success.

*/
SpvReflectResult spvReflectCreateShaderModule(
  size_t                   size,
  const void*              p_code,
  SpvReflectShaderModule*  p_module
);

SPV_REFLECT_DEPRECATED("renamed to spvReflectCreateShaderModule")
SpvReflectResult spvReflectGetShaderModule(
  size_t                   size,
  const void*              p_code,
  SpvReflectShaderModule*  p_module
);


/*! @fn spvReflectDestroyShaderModule

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.

*/
void spvReflectDestroyShaderModule(SpvReflectShaderModule* p_module);


/*! @fn spvReflectGetCodeSize

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @return           Returns the size of the SPIR-V in bytes

*/
uint32_t spvReflectGetCodeSize(const SpvReflectShaderModule* p_module);


/*! @fn spvReflectGetCode

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @return           Returns a const pointer to the SPIR-V code.

*/
const uint32_t* spvReflectGetCode(const SpvReflectShaderModule* p_module);


/*! @fn spvReflectEnumerateBindings

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_count   
 @param  pp_bindings
 @return

*/
SpvReflectResult spvReflectEnumerateDescriptorBindings(
  const SpvReflectShaderModule*  p_module,
  uint32_t*                      p_count,
  SpvReflectDescriptorBinding**  pp_bindings
);


/*! @fn spvReflectEnumerateSets

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_count
 @param  p_set_numbers
 @return

*/
SpvReflectResult spvReflectEnumerateDescriptorSets(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectDescriptorSet**     pp_sets
);


/*! @fn spvReflectEnumerateInputVariables

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_count
 @param  p_locations
 @return

*/
SpvReflectResult spvReflectEnumerateInputVariables(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectInterfaceVariable** pp_variables
);


/*! @fn spvReflectEnumerateInputVariables

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_count
 @param  p_locations
 @return

*/
SpvReflectResult spvReflectEnumerateOutputVariables(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectInterfaceVariable** pp_variables
);


/*! @fn spvReflectEnumeratePushConstantBlocks

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_count
 @return

*/
SpvReflectResult spvReflectEnumeratePushConstantBlocks(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectBlockVariable**     pp_blocks
);
SPV_REFLECT_DEPRECATED("renamed to spvReflectEnumeratePushConstantBlocks")
SpvReflectResult spvReflectEnumeratePushConstants(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectBlockVariable**     pp_blocks
);


/*! @fn spvReflectGetDescriptorBinding

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  binding_number
 @param  set_number
 @param  p_result
 @return

*/
const SpvReflectDescriptorBinding* spvReflectGetDescriptorBinding(
  const SpvReflectShaderModule* p_module,
  uint32_t                      binding_number,
  uint32_t                      set_number,
  SpvReflectResult*             p_result
);


/*! @fn spvReflectGetDescriptorSet

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  set_number
 @param  p_result
 @return

*/
const SpvReflectDescriptorSet* spvReflectGetDescriptorSet(
  const SpvReflectShaderModule* p_module,
  uint32_t                      set_number, 
  SpvReflectResult*             p_result
);


/* @fn spvReflectGetInputVariableByLocation

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  location
 @param  p_result
 @return

*/
const SpvReflectInterfaceVariable* spvReflectGetInputVariableByLocation(
  const SpvReflectShaderModule* p_module,
  uint32_t                      location,
  SpvReflectResult*             p_result
);
SPV_REFLECT_DEPRECATED("renamed to spvReflectGetInputVariableByLocation")
const SpvReflectInterfaceVariable* spvReflectGetInputVariable(
  const SpvReflectShaderModule* p_module,
  uint32_t                      location,
  SpvReflectResult*             p_result
);


/* @fn spvReflectGetInputVariableByLocation

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  set_number
 @param  location
 @return

*/
const SpvReflectInterfaceVariable* spvReflectGetOutputVariableByLocation(
  const SpvReflectShaderModule*  p_module,
  uint32_t                       location,
  SpvReflectResult*              p_result
);
SPV_REFLECT_DEPRECATED("renamed to spvReflectGetOutputVariableByLocation")
const SpvReflectInterfaceVariable* spvReflectGetOutputVariable(
  const SpvReflectShaderModule*  p_module,
  uint32_t                       location,
  SpvReflectResult*              p_result
);


/*! @fn spvReflectGetPushConstantBlock

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  index
 @param  location
 @return

*/
const SpvReflectBlockVariable* spvReflectGetPushConstantBlock(
  const SpvReflectShaderModule*  p_module,
  uint32_t                       index,
  SpvReflectResult*              p_result
);
SPV_REFLECT_DEPRECATED("renamed to spvReflectGetPushConstantBlock")
const SpvReflectBlockVariable* spvReflectGetPushConstant(
  const SpvReflectShaderModule*  p_module,
  uint32_t                       index,
  SpvReflectResult*              p_result
);


/*! @fn spvReflectRemapDescriptorBinding

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_descriptor_binding
 @param  new_binding_number
 @param  optional_new_set_number
 @return

*/
SpvReflectResult spvReflectRemapDescriptorBinding(
  SpvReflectShaderModule*            p_module,
  const SpvReflectDescriptorBinding* p_binding,
  uint32_t                           new_binding_number,
  uint32_t                           new_set_number
);
SPV_REFLECT_DEPRECATED("Renamed to spvReflectRemapDescriptorBinding")
SpvReflectResult spvReflectChangeDescriptorBindingNumber(
  SpvReflectShaderModule*            p_module,
  const SpvReflectDescriptorBinding* p_descriptor_binding,
  uint32_t                           new_binding_number,
  uint32_t                           optional_new_set_number
);

/*! @fn spvReflectRemapDescriptorSet


 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_descriptor_set
 @param  new_set_number
 @return

*/
SpvReflectResult spvReflectRemapDescriptorSet(
  SpvReflectShaderModule*        p_module,
  const SpvReflectDescriptorSet* p_set,
  uint32_t                       new_set_number
);
SPV_REFLECT_DEPRECATED("Renamed to spvReflectRemapDescriptorSet")
SpvReflectResult spvReflectChangeDescriptorSetNumber(
  SpvReflectShaderModule*        p_module,
  const SpvReflectDescriptorSet* p_descriptor_set,
  uint32_t                       new_set_number
);


/*! @fn spvReflectChangeInputVariableLocation

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_input_variable
 @param  new_location
 @return

*/
SpvReflectResult spvReflectChangeInputVariableLocation(
  SpvReflectShaderModule*            p_module,
  const SpvReflectInterfaceVariable* p_input_variable,
  uint32_t                           new_location
);


/*! @fn spvReflectChangeOutputVariableLocation

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_output_variable
 @param  new_location
 @return

*/
SpvReflectResult spvReflectChangeOutputVariableLocation(
  SpvReflectShaderModule*             p_module,
  const SpvReflectInterfaceVariable*  p_output_variable,
  uint32_t                            new_location
);


/*! @fn spvReflectSourceLanguage

 @param  source_lang
 @return Returns string of source language specified in \a source_lang.

*/
const char* spvReflectSourceLanguage(SpvSourceLanguage source_lang);

#if defined(__cplusplus)
};
#endif

#if defined(__cplusplus)
#include <cstdlib>
#include <string>
#include <vector>

namespace spv_reflect {

/*! \class ShaderModule

*/
class ShaderModule {
public:
  ShaderModule();
  ShaderModule(size_t size, const void* p_code);
  ~ShaderModule();

  SpvReflectResult GetResult() const;

  const SpvReflectShaderModule& GetShaderModule() const;

  uint32_t        GetCodeSize() const;
  const uint32_t* GetCode() const;
  const char*     GetEntryPointName() const;

  SpvReflectResult  EnumerateDescriptorBindings(uint32_t* p_count, SpvReflectDescriptorBinding** pp_bindings) const;
  SpvReflectResult  EnumerateDescriptorSets( uint32_t* p_count, SpvReflectDescriptorSet** pp_sets) const ;
  SpvReflectResult  EnumerateInputVariables(uint32_t* p_count,SpvReflectInterfaceVariable** pp_variables) const;
  SpvReflectResult  EnumerateOutputVariables(uint32_t* p_count,SpvReflectInterfaceVariable** pp_variables) const;
  SpvReflectResult  EnumeratePushConstantBlocks(uint32_t* p_count, SpvReflectBlockVariable** pp_blocks) const;
  SPV_REFLECT_DEPRECATED("Renamed to EnumeratePushConstantBlocks")
  SpvReflectResult  EnumeratePushConstants(uint32_t* p_count, SpvReflectBlockVariable** pp_blocks) const {
    return EnumeratePushConstantBlocks(p_count, pp_blocks);
  }

  const SpvReflectDescriptorBinding*  GetDescriptorBinding(uint32_t binding_number, uint32_t set_number, SpvReflectResult* p_result = nullptr) const;
  const SpvReflectDescriptorSet*      GetDescriptorSet(uint32_t set_number, SpvReflectResult* p_result = nullptr) const; 
  const SpvReflectInterfaceVariable*  GetInputVariableByLocation(uint32_t location,  SpvReflectResult* p_result = nullptr) const;
  SPV_REFLECT_DEPRECATED("Renamed to GetInputVariableByLocation")
  const SpvReflectInterfaceVariable*  GetInputVariable(uint32_t location,  SpvReflectResult* p_result = nullptr) const {
    return GetInputVariableByLocation(location, p_result);
  }
  const SpvReflectInterfaceVariable*  GetOutputVariableByLocation(uint32_t location, SpvReflectResult*  p_result = nullptr) const;
  SPV_REFLECT_DEPRECATED("Renamed to GetOutputVariableByLocation")
  const SpvReflectInterfaceVariable*  GetOutputVariable(uint32_t location, SpvReflectResult*  p_result = nullptr) const {
    return GetOutputVariableByLocation(location, p_result);
  }
  const SpvReflectBlockVariable*      GetPushConstantBlock(uint32_t index, SpvReflectResult*  p_result = nullptr) const;
  SPV_REFLECT_DEPRECATED("Renamed to GetPushConstantBlock")
  const SpvReflectBlockVariable*      GetPushConstant(uint32_t index, SpvReflectResult*  p_result = nullptr) const {
    return GetPushConstantBlock(index, p_result);
  }

  SpvReflectResult RemapDescriptorBinding(const SpvReflectDescriptorBinding* p_binding,
      uint32_t new_binding_number = SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE,
      uint32_t optional_new_set_number = SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
  SPV_REFLECT_DEPRECATED("Renamed to RemapDescriptorBinding")
  SpvReflectResult ChangeDescriptorBindingNumber(const SpvReflectDescriptorBinding* p_binding, uint32_t new_binding_number = SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE,
      uint32_t new_set_number = SPV_REFLECT_SET_NUMBER_DONT_CHANGE) {
    return RemapDescriptorBinding(p_binding, new_binding_number, new_set_number);
  }
  SpvReflectResult RemapDescriptorSet(const SpvReflectDescriptorSet* p_set, uint32_t new_set_number = SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
  SPV_REFLECT_DEPRECATED("Renamed to RemapDescriptorSet")
  SpvReflectResult ChangeDescriptorSetNumber(const SpvReflectDescriptorSet* p_set, uint32_t new_set_number = SPV_REFLECT_SET_NUMBER_DONT_CHANGE) {
    return RemapDescriptorSet(p_set, new_set_number);
  }
  SpvReflectResult ChangeInputVariableLocation(const SpvReflectInterfaceVariable* p_input_variable, uint32_t new_location);
  SpvReflectResult ChangeOutputVariableLocation(const SpvReflectInterfaceVariable* p_output_variable, uint32_t new_location);

private:
  mutable SpvReflectResult  m_result = SPV_REFLECT_RESULT_NOT_READY;
  SpvReflectShaderModule    m_module = {};
};


// =================================================================================================
// ShaderModule
// =================================================================================================

/*! @fn ShaderModule
  
*/
inline ShaderModule::ShaderModule() {}


/*! @fn ShaderModule

  @param  size
  @param  p_code

*/
inline ShaderModule::ShaderModule(size_t size, const void* p_code) {
  m_result = spvReflectCreateShaderModule(size, 
                                        p_code, 
                                        &m_module);
}


/*! @fn  ~ShaderModule

*/
inline ShaderModule::~ShaderModule() {
  spvReflectDestroyShaderModule(&m_module);
}


/*! @fn GetResult

  @return

*/
inline SpvReflectResult ShaderModule::GetResult() const {
  return m_result;
}


/*! @fn GetShaderModule

  @return

*/
inline const SpvReflectShaderModule& ShaderModule::GetShaderModule() const {
  return m_module;
}


/*! @fn GetCodeSize

  @return

  */
inline uint32_t ShaderModule::GetCodeSize() const {
  return spvReflectGetCodeSize(&m_module);
}


/*! @fn GetCode

  @return

*/
inline const uint32_t* ShaderModule::GetCode() const {
  return spvReflectGetCode(&m_module);
}

  
/*! @fn GetEntryPoint

  @return Returns entry point

*/
inline const char* ShaderModule::GetEntryPointName() const {
  return m_module.entry_point_name;
}


/*! @fn EnumerateDescriptorBindings

  @param  count
  @param  p_binding_numbers
  @param  p_set_numbers
  @return

*/
inline SpvReflectResult ShaderModule::EnumerateDescriptorBindings(
  uint32_t*                     p_count,
  SpvReflectDescriptorBinding** pp_bindings
) const 
{
  m_result = spvReflectEnumerateDescriptorBindings(&m_module,
                                                    p_count,
                                                    pp_bindings);
  return m_result;
}


/*! @fn EnumerateDescriptorSets

  @param  count
  @param  p_set_numbers
  @return

*/
inline SpvReflectResult ShaderModule::EnumerateDescriptorSets(
  uint32_t*                 p_count,
  SpvReflectDescriptorSet** pp_sets
) const 
{
  m_result = spvReflectEnumerateDescriptorSets(&m_module, 
                                                p_count,
                                                pp_sets);
  return m_result;
}


/*! @fn EnumerateInputVariables

  @param  count
  @param  p_locations
  @return

*/
inline SpvReflectResult ShaderModule::EnumerateInputVariables(
  uint32_t*                     p_count,
  SpvReflectInterfaceVariable** pp_variables
) const 
{
  m_result = spvReflectEnumerateInputVariables(&m_module,
                                                p_count,
                                                pp_variables);
  return m_result;
}


/*! @fn EnumerateOutputVariables

  @param  count
  @param  p_locations
  @return

*/
inline SpvReflectResult ShaderModule::EnumerateOutputVariables(
  uint32_t*                     p_count,
  SpvReflectInterfaceVariable** pp_variables
) const 
{
  m_result = spvReflectEnumerateOutputVariables(&m_module,
                                                p_count,
                                                pp_variables);
  return m_result;
}


/*! @fn EnumerateOutputVariables

  @param  count
  @param  p_locations
  @return

*/
inline SpvReflectResult ShaderModule::EnumeratePushConstantBlocks(
  uint32_t*                 p_count,
  SpvReflectBlockVariable** pp_blocks
) const 
{
  m_result = spvReflectEnumeratePushConstantBlocks(&m_module,
                                              p_count,
                                              pp_blocks);
  return m_result;
}
  

/*! @fn GetDescriptorBinding

  @param  binding_number
  @param  set_number
  @param  p_result
  @return

*/
inline const SpvReflectDescriptorBinding* ShaderModule::GetDescriptorBinding(
  uint32_t          binding_number,                                                           
  uint32_t          set_number, 
  SpvReflectResult* p_result
) const 
{
  return spvReflectGetDescriptorBinding(&m_module, 
                                        binding_number, 
                                        set_number, 
                                        p_result);
}


/*! @fn GetDescriptorSet

  @param  set_number
  @param  p_result
  @return

*/
inline const SpvReflectDescriptorSet* ShaderModule::GetDescriptorSet(
  uint32_t          set_number,
  SpvReflectResult* p_result
) const 
{
  return spvReflectGetDescriptorSet(&m_module, 
                                    set_number, 
                                    p_result);
}


/*! @fn GetInputVariable

  @param  location
  @param  p_result
  @return

*/
inline const SpvReflectInterfaceVariable* ShaderModule::GetInputVariableByLocation(
  uint32_t          location, 
  SpvReflectResult* p_result
) const 
{
  return spvReflectGetInputVariableByLocation(&m_module,
                                    location,
                                    p_result);
}


/*! @fn GetOutputVariable

  @param  location
  @param  p_result
  @return

*/
inline const SpvReflectInterfaceVariable* ShaderModule::GetOutputVariableByLocation(
  uint32_t           location, 
  SpvReflectResult*  p_result
) const 
{
  return spvReflectGetOutputVariableByLocation(&m_module,
                                      location,
                                      p_result);
}


/*! @fn GetPushConstant

  @param  index
  @param  p_result
  @return

*/
inline const SpvReflectBlockVariable* ShaderModule::GetPushConstantBlock(
  uint32_t           index,
  SpvReflectResult*  p_result
) const 
{
  return spvReflectGetPushConstantBlock(&m_module,
                                    index,
                                    p_result);
}


/*! @fn RemapDescriptorBinding

  @param  p_binding
  @param  new_binding_number
  @param  new_set_number
  @return

*/
inline SpvReflectResult ShaderModule::RemapDescriptorBinding(
  const SpvReflectDescriptorBinding* p_binding,
  uint32_t                           new_binding_number,
  uint32_t                           new_set_number
)
{
  return spvReflectRemapDescriptorBinding(&m_module,
                                          p_binding,
                                          new_binding_number,
                                          new_set_number);
}


/*! @fn RemapDescriptorSet

  @param  p_set
  @param  new_set_number
  @return

*/
inline SpvReflectResult ShaderModule::RemapDescriptorSet(
  const SpvReflectDescriptorSet* p_set,
  uint32_t                       new_set_number
)
{
  return spvReflectRemapDescriptorSet(&m_module,
                                      p_set,
                                      new_set_number);
}


/*! @fn ChangeInputVariableLocation

  @param  p_input_variable
  @param  new_location
  @return

*/
inline SpvReflectResult ShaderModule::ChangeInputVariableLocation(
  const SpvReflectInterfaceVariable* p_input_variable,
  uint32_t                           new_location)
{
  return spvReflectChangeInputVariableLocation(&m_module, 
                                                p_input_variable, 
                                                new_location);
}


/*! @fn ChangeOutputVariableLocation

  @param  p_input_variable
  @param  new_location
  @return

*/
inline SpvReflectResult ShaderModule::ChangeOutputVariableLocation(
  const SpvReflectInterfaceVariable* p_output_variable,
  uint32_t                           new_location)
{
  return spvReflectChangeOutputVariableLocation(&m_module, 
                                                p_output_variable, 
                                                new_location);
}

} // namespace spv_reflect
#endif // defined(__cplusplus)
#endif // SPIRV_REFLECT_H
