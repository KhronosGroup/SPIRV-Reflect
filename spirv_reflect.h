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

/*

VERSION HISTORY

  1.0   (2018-03-27) Initial public release

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
  SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER,
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
  uint32_t                            spirv_id;
  const char*                         name;
  uint32_t                            location;
  SpvStorageClass                     storage_class;
  const char*                         semantic;
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
  uint32_t                          offset;           // Measured in bytes
  uint32_t                          absolute_offset;  // Measured in bytes
  uint32_t                          size;             // Measured in bytes
  uint32_t                          padded_size;      // Measured in bytes
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
  uint32_t                            spirv_id;
  const char*                         name;
  uint32_t                            binding;
  uint32_t                            input_attachment_index;
  uint32_t                            set;
  VkDescriptorType                    descriptor_type;
  SpvReflectResourceType              resource_type;
  SpvReflectImageTraits               image;
  SpvReflectBlockVariable             block;
  SpvReflectBindingArrayTraits        array;
  uint32_t                            uav_counter_id;
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
 @return           Returns a const pointer to the compiled SPIR-V bytecode.

*/
const uint32_t* spvReflectGetCode(const SpvReflectShaderModule* p_module);


/*! @fn spvReflectEnumerateDescriptorBindings

 @param  p_module     Pointer to an instance of SpvReflectShaderModule.
 @param  p_count      If pp_bindings is NULL, the module's descriptor binding
                      count (across all descriptor sets) will be stored here.
                      If pp_bindings is not NULL, *p_count must contain the
                      module's descriptor binding count.
 @param  pp_bindings  If NULL, the module's total descriptor binding count
                      will be written to *p_count.
                      If non-NULL, pp_bindings must point to an array with
                      *p_count entries, where pointers to the module's
                      descriptor bindings will be written. The caller must not
                      free the binding pointers written to this array.
 @return              If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                      Otherwise, the error code indicates the cause of the
                      failure.

*/
SpvReflectResult spvReflectEnumerateDescriptorBindings(
  const SpvReflectShaderModule*  p_module,
  uint32_t*                      p_count,
  SpvReflectDescriptorBinding**  pp_bindings
);


/*! @fn spvReflectEnumerateDescriptorSets

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  p_count   If pp_sets is NULL, the module's descriptor set
                   count will be stored here.
                   If pp_sets is not NULL, *p_count must contain the
                   module's descriptor set count.
 @param  pp_sets   If NULL, the module's total descriptor set count
                   will be written to *p_count.
                   If non-NULL, pp_sets must point to an array with
                   *p_count entries, where pointers to the module's
                   descriptor sets will be written. The caller must not
                   free the descriptor set pointers written to this array.
 @return           If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                   Otherwise, the error code indicates the cause of the
                   failure.

*/
SpvReflectResult spvReflectEnumerateDescriptorSets(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectDescriptorSet**     pp_sets
);


/*! @fn spvReflectEnumerateInputVariables

 @param  p_module      Pointer to an instance of SpvReflectShaderModule.
 @param  p_count       If pp_variables is NULL, the module's input variable
                       count will be stored here.
                       If pp_variables is not NULL, *p_count must contain
                       the module's input variable count.
 @param  pp_variables  If NULL, the module's input variable count will be
                       written to *p_count.
                       If non-NULL, pp_variables must point to an array with
                       *p_count entries, where pointers to the module's
                       input variables will be written. The caller must not
                       free the interface variables written to this array.
 @return               If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                       Otherwise, the error code indicates the cause of the
                       failure.

*/
SpvReflectResult spvReflectEnumerateInputVariables(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectInterfaceVariable** pp_variables
);


/*! @fn spvReflectEnumerateOutputVariables

 @param  p_module      Pointer to an instance of SpvReflectShaderModule.
 @param  p_count       If pp_variables is NULL, the module's output variable
                       count will be stored here.
                       If pp_variables is not NULL, *p_count must contain
                       the module's output variable count.
 @param  pp_variables  If NULL, the module's output variable count will be
                       written to *p_count.
                       If non-NULL, pp_variables must point to an array with
                       *p_count entries, where pointers to the module's
                       output variables will be written. The caller must not
                       free the interface variables written to this array.
 @return               If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                       Otherwise, the error code indicates the cause of the
                       failure.

*/
SpvReflectResult spvReflectEnumerateOutputVariables(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectInterfaceVariable** pp_variables
);


/*! @fn spvReflectEnumeratePushConstantBlocks

 @param  p_module   Pointer to an instance of SpvReflectShaderModule.
 @param  p_count    If pp_blocks is NULL, the module's push constant
                    block count will be stored here.
                    If pp_blocks is not NULL, *p_count must
                    contain the module's push constant block count.
 @param  pp_blocks  If NULL, the module's push constant block count
                    will be written to *p_count.
                    If non-NULL, pp_blocks must point to an
                    array with *p_count entries, where pointers to
                    the module's push constant blocks will be written.
                    The caller must not free the block variables written
                    to this array.
 @return            If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                    Otherwise, the error code indicates the cause of the
                    failure.

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

 @param  p_module        Pointer to an instance of SpvReflectShaderModule.
 @param  binding_number  The "binding" value of the requested descriptor
                         binding.
 @param  set_number      The "set" value of the requested descriptor binding.
 @param  p_result        If successful, SPV_REFLECT_RESULT_SUCCESS will be
                         written to *p_result. Otherwise, a error code
                         indicating the cause of the failure will be stored
                         here.
 @return                 If the module contains a descriptor binding that
                         matches the provided [binding_number, set_number]
                         values, a pointer to that binding is returned. The
                         caller must not free this pointer.
                         If no match can be found, or if an unrelated error
                         occurs, the return value will be NULL. Detailed
                         error results are written to *pResult.
@note                    If the module contains multiple desriptor bindings
                         with the same set and binding numbers, there are
                         no guarantees about which binding will be returned.

*/
const SpvReflectDescriptorBinding* spvReflectGetDescriptorBinding(
  const SpvReflectShaderModule* p_module,
  uint32_t                      binding_number,
  uint32_t                      set_number,
  SpvReflectResult*             p_result
);


/*! @fn spvReflectGetDescriptorSet

 @param  p_module    Pointer to an instance of SpvReflectShaderModule.
 @param  set_number  The "set" value of the requested descriptor set.
 @param  p_result    If successful, SPV_REFLECT_RESULT_SUCCESS will be
                     written to *p_result. Otherwise, a error code
                     indicating the cause of the failure will be stored
                     here.
 @return             If the module contains a descriptor set with the
                     provided set_number, a pointer to that set is
                     returned. The caller must not free this pointer.
                     If no match can be found, or if an unrelated error
                     occurs, the return value will be NULL. Detailed
                     error results are written to *pResult.

*/
const SpvReflectDescriptorSet* spvReflectGetDescriptorSet(
  const SpvReflectShaderModule* p_module,
  uint32_t                      set_number,
  SpvReflectResult*             p_result
);


/* @fn spvReflectGetInputVariableByLocation

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  location  The "location" value of the requested input variable.
                   A location of 0xFFFFFFFF will always return NULL
                   with *p_result == ELEMENT_NOT_FOUND.
 @param  p_result  If successful, SPV_REFLECT_RESULT_SUCCESS will be
                   written to *p_result. Otherwise, a error code
                   indicating the cause of the failure will be stored
                   here.
 @return           If the module contains an input interface variable
                   with the provided location value, a pointer to that
                   variable is returned. The caller must not free this
                   pointer.
                   If no match can be found, or if an unrelated error
                   occurs, the return value will be NULL. Detailed
                   error results are written to *pResult.
@note

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

/* @fn spvReflectGetInputVariableBySemantic

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  semantic  The "semantic" value of the requested input variable.
                   A semantic of NULL will return NULL.
                   A semantic of "" will always return NULL with
                   *p_result == ELEMENT_NOT_FOUND.
 @param  p_result  If successful, SPV_REFLECT_RESULT_SUCCESS will be
                   written to *p_result. Otherwise, a error code
                   indicating the cause of the failure will be stored
                   here.
 @return           If the module contains an input interface variable
                   with the provided semantic, a pointer to that
                   variable is returned. The caller must not free this
                   pointer.
                   If no match can be found, or if an unrelated error
                   occurs, the return value will be NULL. Detailed
                   error results are written to *pResult.
@note

*/
const SpvReflectInterfaceVariable* spvReflectGetInputVariableBySemantic(
  const SpvReflectShaderModule* p_module,
  const char*                   semantic,
  SpvReflectResult*             p_result
);

/* @fn spvReflectGetOutputVariableByLocation

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  location  The "location" value of the requested output variable.
                   A location of 0xFFFFFFFF will always return NULL
                   with *p_result == ELEMENT_NOT_FOUND.
 @param  p_result  If successful, SPV_REFLECT_RESULT_SUCCESS will be
                   written to *p_result. Otherwise, a error code
                   indicating the cause of the failure will be stored
                   here.
 @return           If the module contains an output interface variable
                   with the provided location value, a pointer to that
                   variable is returned. The caller must not free this
                   pointer.
                   If no match can be found, or if an unrelated error
                   occurs, the return value will be NULL. Detailed
                   error results are written to *pResult.
@note

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

/* @fn spvReflectGetOutputVariableBySemantic

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  semantic  The "semantic" value of the requested output variable.
                   A semantic of NULL will return NULL.
                   A semantic of "" will always return NULL with
                   *p_result == ELEMENT_NOT_FOUND.
 @param  p_result  If successful, SPV_REFLECT_RESULT_SUCCESS will be
                   written to *p_result. Otherwise, a error code
                   indicating the cause of the failure will be stored
                   here.
 @return           If the module contains an output interface variable
                   with the provided semantic, a pointer to that
                   variable is returned. The caller must not free this
                   pointer.
                   If no match can be found, or if an unrelated error
                   occurs, the return value will be NULL. Detailed
                   error results are written to *pResult.
@note

*/
const SpvReflectInterfaceVariable* spvReflectGetOutputVariableBySemantic(
  const SpvReflectShaderModule*  p_module,
  const char*                    semantic,
  SpvReflectResult*              p_result
);

/*! @fn spvReflectGetPushConstantBlock

 @param  p_module  Pointer to an instance of SpvReflectShaderModule.
 @param  index     The index of the desired block within the module's
                   array of push constant blocks.
 @param  p_result  If successful, SPV_REFLECT_RESULT_SUCCESS will be
                   written to *p_result. Otherwise, a error code
                   indicating the cause of the failure will be stored
                   here.
 @return           If the provided index is within range, a pointer to
                   the corresponding push constant block is returned.
                   The caller must not free this pointer.
                   If no match can be found, or if an unrelated error
                   occurs, the return value will be NULL. Detailed
                   error results are written to *pResult.

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


/*! @fn spvReflectChangeDescriptorBindingNumbers
 @brief  Assign new set and/or binding numbers to a descriptor binding.
         In addition to updating the reflection data, this function modifies
         the underlying SPIR-V bytecode. The updated code can be retrieved
         with spvReflectGetCode().
 @param  p_module            Pointer to an instance of SpvReflectShaderModule.
 @param  p_binding           Pointer to the descriptor binding to modify.
 @param  new_binding_number  The new binding number to assign to the
                             provided descriptor binding.
                             To leave the binding number unchanged, pass
                             SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE.
 @param  new_set_number      The new set number to assign to the
                             provided descriptor binding. Successfully changing
                             a descriptor binding's set number invalidates all
                             existing SpvReflectDescriptorBinding and
                             SpvReflectDescriptorSet pointers from this module.
                             To leave the set number unchanged, pass
                             SPV_REFLECT_SET_NUMBER_DONT_CHANGE.
 @return                     If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                             Otherwise, the error code indicates the cause of
                             the failure.
*/
SpvReflectResult spvReflectChangeDescriptorBindingNumbers(
  SpvReflectShaderModule*            p_module,
  const SpvReflectDescriptorBinding* p_binding,
  uint32_t                           new_binding_number,
  uint32_t                           new_set_number
);
SPV_REFLECT_DEPRECATED("Renamed to spvReflectChangeDescriptorBindingNumbers")
SpvReflectResult spvReflectChangeDescriptorBindingNumber(
  SpvReflectShaderModule*            p_module,
  const SpvReflectDescriptorBinding* p_descriptor_binding,
  uint32_t                           new_binding_number,
  uint32_t                           optional_new_set_number
);

/*! @fn spvReflectChangeDescriptorSetNumber
 @brief  Assign a new set number to an entire descriptor set (including
         all descriptor bindings in that set).
         In addition to updating the reflection data, this function modifies
         the underlying SPIR-V bytecode. The updated code can be retrieved
         with spvReflectGetCode().
 @param  p_module        Pointer to an instance of SpvReflectShaderModule.
 @param  p_set           Pointer to the descriptor binding to modify.
 @param  new_set_number  The new set number to assign to the
                         provided descriptor set, and all its descriptor
                         bindings. Successfully changing a descriptor
                         binding's set number invalidates all existing
                         SpvReflectDescriptorBinding and
                         SpvReflectDescriptorSet pointers from this module.
                         To leave the set number unchanged, pass
                         SPV_REFLECT_SET_NUMBER_DONT_CHANGE.
 @return                 If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                         Otherwise, the error code indicates the cause of
                         the failure.
*/
SpvReflectResult spvReflectChangeDescriptorSetNumber(
  SpvReflectShaderModule*        p_module,
  const SpvReflectDescriptorSet* p_set,
  uint32_t                       new_set_number
);

/*! @fn spvReflectChangeInputVariableLocation
 @brief  Assign a new location to an input interface variable.
         In addition to updating the reflection data, this function modifies
         the underlying SPIR-V bytecode. The updated code can be retrieved
         with spvReflectGetCode().
         It is the caller's responsibility to avoid assigning the same
         location to multiple input variables.
 @param  p_module          Pointer to an instance of SpvReflectShaderModule.
 @param  p_input_variable  Pointer to the input variable to update.
 @param  new_location      The new location to assign to p_input_variable.
 @return                   If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                           Otherwise, the error code indicates the cause of
                           the failure.

*/
SpvReflectResult spvReflectChangeInputVariableLocation(
  SpvReflectShaderModule*            p_module,
  const SpvReflectInterfaceVariable* p_input_variable,
  uint32_t                           new_location
);


/*! @fn spvReflectChangeOutputVariableLocation
 @brief  Assign a new location to an output interface variable.
         In addition to updating the reflection data, this function modifies
         the underlying SPIR-V bytecode. The updated code can be retrieved
         with spvReflectGetCode().
         It is the caller's responsibility to avoid assigning the same
         location to multiple output variables.
 @param  p_module          Pointer to an instance of SpvReflectShaderModule.
 @param  p_output_variable  Pointer to the output variable to update.
 @param  new_location      The new location to assign to p_output_variable.
 @return                   If successful, returns SPV_REFLECT_RESULT_SUCCESS.
                           Otherwise, the error code indicates the cause of
                           the failure.

*/
SpvReflectResult spvReflectChangeOutputVariableLocation(
  SpvReflectShaderModule*             p_module,
  const SpvReflectInterfaceVariable*  p_output_variable,
  uint32_t                            new_location
);


/*! @fn spvReflectSourceLanguage

 @param  source_lang  The source language code.
 @return Returns string of source language specified in \a source_lang.
         The caller must not free the memory associated with this string.
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

  const char*           GetEntryPointName() const;
  VkShaderStageFlagBits GetVulkanShaderStage() const;

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
  const SpvReflectInterfaceVariable*  GetInputVariableBySemantic(const char* semantic,  SpvReflectResult* p_result = nullptr) const;
  const SpvReflectInterfaceVariable*  GetOutputVariableByLocation(uint32_t location, SpvReflectResult*  p_result = nullptr) const;
  SPV_REFLECT_DEPRECATED("Renamed to GetOutputVariableByLocation")
  const SpvReflectInterfaceVariable*  GetOutputVariable(uint32_t location, SpvReflectResult*  p_result = nullptr) const {
    return GetOutputVariableByLocation(location, p_result);
  }
  const SpvReflectInterfaceVariable*  GetOutputVariableBySemantic(const char* semantic, SpvReflectResult*  p_result = nullptr) const;
  const SpvReflectBlockVariable*      GetPushConstantBlock(uint32_t index, SpvReflectResult*  p_result = nullptr) const;
  SPV_REFLECT_DEPRECATED("Renamed to GetPushConstantBlock")
  const SpvReflectBlockVariable*      GetPushConstant(uint32_t index, SpvReflectResult*  p_result = nullptr) const {
    return GetPushConstantBlock(index, p_result);
  }

  SpvReflectResult ChangeDescriptorBindingNumbers(const SpvReflectDescriptorBinding* p_binding,
      uint32_t new_binding_number = SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE,
      uint32_t optional_new_set_number = SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
  SPV_REFLECT_DEPRECATED("Renamed to ChangeDescriptorBindingNumbers")
  SpvReflectResult ChangeDescriptorBindingNumber(const SpvReflectDescriptorBinding* p_binding, uint32_t new_binding_number = SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE,
      uint32_t new_set_number = SPV_REFLECT_SET_NUMBER_DONT_CHANGE) {
    return ChangeDescriptorBindingNumbers(p_binding, new_binding_number, new_set_number);
  }
  SpvReflectResult ChangeDescriptorSetNumber(const SpvReflectDescriptorSet* p_set, uint32_t new_set_number = SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
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

/*! @fn GetShaderStage

  @return Returns Vulkan shader stage

*/
inline VkShaderStageFlagBits ShaderModule::GetVulkanShaderStage() const {
  return m_module.vulkan_shader_stage;
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
inline const SpvReflectInterfaceVariable* ShaderModule::GetInputVariableBySemantic(
  const char*       semantic,
  SpvReflectResult* p_result
) const
{
  return spvReflectGetInputVariableBySemantic(&m_module,
                                              semantic,
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
inline const SpvReflectInterfaceVariable* ShaderModule::GetOutputVariableBySemantic(
  const char*       semantic,
  SpvReflectResult* p_result
) const
{
  return spvReflectGetOutputVariableBySemantic(&m_module,
    semantic,
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


/*! @fn ChangeDescriptorBindingNumbers

  @param  p_binding
  @param  new_binding_number
  @param  new_set_number
  @return

*/
inline SpvReflectResult ShaderModule::ChangeDescriptorBindingNumbers(
  const SpvReflectDescriptorBinding* p_binding,
  uint32_t                           new_binding_number,
  uint32_t                           new_set_number
)
{
  return spvReflectChangeDescriptorBindingNumbers(&m_module,
                                                  p_binding,
                                                  new_binding_number,
                                                  new_set_number);
}


/*! @fn ChangeDescriptorSetNumber

  @param  p_set
  @param  new_set_number
  @return

*/
inline SpvReflectResult ShaderModule::ChangeDescriptorSetNumber(
  const SpvReflectDescriptorSet* p_set,
  uint32_t                       new_set_number
)
{
  return spvReflectChangeDescriptorSetNumber(&m_module,
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
