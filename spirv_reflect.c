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

#include "spirv_reflect.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#if defined(WIN32)
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
#else
  #include <stdlib.h>
#endif

// Temporary enums until these make it into SPIR-V/Vulkan
enum {
  SpvReflectOpDecorateId                      = 332,
  SpvReflectOpDecorateStringGOOGLE            = 5632,
  SpvReflectOpMemberDecorateStringGOOGLE      = 5633,
  SpvReflectDecorationHlslCounterBufferGOOGLE = 5634,
  SpvReflectDecorationHlslSemanticGOOGLE      = 5635
};

enum {
  SPIRV_STARTING_WORD_INDEX = 5,
  SPIRV_WORD_SIZE           = sizeof(uint32_t),
  SPIRV_BYTE_WIDTH          = 8,
  SPIRV_MINIMUM_FILE_SIZE   = SPIRV_STARTING_WORD_INDEX * SPIRV_WORD_SIZE,
  SPIRV_DATA_ALIGNMENT      = 4 * SPIRV_WORD_SIZE, // 16
};

enum {
  INVALID_VALUE  = 0xFFFFFFFF,
};

enum {
  MAX_NODE_NAME_LENGTH  = 1024,
};

enum {
  IMAGE_SAMPLED = 1,
  IMAGE_STORAGE = 2
};

typedef struct ArrayTraits {
  uint32_t              element_type_id;
  uint32_t              length_id;
} ArrayTraits;

typedef struct ImageTraits {
  uint32_t              sampled_type_id;
  SpvDim                dim;
  uint32_t              depth;
  uint32_t              arrayed;
  uint32_t              ms;
  uint32_t              sampled;
  SpvImageFormat        image_format;
} ImageTraits;

typedef struct NumberDecoration {
  uint32_t              word_offset;
  uint32_t              value;
} NumberDecoration;

typedef struct StringDecoration {
  uint32_t              word_offset;
  const char*           value;
} StringDecoration;

typedef struct Decorations {
  bool                  is_block;
  bool                  is_buffer_block;
  bool                  is_row_major;
  bool                  is_column_major;
  bool                  is_built_in;
  bool                  is_noperspective;
  bool                  is_flat;
  bool                  is_non_writable;
  NumberDecoration      set;
  NumberDecoration      binding;
  NumberDecoration      input_attachment_index;
  NumberDecoration      location;
  NumberDecoration      offset;
  NumberDecoration      uav_counter_buffer;
  StringDecoration      semantic;
  uint32_t              array_stride;
  uint32_t              matrix_stride;
  SpvBuiltIn            built_in;
} Decorations;

typedef struct Node {
  uint32_t              result_id;
  SpvOp                 op;
  uint32_t              result_type_id;
  uint32_t              type_id;
  SpvStorageClass       storage_class;
  uint32_t              word_offset;
  uint32_t              word_count;
  bool                  is_type;

  ArrayTraits           array_traits;
  ImageTraits           image_traits;
  uint32_t              image_type_id;

  const char*           name;
  Decorations           decorations;
  uint32_t              member_count;
  const char**          member_names;
  Decorations*          member_decorations;
} Node;

typedef struct Parser {
  size_t                spirv_word_count;
  uint32_t*             spirv_code;
  SpvExecutionModel     spirv_execution_model;
  const char*           entry_point_name;
  uint32_t              entry_point_id;
  SpvSourceLanguage     source_language;
  uint32_t              source_language_version;
  size_t                node_count;
  Node*                 nodes;
  size_t                type_count;
  size_t                descriptor_count;
  size_t                interface_variable_count;
  size_t                push_constant_count;
  uint32_t*             io_vars;
} Parser;

//static uint32_t Min(uint32_t a, uint32_t b)
//{
//  return a < b ? a : b;
//}

static uint32_t Max(uint32_t a, uint32_t b)
{
  return a > b ? a : b;
}

static uint32_t RoundUp(uint32_t value, uint32_t multiple)
{
  assert(multiple && ((multiple & (multiple - 1)) == 0));
  return (value + multiple - 1) & ~(multiple - 1);
}

//static bool IsNull(const void* p)
//{
//  bool is_null = (p == NULL);
//  return is_null;
//}

#define IsNull(ptr) \
  (ptr == NULL)

//static bool IsNotNull(const void* p)
//{
//  bool is_not_null = (p != NULL);
//  return is_not_null;
//}

#define IsNotNull(ptr) \
  (ptr != NULL)

#define SafeFree(ptr)    \
  {                      \
     if (ptr != NULL) {  \
       free((void*)ptr); \
       ptr = NULL;       \
     }                   \
  }

static bool InRange(const Parser* p_parser, uint32_t index)
{
  bool in_range = false;
  if (IsNotNull(p_parser)) {
    in_range = (index < p_parser->spirv_word_count);
  }
  return in_range;
}

static SpvReflectResult ReadU32(Parser* p_parser, uint32_t word_offset, uint32_t* p_value)
{
  assert(IsNotNull(p_parser));
  assert(IsNotNull(p_parser->spirv_code));
  assert(InRange(p_parser, word_offset));
  SpvReflectResult result = SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF;
  if (IsNotNull(p_parser) && IsNotNull(p_parser->spirv_code) && InRange(p_parser, word_offset)) {
    *p_value = *(p_parser->spirv_code + word_offset);
    result = SPV_REFLECT_RESULT_SUCCESS;
  }
  return result;
}

#define CHECKED_READU32(parser, word_offset, value)                       \
  {                                                                       \
    SpvReflectResult checked_readu32_result = ReadU32(parser,             \
                                                      word_offset, (uint32_t*)&(value)); \
    if (checked_readu32_result != SPV_REFLECT_RESULT_SUCCESS) {           \
      return checked_readu32_result;                                      \
    }                                                                     \
  }

#define CHECKED_READU32_CAST(parser, word_offset, cast_to_type, value)    \
  {                                                                       \
    uint32_t checked_readu32_cast_u32 = UINT32_MAX;                       \
    SpvReflectResult checked_readu32_cast_result = ReadU32(parser,        \
                                      word_offset,                        \
                                      (uint32_t*)&(checked_readu32_cast_u32)); \
    if (checked_readu32_cast_result != SPV_REFLECT_RESULT_SUCCESS) {      \
      return checked_readu32_cast_result;                                 \
    }                                                                     \
    value = (cast_to_type)checked_readu32_cast_u32;                       \
  }

#define IF_READU32(result, parser, word_offset, value)      \
  if ((result) == SPV_REFLECT_RESULT_SUCCESS) {               \
    result = ReadU32(parser, word_offset, (uint32_t*)&(value)); \
  }

#define IF_READU32_CAST(result, parser, word_offset, cast_to_type, value) \
  if ((result) == SPV_REFLECT_RESULT_SUCCESS) {             \
    uint32_t if_readu32_cast_u32 = UINT32_MAX;              \
    result = ReadU32(parser, word_offset, &if_readu32_cast_u32); \
    if ((result) == SPV_REFLECT_RESULT_SUCCESS) {           \
      value = (cast_to_type)if_readu32_cast_u32;            \
    }                                                       \
  }

static SpvReflectResult ReadStr(Parser* p_parser, uint32_t word_offset, uint32_t word_index, uint32_t word_count, uint32_t* p_buf_size, char *p_buf)
{
  uint32_t limit = (word_offset + word_count);
  assert(IsNotNull(p_parser));
  assert(IsNotNull(p_parser->spirv_code));
  assert(InRange(p_parser, limit));
  SpvReflectResult result = SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF;
  if (IsNotNull(p_parser) && IsNotNull(p_parser->spirv_code) && InRange(p_parser, limit)) {
    const char* c_str = (const char*)(p_parser->spirv_code + word_offset + word_index);
    uint32_t n = word_count * SPIRV_WORD_SIZE;
    uint32_t length_with_terminator = 0;
    for (uint32_t i = 0; i < n; ++i) {
      char c = *(c_str + i);
      if (c == 0) {
        length_with_terminator = i + 1;
        break;
      }
    }

    if (length_with_terminator > 0) {
      result = SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
      if (IsNotNull(p_buf_size) && IsNotNull(p_buf)) {
        result = SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED;
        if (length_with_terminator <= *p_buf_size) {
          memset(p_buf, 0, *p_buf_size);
          memcpy(p_buf, c_str, length_with_terminator);
          result = SPV_REFLECT_RESULT_SUCCESS;
        }
      }
      else {
        if (IsNotNull(p_buf_size)) {
          *p_buf_size = length_with_terminator;
          result = SPV_REFLECT_RESULT_SUCCESS;
        }
      }
    }
  }
  return result;
}

static SpvReflectDecorationFlags ApplyDecorations(const Decorations* p_decoration_fields)
{
  SpvReflectDecorationFlags decorations = SPV_REFLECT_DECORATION_NONE;
  if (p_decoration_fields->is_block) {
    decorations |= SPV_REFLECT_DECORATION_BLOCK;
  }
  if (p_decoration_fields->is_buffer_block) {
    decorations |= SPV_REFLECT_DECORATION_BUFFER_BLOCK;
  }
  if (p_decoration_fields->is_row_major) {
    decorations |= SPV_REFLECT_DECORATION_ROW_MAJOR;
  }
  if (p_decoration_fields->is_column_major) {
    decorations |= SPV_REFLECT_DECORATION_COLUMN_MAJOR;
  }
  if (p_decoration_fields->is_built_in) {
    decorations |= SPV_REFLECT_DECORATION_BUILT_IN;
  }
  if (p_decoration_fields->is_noperspective) {
    decorations |= SPV_REFLECT_DECORATION_NOPERSPECTIVE;
  }
  if (p_decoration_fields->is_flat) {
    decorations |= SPV_REFLECT_DECORATION_FLAT;
  }
  if (p_decoration_fields->is_non_writable) {
    decorations |= SPV_REFLECT_DECORATION_NON_WRITABLE;
  }
  return decorations;
}

static void ApplyNumericTraits(const SpvReflectTypeDescription* p_type, SpvReflectNumericTraits* p_numeric_traits)
{
  memcpy(p_numeric_traits, &p_type->traits.numeric, sizeof(p_type->traits.numeric));
}

static void ApplyArrayTraits(const SpvReflectTypeDescription* p_type, SpvReflectArrayTraits* p_array_traits)
{
  memcpy(p_array_traits, &p_type->traits.array, sizeof(p_type->traits.array));
}

static Node* FindNode(Parser* p_parser, uint32_t result_id)
{
  Node* p_node = NULL;
  for (size_t i = 0; i < p_parser->node_count; ++i) {
    Node* p_elem = &(p_parser->nodes[i]);
    if (p_elem->result_id == result_id) {
      p_node = p_elem;
      break;
    }
  }
  return p_node;
}

static SpvReflectTypeDescription* FindType(SpvReflectShaderModule* p_module, uint32_t type_id)
{
  SpvReflectTypeDescription* p_type = NULL;
  for (size_t i = 0; i < p_module->_internal->type_description_count; ++i) {
    SpvReflectTypeDescription* p_elem = &(p_module->_internal->type_descriptions[i]);
    if (p_elem->id == type_id) {
      p_type = p_elem;
      break;
    }
  }
  return p_type;
}

static SpvReflectResult CreateParser(size_t size, void* p_code, Parser* p_parser)
{
  if (p_code == NULL) {
    return SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
  }

  if (size < SPIRV_MINIMUM_FILE_SIZE) {
    return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE;
  }
  if ((size % 4) != 0) {
    return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE;
  }

  p_parser->spirv_word_count = size / SPIRV_WORD_SIZE;
  p_parser->spirv_code = (uint32_t*)p_code;

  if (p_parser->spirv_code[0] != SpvMagicNumber) {
    return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static void DestroyParser(Parser* p_parser)
{
  if (!IsNull(p_parser->nodes)) {
    for (size_t i = 0; i < p_parser->node_count; ++i) {
      Node* p_node = &(p_parser->nodes[i]);
      if (IsNotNull(p_node->member_names)) {
        SafeFree(p_node->member_names);
      }
      if (IsNotNull(p_node->member_decorations)) {
        SafeFree(p_node->member_decorations);
      }
    }

    SafeFree(p_parser->nodes);
    SafeFree(p_parser->io_vars);
    p_parser->node_count = 0;
  }
}

static SpvReflectResult ParseNodes(Parser* p_parser)
{
  assert(IsNotNull(p_parser));
  assert(IsNotNull(p_parser->spirv_code));

  uint32_t* p_spirv = p_parser->spirv_code;
  uint32_t spirv_word_index = SPIRV_STARTING_WORD_INDEX;

  // Count nodes
  uint32_t node_count = 0;
  while (spirv_word_index < p_parser->spirv_word_count) {
    uint32_t word = p_spirv[spirv_word_index];
    uint32_t node_word_count = (word >> 16) & 0xFFFF;
    spirv_word_index += node_word_count;
    ++node_count;
  }

  if (node_count == 0) {
    return SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF;
  }

  // Allocate nodes
  p_parser->node_count = node_count;
  p_parser->nodes = (Node*)calloc(p_parser->node_count, sizeof(*(p_parser->nodes)));
  if (IsNull(p_parser->nodes)) {
    return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
  }
  // Mark all nodes with an invalid state
  for (uint32_t i = 0; i < node_count; ++i) {
    p_parser->nodes[i].op = (SpvOp)INVALID_VALUE;
    p_parser->nodes[i].storage_class = (SpvStorageClass)INVALID_VALUE;
    p_parser->nodes[i].decorations.set.value = (uint32_t)INVALID_VALUE;
    p_parser->nodes[i].decorations.binding.value = (uint32_t)INVALID_VALUE;
    p_parser->nodes[i].decorations.location.value = (uint32_t)INVALID_VALUE;
    p_parser->nodes[i].decorations.offset.value = (uint32_t)INVALID_VALUE;
    p_parser->nodes[i].decorations.uav_counter_buffer.value = (uint32_t)INVALID_VALUE;
    p_parser->nodes[i].decorations.built_in = (SpvBuiltIn)INVALID_VALUE;
  }

  // Parse nodes
  uint32_t node_index = 0;
  spirv_word_index = SPIRV_STARTING_WORD_INDEX;
  while (spirv_word_index < p_parser->spirv_word_count) {
    uint32_t word = p_spirv[spirv_word_index];
    SpvOp op = (SpvOp)(word & 0xFFFF);
    uint32_t node_word_count = (word >> 16) & 0xFFFF;

    Node* p_node = &(p_parser->nodes[node_index]);
    p_node->op = op;
    p_node->word_offset = spirv_word_index;
    p_node->word_count = node_word_count;

    switch (p_node->op) {
      default: break;

      case SpvOpSource: {
        CHECKED_READU32_CAST(p_parser, p_node->word_offset + 1, SpvSourceLanguage, p_parser->source_language);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_parser->source_language_version);
      }
      break;

      case SpvOpEntryPoint: {
        CHECKED_READU32_CAST(p_parser, p_node->word_offset + 1, SpvExecutionModel, p_parser->spirv_execution_model);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_parser->entry_point_id);

        // Name length is required to calculate next operand
        uint32_t name_start_word_offset = 3;
        uint32_t name_length_with_terminator = 0;
        SpvReflectResult result = ReadStr(p_parser, p_node->word_offset + name_start_word_offset, 0, p_node->word_count, &name_length_with_terminator, NULL);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
          return result;
        }
        p_parser->entry_point_name = (const char*)(p_parser->spirv_code + p_node->word_offset + name_start_word_offset);

        uint32_t name_word_count = RoundUp(name_length_with_terminator, SPIRV_WORD_SIZE) / SPIRV_WORD_SIZE;
        p_parser->interface_variable_count = (p_node->word_count - (name_start_word_offset + name_word_count));
        if (p_parser->interface_variable_count > 0) {
          p_parser->io_vars = (uint32_t*)calloc(p_parser->interface_variable_count, sizeof(*(p_parser->io_vars)));
          if (IsNull(p_parser->io_vars)) {
            return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
          }
        }

        for (uint32_t var_index = 0; var_index < p_parser->interface_variable_count; ++var_index) {
          uint32_t var_result_id = (uint32_t)INVALID_VALUE;
          uint32_t offset = name_start_word_offset + name_word_count + var_index;
          CHECKED_READU32(p_parser, p_node->word_offset + offset, var_result_id);
          *(p_parser->io_vars + var_index) = var_result_id;
        }
      }
      break;

      case SpvOpName:
      case SpvOpMemberName:
      {
        uint32_t member_offset = (p_node->op == SpvOpMemberName) ? 1 : 0;
        uint32_t name_start = p_node->word_offset + member_offset + 2;
        p_node->name = (const char*)(p_parser->spirv_code + name_start);
      }
      break;

      case SpvOpTypeVoid:
      case SpvOpTypeBool:
      case SpvOpTypeInt:
      case SpvOpTypeFloat:
      case SpvOpTypeVector:
      case SpvOpTypeMatrix:
      case SpvOpTypeSampler:
      case SpvOpTypeRuntimeArray:
      case SpvOpTypeStruct:
      case SpvOpTypeOpaque:
      case SpvOpTypeFunction:
      case SpvOpTypeEvent:
      case SpvOpTypeDeviceEvent:
      case SpvOpTypeReserveId:
      case SpvOpTypeQueue:
      case SpvOpTypePipe:
      {
        CHECKED_READU32(p_parser, p_node->word_offset + 1, p_node->result_id);
        p_node->is_type = true;
      }
      break;

      case SpvOpTypeImage: {
        CHECKED_READU32(p_parser, p_node->word_offset + 1, p_node->result_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_node->image_traits.sampled_type_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 3, p_node->image_traits.dim);
        CHECKED_READU32(p_parser, p_node->word_offset + 4, p_node->image_traits.depth);
        CHECKED_READU32(p_parser, p_node->word_offset + 5, p_node->image_traits.arrayed);
        CHECKED_READU32(p_parser, p_node->word_offset + 6, p_node->image_traits.ms);
        CHECKED_READU32(p_parser, p_node->word_offset + 7, p_node->image_traits.sampled);
        CHECKED_READU32(p_parser, p_node->word_offset + 8, p_node->image_traits.image_format);
        p_node->is_type = true;
      }
      break;

      case SpvOpTypeSampledImage: {
        CHECKED_READU32(p_parser, p_node->word_offset + 1, p_node->result_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_node->image_type_id);
        p_node->is_type = true;
      }
      break;

      case SpvOpTypeArray:  {
        CHECKED_READU32(p_parser, p_node->word_offset + 1, p_node->result_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_node->array_traits.element_type_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 3, p_node->array_traits.length_id);
        p_node->is_type = true;
      }
      break;

      case SpvOpTypePointer: {
        CHECKED_READU32(p_parser, p_node->word_offset + 1, p_node->result_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_node->storage_class);
        CHECKED_READU32(p_parser, p_node->word_offset + 3, p_node->type_id);
        p_node->is_type = true;
      }
      break;

      case SpvOpTypeForwardPointer:
      {
        CHECKED_READU32(p_parser, p_node->word_offset + 1, p_node->result_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_node->storage_class);
        p_node->is_type = true;
      }
      break;

      case SpvOpConstantTrue:
      case SpvOpConstantFalse:
      case SpvOpConstant:
      case SpvOpConstantComposite:
      case SpvOpConstantSampler:
      case SpvOpConstantNull: {
        CHECKED_READU32(p_parser, p_node->word_offset + 1, p_node->result_type_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_node->result_id);
      }
      break;

      case SpvOpVariable:
      {
        CHECKED_READU32(p_parser, p_node->word_offset + 1, p_node->type_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 2, p_node->result_id);
        CHECKED_READU32(p_parser, p_node->word_offset + 3, p_node->storage_class);
      }
      break;
    }

    if (p_node->is_type) {
      ++(p_parser->type_count);
    }

    spirv_word_index += node_word_count;
    ++node_index;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseMemberCounts(Parser* p_parser)
{
  assert(IsNotNull(p_parser));
  assert(IsNotNull(p_parser->spirv_code));
  assert(IsNotNull(p_parser->nodes));

  if (IsNotNull(p_parser) && IsNotNull(p_parser->spirv_code) && IsNotNull(p_parser->nodes)) {
    for (size_t i = 0; i < p_parser->node_count; ++i) {
      Node* p_node = &(p_parser->nodes[i]);
      if ((p_node->op != SpvOpMemberName) && (p_node->op != SpvOpMemberDecorate)) {
        continue;
      }

      uint32_t target_id = 0;
      uint32_t member_index = (uint32_t)INVALID_VALUE;
      CHECKED_READU32(p_parser, p_node->word_offset + 1, target_id);
      CHECKED_READU32(p_parser, p_node->word_offset + 2, member_index);
      Node* p_target_node = FindNode(p_parser, target_id);
      // Not all nodes get parsed, so FindNode returning NULL is expected.
      if (IsNull(p_target_node)) {
        continue;
      }

      if (member_index == INVALID_VALUE) {
        return SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED;
      }

      p_target_node->member_count = Max(p_target_node->member_count, member_index + 1);
    }

    for (uint32_t i = 0; i < p_parser->node_count; ++i) {
      Node* p_node = &(p_parser->nodes[i]);
      if (p_node->member_count == 0) {
        continue;
      }

      p_node->member_names = (const char **)calloc(p_node->member_count, sizeof(*(p_node->member_names)));
      if (IsNull(p_node->member_names)) {
        return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
      }

      p_node->member_decorations = (Decorations*)calloc(p_node->member_count, sizeof(*(p_node->member_decorations)));
      if (IsNull(p_node->member_decorations)) {
        return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
      }
    }
  }
  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseNames(Parser* p_parser)
{
  assert(IsNotNull(p_parser));
  assert(IsNotNull(p_parser->spirv_code));
  assert(IsNotNull(p_parser->nodes));

  if (IsNotNull(p_parser) && IsNotNull(p_parser->spirv_code) && IsNotNull(p_parser->nodes)) {
    for (size_t i = 0; i < p_parser->node_count; ++i) {
      Node* p_node = &(p_parser->nodes[i]);
      if ((p_node->op != SpvOpName) && (p_node->op != SpvOpMemberName)) {
        continue;
      }

      uint32_t target_id = 0;
      CHECKED_READU32(p_parser, p_node->word_offset + 1, target_id);
      Node* p_target_node = FindNode(p_parser, target_id);
      // Not all nodes get parsed, so FindNode returning NULL is expected.
      if (IsNull(p_target_node)) {
        continue;
      }

      const char** pp_target_name = &(p_target_node->name);
      if (p_node->op == SpvOpMemberName) {
        uint32_t member_index = UINT32_MAX;
        CHECKED_READU32(p_parser, p_node->word_offset + 2, member_index);
        pp_target_name = &(p_target_node->member_names[member_index]);
      }

      *pp_target_name = p_node->name;
    }
  }
  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseDecorations(Parser* p_parser)
{
  for (uint32_t i = 0; i < p_parser->node_count; ++i) {
    Node* p_node = &(p_parser->nodes[i]);

    if (((uint32_t)p_node->op != (uint32_t)SpvOpDecorate) &&
        ((uint32_t)p_node->op != (uint32_t)SpvOpMemberDecorate) &&
        ((uint32_t)p_node->op != (uint32_t)SpvReflectOpDecorateId) &&
        ((uint32_t)p_node->op != (uint32_t)SpvReflectOpDecorateStringGOOGLE) &&
        ((uint32_t)p_node->op != (uint32_t)SpvReflectOpMemberDecorateStringGOOGLE))
    {
     continue;
    }

    uint32_t target_id = 0;
    CHECKED_READU32(p_parser, p_node->word_offset + 1, target_id);
    Node* p_target_node = FindNode(p_parser, target_id);
    if (IsNull(p_target_node)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }
    Decorations* p_target_decorations = &(p_target_node->decorations);

    uint32_t member_offset = 0;
    if (p_node->op == SpvOpMemberDecorate) {
      member_offset = 1;
      uint32_t member_index = (uint32_t)INVALID_VALUE;
      CHECKED_READU32(p_parser, p_node->word_offset + 2, member_index);
      p_target_decorations = &(p_target_node->member_decorations[member_index]);
    }

    uint32_t decoration = (uint32_t)INVALID_VALUE;
    CHECKED_READU32(p_parser, p_node->word_offset + member_offset + 2, decoration);
    switch (decoration) {
      default: break;

      case SpvDecorationBlock: {
        p_target_decorations->is_block = true;
      }
      break;

      case SpvDecorationBufferBlock: {
        p_target_decorations->is_buffer_block = true;
      }
      break;

      case SpvDecorationColMajor: {
        p_target_decorations->is_column_major = true;
      }
      break;

      case SpvDecorationRowMajor: {
        p_target_decorations->is_row_major = true;
      }
      break;

      case SpvDecorationArrayStride: {
        uint32_t word_offset = p_node->word_offset + member_offset + 3;
        CHECKED_READU32(p_parser, word_offset, p_target_decorations->array_stride);
      }
      break;

      case SpvDecorationMatrixStride: {
        uint32_t word_offset = p_node->word_offset + member_offset + 3;
        CHECKED_READU32(p_parser, word_offset, p_target_decorations->matrix_stride);
      }
      break;

      case SpvDecorationBuiltIn: {
        p_target_decorations->is_built_in = true;
        uint32_t word_offset = p_node->word_offset + member_offset + 3;
        CHECKED_READU32_CAST(p_parser, word_offset, SpvBuiltIn, p_target_decorations->built_in);
      }
      break;

      case SpvDecorationNoPerspective: {
        p_target_decorations->is_noperspective = true;
      }
      break;

      case SpvDecorationFlat: {
        p_target_decorations->is_flat = true;
      }
      break;

      case SpvDecorationNonWritable: {
        p_target_decorations->is_non_writable = true;
      }
      break;

      case SpvDecorationLocation: {
        uint32_t word_offset = p_node->word_offset + member_offset + 3;
        CHECKED_READU32(p_parser, word_offset, p_target_decorations->location.value);
        p_target_decorations->location.word_offset = word_offset;
      }
      break;

      case SpvDecorationBinding: {
        uint32_t word_offset = p_node->word_offset + member_offset+ 3;
        CHECKED_READU32(p_parser, word_offset, p_target_decorations->binding.value);
        p_target_decorations->binding.word_offset = word_offset;
      }
      break;

      case SpvDecorationDescriptorSet: {
        uint32_t word_offset = p_node->word_offset + member_offset+ 3;
        CHECKED_READU32(p_parser, word_offset, p_target_decorations->set.value);
        p_target_decorations->set.word_offset = word_offset;
      }
      break;

      case SpvDecorationOffset: {
        uint32_t word_offset = p_node->word_offset + member_offset+ 3;
        CHECKED_READU32(p_parser, word_offset, p_target_decorations->offset.value);
        p_target_decorations->offset.word_offset = word_offset;
      }
      break;

      case SpvDecorationInputAttachmentIndex: {
        uint32_t word_offset = p_node->word_offset + member_offset+ 3;
        CHECKED_READU32(p_parser, word_offset, p_target_decorations->input_attachment_index.value);
        p_target_decorations->input_attachment_index.word_offset = word_offset;
      }
      break;

      case SpvReflectDecorationHlslCounterBufferGOOGLE: {
        uint32_t word_offset = p_node->word_offset + member_offset+ 3;
        CHECKED_READU32(p_parser, word_offset, p_target_decorations->uav_counter_buffer.value);
        p_target_decorations->uav_counter_buffer.word_offset = word_offset;
      }
      break;

      case SpvReflectDecorationHlslSemanticGOOGLE: {
        uint32_t word_offset = p_node->word_offset + member_offset + 3;
        p_target_decorations->semantic.value = (const char*)(p_parser->spirv_code + word_offset);
        p_target_decorations->semantic.word_offset = word_offset;
      }
      break;
    }
  }
  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseType(Parser* p_parser, Node* p_node, Decorations* p_struct_member_decorations, SpvReflectShaderModule* p_module, SpvReflectTypeDescription* p_type)
{
  SpvReflectResult result = SPV_REFLECT_RESULT_SUCCESS;

  if (p_node->member_count > 0) {
    p_type->member_count = p_node->member_count;
    p_type->members = (SpvReflectTypeDescription*)calloc(p_type->member_count, sizeof(*(p_type->members)));
    if (IsNotNull(p_type->members)) {
      // Mark all members types with an invalid state
      for (size_t i = 0; i < p_type->members->member_count; ++i) {
        SpvReflectTypeDescription* p_member_type = &(p_type->members[i]);
        p_member_type->id = (uint32_t)INVALID_VALUE;
        p_member_type->op = (SpvOp)INVALID_VALUE;
        p_member_type->storage_class = (SpvStorageClass)INVALID_VALUE;
      }
    }
    else {
      result = SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
    }
  }

  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    // Since the parse descends on type information, these will get overwritten
    // if not guarded against assignment. Only assign if the id is invalid.
    if (p_type->id == INVALID_VALUE) {
      p_type->id = p_node->result_id;
      p_type->op = p_node->op;
      p_type->decoration_flags = ApplyDecorations(&p_node->decorations);
    }

    switch (p_node->op) {
      default: break;
      case SpvOpTypeVoid:
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_VOID;
        break;

      case SpvOpTypeBool:
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_BOOL;
        break;

      case SpvOpTypeInt: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_INT;
        IF_READU32(result, p_parser, p_node->word_offset + 2, p_type->traits.numeric.scalar.width);
        IF_READU32(result, p_parser, p_node->word_offset + 3, p_type->traits.numeric.scalar.signedness);
      }
      break;

      case SpvOpTypeFloat: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_FLOAT;
        IF_READU32(result, p_parser, p_node->word_offset + 2, p_type->traits.numeric.scalar.width);
      }
      break;

      case SpvOpTypeVector: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_VECTOR;
        uint32_t component_type_id = (uint32_t)INVALID_VALUE;
        IF_READU32(result, p_parser, p_node->word_offset + 2, component_type_id);
        IF_READU32(result, p_parser, p_node->word_offset + 3, p_type->traits.numeric.vector.component_count);
        // Parse component type
        Node* p_next_node = FindNode(p_parser, component_type_id);
        if (IsNotNull(p_next_node)) {
          result = ParseType(p_parser, p_next_node, NULL, p_module, p_type);
        }
        else {
          result = SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
        }
      }
      break;

      case SpvOpTypeMatrix: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_MATRIX;
        uint32_t column_type_id = (uint32_t)INVALID_VALUE;
        IF_READU32(result, p_parser, p_node->word_offset + 2, column_type_id);
        IF_READU32(result, p_parser, p_node->word_offset + 3, p_type->traits.numeric.matrix.column_count);
        Node* p_next_node = FindNode(p_parser, column_type_id);
        if (IsNotNull(p_next_node)) {
          result = ParseType(p_parser, p_next_node, NULL, p_module, p_type);
        }
        else {
          result = SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
        }
        p_type->traits.numeric.matrix.row_count = p_type->traits.numeric.vector.component_count;
        p_type->traits.numeric.matrix.stride = p_node->decorations.matrix_stride;
        // NOTE: Matrix stride is decorated using OpMemberDecoreate - not OpDecoreate.
        if (IsNotNull(p_struct_member_decorations)) {
          p_type->traits.numeric.matrix.stride = p_struct_member_decorations->matrix_stride;
        }
      }
      break;

      case SpvOpTypeImage: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE;
        IF_READU32_CAST(result, p_parser, p_node->word_offset + 3, SpvDim, p_type->traits.image.dim);
        IF_READU32(result, p_parser, p_node->word_offset + 4, p_type->traits.image.depth);
        IF_READU32(result, p_parser, p_node->word_offset + 5, p_type->traits.image.arrayed);
        IF_READU32(result, p_parser, p_node->word_offset + 6, p_type->traits.image.ms);
        IF_READU32(result, p_parser, p_node->word_offset + 7, p_type->traits.image.sampled);
        IF_READU32_CAST(result, p_parser, p_node->word_offset + 8, SpvImageFormat, p_type->traits.image.image_format);
      }
      break;

      case SpvOpTypeSampler: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLER;
      }
      break;

      case SpvOpTypeSampledImage: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE;
        uint32_t image_type_id = (uint32_t)INVALID_VALUE;
        IF_READU32(result, p_parser, p_node->word_offset + 2, image_type_id);
        Node* p_next_node = FindNode(p_parser, image_type_id);
        if (IsNotNull(p_next_node)) {
          result = ParseType(p_parser, p_next_node, NULL, p_module, p_type);
        }
        else {
          result = SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
        }
      }
      break;

      case SpvOpTypeArray: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_ARRAY;
        if (result == SPV_REFLECT_RESULT_SUCCESS) {
          uint32_t element_type_id = (uint32_t)INVALID_VALUE;
          uint32_t length_id = (uint32_t)INVALID_VALUE;
          IF_READU32(result, p_parser, p_node->word_offset + 2, element_type_id);
          IF_READU32(result, p_parser, p_node->word_offset + 3, length_id);
          // NOTE: Array stride is decorated using OpDecorate instead of
          //       OpMemberDecorate, even if the array is apart of a struct.
          p_type->traits.array.stride = p_node->decorations.array_stride;
          // Get length for current dimension
          Node* p_length_node = FindNode(p_parser, length_id);
          if (IsNotNull(p_length_node)) {
            uint32_t length = 0;
            IF_READU32(result, p_parser, p_length_node->word_offset + 3, length);
            if (result == SPV_REFLECT_RESULT_SUCCESS) {
              // Write the array dim and increment the count and offset
              p_type->traits.array.dims[p_type->traits.array.dims_count] = length;
              p_type->traits.array.dims_count += 1;
              // Parse next dimension or element type
              Node* p_next_node = FindNode(p_parser, element_type_id);
              if (IsNotNull(p_next_node)) {
                result = ParseType(p_parser, p_next_node, NULL, p_module, p_type);
              }
            }
            else {
              result = SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
            }
          }
          else {
            result = SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
          }
        }
      }
      break;

      case SpvOpTypeRuntimeArray: {
        uint32_t element_type_id = (uint32_t)INVALID_VALUE;
        IF_READU32(result, p_parser, p_node->word_offset + 2, element_type_id);
        // Parse next dimension or element type
        Node* p_next_node = FindNode(p_parser, element_type_id);
        if (IsNotNull(p_next_node)) {
          result = ParseType(p_parser, p_next_node, NULL, p_module, p_type);
        }
        else {
          result = SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
        }
      }
      break;

      case SpvOpTypeStruct: {
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_STRUCT;
        p_type->type_flags |= SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK;
        uint32_t word_index = 2;
        uint32_t member_index = 0;
        for (; word_index < p_node->word_count; ++word_index, ++member_index) {
          uint32_t member_id = (uint32_t)INVALID_VALUE;
          IF_READU32(result, p_parser, p_node->word_offset + word_index, member_id);
          // Find member node
          Node* p_member_node = FindNode(p_parser, member_id);
          if (IsNull(p_member_node)) {
            result = SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
            break;
          }

          // Member decorations
          Decorations* p_member_decorations = &p_node->member_decorations[member_index];

          assert(member_index < p_type->member_count);
          // Parse member type
          SpvReflectTypeDescription* p_member_type = &(p_type->members[member_index]);
          p_member_type->id = member_id;
          p_member_type->op = p_member_node->op;
          result = ParseType(p_parser, p_member_node, p_member_decorations, p_module, p_member_type);
          if (result != SPV_REFLECT_RESULT_SUCCESS) {
            break;
          }
          p_member_type->type_name = p_member_node->name;
          p_member_type->struct_member_name = p_node->member_names[member_index];
        }
      }
      break;

      case SpvOpTypeOpaque: break;

      case SpvOpTypePointer: {
        IF_READU32_CAST(result, p_parser, p_node->word_offset + 2, SpvStorageClass, p_type->storage_class);
        uint32_t type_id = (uint32_t)INVALID_VALUE;
        IF_READU32(result, p_parser, p_node->word_offset + 3, type_id);
        // Parse type
        Node* p_next_node = FindNode(p_parser, type_id);
        if (IsNotNull(p_next_node)) {
          result = ParseType(p_parser, p_next_node, NULL, p_module, p_type);
        }
        else {
          result = SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
        }
      }
      break;
    }

    if (result == SPV_REFLECT_RESULT_SUCCESS) {
      // Names get assigned on the way down. Guard against names
      // get overwritten on the way up.
      if (IsNull(p_type->type_name)) {
        p_type->type_name = p_node->name;
      }
    }
  }

  return result;
}

static SpvReflectResult ParseTypes(Parser* p_parser, SpvReflectShaderModule* p_module)
{
  if (p_parser->type_count == 0) {
    return SPV_REFLECT_RESULT_SUCCESS;
  }

  p_module->_internal->type_description_count = p_parser->type_count;
  p_module->_internal->type_descriptions = (SpvReflectTypeDescription*)calloc(p_module->_internal->type_description_count,
                                                                              sizeof(*(p_module->_internal->type_descriptions)));
  if (IsNull(p_module->_internal->type_descriptions)) {
    return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
  }

  // Mark all types with an invalid state
  for (size_t i = 0; i < p_module->_internal->type_description_count; ++i) {
    SpvReflectTypeDescription* p_type = &(p_module->_internal->type_descriptions[i]);
    p_type->id = (uint32_t)INVALID_VALUE;
    p_type->op = (SpvOp)INVALID_VALUE;
    p_type->storage_class = (SpvStorageClass)INVALID_VALUE;
  }

  size_t type_index = 0;
  for (size_t i = 0; i < p_parser->node_count; ++i) {
    Node* p_node = &(p_parser->nodes[i]);
    if (! p_node->is_type) {
      continue;
    }

    SpvReflectTypeDescription* p_type = &(p_module->_internal->type_descriptions[type_index]);
    SpvReflectResult result = ParseType(p_parser, p_node, NULL, p_module, p_type);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
      return result;
    }
    ++type_index;
  }
  return SPV_REFLECT_RESULT_SUCCESS;
}

static int SortCompareDescriptorBinding(const void* a, const void* b)
{
  const SpvReflectDescriptorBinding* p_elem_a = (const SpvReflectDescriptorBinding*)a;
  const SpvReflectDescriptorBinding* p_elem_b = (const SpvReflectDescriptorBinding*)b;
  int value = (int)(p_elem_a->binding) - (int)(p_elem_b->binding);
  return value;
}

static SpvReflectResult ParseDescriptorBindings(Parser* p_parser, SpvReflectShaderModule* p_module)
{
  p_module->descriptor_binding_count = 0;
  for (size_t i = 0; i < p_parser->node_count; ++i) {
    Node* p_node = &(p_parser->nodes[i]);
    if ((p_node->op != SpvOpVariable) ||
        ((p_node->storage_class != SpvStorageClassUniform) && (p_node->storage_class != SpvStorageClassUniformConstant)))
    {
      continue;
    }
    if ((p_node->decorations.set.value == INVALID_VALUE) || (p_node->decorations.binding.value == INVALID_VALUE)) {
      continue;
    }

    p_module->descriptor_binding_count += 1;
  }

  if (p_module->descriptor_binding_count == 0) {
    return SPV_REFLECT_RESULT_SUCCESS;
  }

  p_module->descriptor_bindings = (SpvReflectDescriptorBinding*)calloc(p_module->descriptor_binding_count, sizeof(*(p_module->descriptor_bindings)));
  if (IsNull(p_module->descriptor_bindings)) {
    return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
  }

  // Mark all types with an invalid state
  for (uint32_t descriptor_index = 0; descriptor_index < p_module->descriptor_binding_count; ++descriptor_index) {
    SpvReflectDescriptorBinding* p_descriptor = &(p_module->descriptor_bindings[descriptor_index]);
    p_descriptor->binding = (uint32_t)INVALID_VALUE;
    p_descriptor->input_attachment_index = (uint32_t)INVALID_VALUE;
    p_descriptor->set = (uint32_t)INVALID_VALUE;
    p_descriptor->descriptor_type = (VkDescriptorType)INVALID_VALUE;
    p_descriptor->uav_counter_id = (VkDescriptorType)INVALID_VALUE;
  }

  size_t descriptor_index = 0;
  for (size_t i = 0; i < p_parser->node_count; ++i) {
    Node* p_node = &(p_parser->nodes[i]);
    if ((p_node->op != SpvOpVariable) ||
        ((p_node->storage_class != SpvStorageClassUniform) && (p_node->storage_class != SpvStorageClassUniformConstant)))\
    {
      continue;
    }
    if ((p_node->decorations.set.value == INVALID_VALUE) || (p_node->decorations.binding.value == INVALID_VALUE)) {
      continue;
    }

    SpvReflectTypeDescription* p_type = FindType(p_module, p_node->type_id);
    if (IsNull(p_type)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }
    // If the type is a pointer, resolve it
    if (p_type->op == SpvOpTypePointer) {
      // Find the type's node
      Node* p_type_node = FindNode(p_parser, p_type->id);
      if (IsNull(p_type_node)) {
        return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
      }
      // Should be the resolved type
      p_type = FindType(p_module, p_type_node->type_id);
      if (IsNull(p_type)) {
        return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
      }
    }

    SpvReflectDescriptorBinding* p_descriptor = &p_module->descriptor_bindings[descriptor_index];
    p_descriptor->spirv_id = p_node->result_id;
    p_descriptor->name = p_node->name;
    p_descriptor->binding = p_node->decorations.binding.value;
    p_descriptor->input_attachment_index = p_node->decorations.input_attachment_index.value;
    p_descriptor->set = p_node->decorations.set.value;
    p_descriptor->uav_counter_id = p_node->decorations.uav_counter_buffer.value;
    p_descriptor->type_description = p_type;

    // Copy image traits
    if ((p_type->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_MASK) == SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE) {
      memcpy(&p_descriptor->image, &p_type->traits.image, sizeof(p_descriptor->image));
    }

    // This is a workaround for: https://github.com/KhronosGroup/glslang/issues/1096
    {
      const uint32_t resource_mask = SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE | SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE;
      if ((p_type->type_flags & resource_mask) == resource_mask) {
        memcpy(&p_descriptor->image, &p_type->traits.image, sizeof(p_descriptor->image));
      }
    }

    // Copy array traits
    if (p_type->traits.array.dims_count > 0) {
      p_descriptor->array.dims_count = p_type->traits.array.dims_count;
      for (uint32_t dim_index = 0; dim_index < p_type->traits.array.dims_count; ++dim_index) {
        p_descriptor->array.dims[dim_index] = p_type->traits.array.dims[dim_index];
      }
    }

    p_descriptor->word_offset.binding = p_node->decorations.binding.word_offset;
    p_descriptor->word_offset.set = p_node->decorations.set.word_offset;

    ++descriptor_index;
  }

  if (p_module->descriptor_binding_count > 0) {
    qsort(p_module->descriptor_bindings,
          p_module->descriptor_binding_count,
          sizeof(*(p_module->descriptor_bindings)),
          SortCompareDescriptorBinding);
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseDescriptorType(SpvReflectShaderModule* p_module)
{
  if (p_module->descriptor_binding_count == 0) {
    return SPV_REFLECT_RESULT_SUCCESS;
  }

  for (uint32_t descriptor_index = 0; descriptor_index < p_module->descriptor_binding_count; ++descriptor_index) {
    SpvReflectDescriptorBinding* p_descriptor = &(p_module->descriptor_bindings[descriptor_index]);
    SpvReflectTypeDescription* p_type = p_descriptor->type_description;

    switch (p_type->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_MASK) {
      default: assert(false && "unknown type flag"); break;

      case SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE: {
        if (p_descriptor->image.dim == SpvDimBuffer) {
          switch (p_descriptor->image.sampled) {
            default: assert(false && "unknown texel buffer sampled value"); break;
            case IMAGE_SAMPLED: p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
            case IMAGE_STORAGE: p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
          }
        }
        else if(p_descriptor->image.dim == SpvDimSubpassData) {
          p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        }
        else {
          switch (p_descriptor->image.sampled) {
            default: assert(false && "unknown image sampled value"); break;
            case IMAGE_SAMPLED: p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
            case IMAGE_STORAGE: p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
          }
        }
      }
      break;

      case SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLER: {
        p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLER;
      }
      break;

      case (SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE | SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE): {
        // This is a workaround for: https://github.com/KhronosGroup/glslang/issues/1096
        if (p_descriptor->image.dim == SpvDimBuffer) {
          switch (p_descriptor->image.sampled) {
            default: assert(false && "unknown texel buffer sampled value"); break;
            case IMAGE_SAMPLED: p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
            case IMAGE_STORAGE: p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
          }
        }
        else {
          p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
      }
      break;

      case SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK: {
        if (p_type->decoration_flags & SPV_REFLECT_DECORATION_BLOCK) {
          p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        else if (p_type->decoration_flags & SPV_REFLECT_DECORATION_BUFFER_BLOCK) {
          p_descriptor->descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }
        else {
          assert(false && "unknown struct");
        }
      }
      break;
    }

    switch (p_descriptor->descriptor_type) {
      case VK_DESCRIPTOR_TYPE_SAMPLER                : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_SAMPLER; break;
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : p_descriptor->resource_type = (SpvReflectResourceType)(SPV_REFLECT_RESOURCE_FLAG_SAMPLER | SPV_REFLECT_RESOURCE_FLAG_SRV); break;
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE          : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_SRV; break;
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE          : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_UAV; break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_SRV; break;
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_UAV; break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_CBV; break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_CBV; break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_UAV; break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_UAV; break;

      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      case VK_DESCRIPTOR_TYPE_RANGE_SIZE:
      case VK_DESCRIPTOR_TYPE_MAX_ENUM:
        break;
    }
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseUAVCounterBindings(SpvReflectShaderModule* p_module)
{
  char name[MAX_NODE_NAME_LENGTH];
  const char* k_count_tag = "@count";

  for (uint32_t descriptor_index = 0; descriptor_index < p_module->descriptor_binding_count; ++descriptor_index) {
    SpvReflectDescriptorBinding* p_descriptor = &(p_module->descriptor_bindings[descriptor_index]);

    if (p_descriptor->descriptor_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
      continue;
    }

    SpvReflectDescriptorBinding* p_counter_descriptor = NULL;
    // Use UAV counter buffer id if present...
    if (p_descriptor->uav_counter_id != UINT32_MAX) {
      for (uint32_t counter_descriptor_index = 0; counter_descriptor_index < p_module->descriptor_binding_count; ++counter_descriptor_index) {
        SpvReflectDescriptorBinding* p_test_counter_descriptor = &(p_module->descriptor_bindings[counter_descriptor_index]);
        if (p_test_counter_descriptor->descriptor_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
          continue;
        }
        if (p_descriptor->uav_counter_id == p_test_counter_descriptor->spirv_id) {
          p_counter_descriptor = p_test_counter_descriptor;
          break;
        }
      }
    }
    // ...otherwise use old @count convention.
    else {
      const size_t descriptor_name_length = strlen(p_descriptor->name);

      memset(name, 0, MAX_NODE_NAME_LENGTH);    
      memcpy(name, p_descriptor->name, descriptor_name_length);
#if defined(WIN32)
      strcat_s(name, MAX_NODE_NAME_LENGTH, k_count_tag);
#else
      strcat(name, k_count_tag);
#endif

      for (uint32_t counter_descriptor_index = 0; counter_descriptor_index < p_module->descriptor_binding_count; ++counter_descriptor_index) {
        SpvReflectDescriptorBinding* p_test_counter_descriptor = &(p_module->descriptor_bindings[counter_descriptor_index]);
        if (p_test_counter_descriptor->descriptor_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
          continue;
        }
        if (strcmp(name, p_test_counter_descriptor->name) == 0) {
          p_counter_descriptor = p_test_counter_descriptor;
          break;
        }
      }
    }

    if (p_counter_descriptor != NULL) {
      p_descriptor->uav_counter_binding = p_counter_descriptor;
    }
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseDescriptorBlockVariable(Parser* p_parser, SpvReflectShaderModule* p_module, SpvReflectTypeDescription* p_type, SpvReflectBlockVariable* p_block)
{
  bool has_non_writable = false;

  if (IsNotNull(p_type->members) && (p_type->member_count > 0)) {
    p_block->member_count = p_type->member_count;
    p_block->members = (SpvReflectBlockVariable*)calloc(p_block->member_count, sizeof(*p_block->members));
    if (IsNull(p_block->members)) {
      return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
    }

    Node* p_type_node = FindNode(p_parser, p_type->id);
    if (IsNull(p_type_node)) {
      return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
    }

    // Parse members
    for (uint32_t member_index = 0; member_index < p_type->member_count; ++member_index) {
      SpvReflectTypeDescription* p_member_type = &p_type->members[member_index];
      SpvReflectBlockVariable* p_member_var = &p_block->members[member_index];
      if (p_member_type->op == SpvOpTypeStruct) {
        SpvReflectResult result = ParseDescriptorBlockVariable(p_parser, p_module, p_member_type, p_member_var);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
          return result;
        }
      }

      p_member_var->name = p_type_node->member_names[member_index];
      p_member_var->offset = p_type_node->member_decorations[member_index].offset.value;
      p_member_var->decoration_flags = ApplyDecorations(&p_type_node->member_decorations[member_index]);
      if (!has_non_writable && (p_member_var->decoration_flags & SPV_REFLECT_DECORATION_NON_WRITABLE)) {
        has_non_writable = true;
      }
      ApplyNumericTraits(p_member_type, &p_member_var->numeric);
      if (p_member_type->op == SpvOpTypeArray) {
        ApplyArrayTraits(p_member_type, &p_member_var->array);
      }

      p_member_var->type_description = p_member_type;
    }
  }

  p_block->name = p_type->type_name;
  p_block->type_description = p_type;
  if (has_non_writable) {
    p_block->decoration_flags |= SPV_REFLECT_DECORATION_NON_WRITABLE;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseDescriptorBlockVariableSizes(Parser* p_parser, SpvReflectShaderModule* p_module, SpvReflectBlockVariable* p_block)
{
  if (p_block->member_count == 0) {
    return SPV_REFLECT_RESULT_SUCCESS;;
  }

  // Size
  for (uint32_t member_index = 0; member_index < p_block->member_count; ++member_index) {
    SpvReflectBlockVariable* p_member_var = &p_block->members[member_index];
    SpvReflectTypeDescription* p_member_type = p_member_var->type_description;

    switch (p_member_type->op) {
      case SpvOpTypeBool: {
        p_member_var->size = SPIRV_WORD_SIZE;
      }
      break;

      case SpvOpTypeInt:
      case SpvOpTypeFloat: {
        p_member_var->size = p_member_type->traits.numeric.scalar.width / SPIRV_BYTE_WIDTH;
      }
      break;

      case SpvOpTypeVector: {
        uint32_t size = p_member_type->traits.numeric.vector.component_count *
                        (p_member_type->traits.numeric.scalar.width / SPIRV_BYTE_WIDTH);
        p_member_var->size = size;
      }
      break;

      case SpvOpTypeMatrix: {
        if (p_member_var->decoration_flags & SPV_REFLECT_DECORATION_COLUMN_MAJOR) {
          p_member_var->size = p_member_var->numeric.matrix.column_count * p_member_var->numeric.matrix.stride;
        }
        else if (p_member_var->decoration_flags & SPV_REFLECT_DECORATION_ROW_MAJOR) {
          p_member_var->size = p_member_var->numeric.matrix.row_count * p_member_var->numeric.matrix.stride;
        }
      }
      break;

      case SpvOpTypeArray: {
        uint32_t element_count = (p_member_var->array.dims_count > 0 ? 1 : 0);
        for (uint32_t i = 0; i < p_member_var->array.dims_count; ++i) {
          element_count *= p_member_var->array.dims[i];
        }
        p_member_var->size = element_count * p_member_var->array.stride;
      }
      break;

      case SpvOpTypeStruct: {
        SpvReflectResult result = ParseDescriptorBlockVariableSizes(p_parser, p_module, p_member_var);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
          return result;
        }
      }
      break;

      default:
        break;
    }
  }

  // Absolute offsets
  for (uint32_t member_index = 0; member_index < p_block->member_count; ++member_index) {
    SpvReflectBlockVariable* p_member_var = &p_block->members[member_index];
    p_member_var->absolute_offset = p_member_var->offset + p_block->offset;
  }

  // Parse padded size using offset difference for all member except for the last entry...
  for (uint32_t member_index = 0; member_index < (p_block->member_count - 1); ++member_index) {
    SpvReflectBlockVariable* p_member_var = &p_block->members[member_index];
    SpvReflectBlockVariable* p_next_member_var = &p_block->members[member_index + 1];
    p_member_var->padded_size = p_next_member_var->offset - p_member_var->offset;
  }
  // ...last entry just gets rounded up to near multiple of SPIRV_DATA_ALIGNMENT, which is 16 and
  // subtract the offset.
  if (p_block->member_count > 0) {
    SpvReflectBlockVariable* p_member_var = &p_block->members[p_block->member_count - 1];
    p_member_var->padded_size = RoundUp(p_member_var->offset  + p_member_var->size, SPIRV_DATA_ALIGNMENT) - p_member_var->offset;
  }

  // @TODO validate this with assertion
  p_block->size = p_block->members[p_block->member_count - 1].offset +
                  p_block->members[p_block->member_count - 1].padded_size;
  p_block->padded_size = p_block->size;

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseDescriptorBlocks(Parser* p_parser, SpvReflectShaderModule* p_module)
{
  if (p_module->descriptor_binding_count == 0) {
    return SPV_REFLECT_RESULT_SUCCESS;
  }

  for (uint32_t descriptor_index = 0; descriptor_index < p_module->descriptor_binding_count; ++descriptor_index) {
    SpvReflectDescriptorBinding* p_descriptor = &(p_module->descriptor_bindings[descriptor_index]);
    SpvReflectTypeDescription* p_type = p_descriptor->type_description;
    if ((p_descriptor->descriptor_type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) &&
        (p_descriptor->descriptor_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) )
    {
      continue;
    }

    SpvReflectResult result = ParseDescriptorBlockVariable(p_parser, p_module, p_type, &p_descriptor->block);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
      return result;
    }
    result = ParseDescriptorBlockVariableSizes(p_parser, p_module, &p_descriptor->block);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
      return result;
    }
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseFormat(
  const SpvReflectTypeDescription*  p_type,
  VkFormat*                         p_format
)
{
  SpvReflectResult result = SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR;
  bool signedness = p_type->traits.numeric.scalar.signedness;
  if (p_type->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
    uint32_t component_count = p_type->traits.numeric.vector.component_count;
    if (p_type->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
      switch (component_count) {
        case 2: *p_format = VK_FORMAT_R32G32_SFLOAT; break;
        case 3: *p_format = VK_FORMAT_R32G32B32_SFLOAT; break;
        case 4: *p_format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
      }
      result = SPV_REFLECT_RESULT_SUCCESS;
    }
    else if (p_type->type_flags & (SPV_REFLECT_TYPE_FLAG_INT | SPV_REFLECT_TYPE_FLAG_BOOL)) {
      switch (component_count) {
        case 2: *p_format = signedness ? VK_FORMAT_R32G32_SINT : VK_FORMAT_R32G32_UINT; break;
        case 3: *p_format = signedness ? VK_FORMAT_R32G32B32_SINT : VK_FORMAT_R32G32B32_UINT; break;
        case 4: *p_format = signedness ? VK_FORMAT_R32G32B32A32_SINT : VK_FORMAT_R32G32B32A32_UINT; break;
      }
      result = SPV_REFLECT_RESULT_SUCCESS;
    }
  }
  else if (p_type->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
    *p_format = VK_FORMAT_R32_SFLOAT;
    result = SPV_REFLECT_RESULT_SUCCESS;
  }
  else if (p_type->type_flags & (SPV_REFLECT_TYPE_FLAG_INT | SPV_REFLECT_TYPE_FLAG_BOOL)) {
    if (signedness) {
      *p_format = VK_FORMAT_R32_SINT;
      result = SPV_REFLECT_RESULT_SUCCESS;
    }
    else {
      *p_format = VK_FORMAT_R32_UINT;
      result = SPV_REFLECT_RESULT_SUCCESS;
    }
  }
  else if (p_type->type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT) {
    *p_format = VK_FORMAT_UNDEFINED;
    result = SPV_REFLECT_RESULT_SUCCESS;
  }
  return result;
}

static SpvReflectResult ParseInterfaceVariable(Parser*                      p_parser,
                                               const Decorations*           p_type_node_decorations,
                                               SpvReflectShaderModule*      p_module,
                                               SpvReflectTypeDescription*   p_type,
                                               SpvReflectInterfaceVariable* p_var,
                                               bool*                        p_has_built_in)
{
  Node* p_type_node = FindNode(p_parser, p_type->id);
  if (IsNull(p_type_node)) {
    return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
  }

  if (p_type->member_count > 0) {
    p_var->member_count = p_type->member_count;
    p_var->members = (SpvReflectInterfaceVariable*)calloc(p_var->member_count, sizeof(*p_var->members));
    if (IsNull(p_var->members)) {
      return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
    }

    for (uint32_t member_index = 0; member_index < p_type_node->member_count; ++member_index) {
      Decorations* p_member_decorations = &p_type_node->member_decorations[member_index];
      SpvReflectTypeDescription* p_member_type = &p_type->members[member_index];
      SpvReflectInterfaceVariable* p_member_var = &p_var->members[member_index];
      SpvReflectResult result = ParseInterfaceVariable(p_parser, p_member_decorations, p_module, p_member_type, p_member_var, p_has_built_in);
      if (result != SPV_REFLECT_RESULT_SUCCESS) {
        return result;
      }
    }
  }

  p_var->name = p_type_node->name;
  p_var->decoration_flags = ApplyDecorations(p_type_node_decorations);
  p_var->built_in = p_type_node_decorations->built_in;
  ApplyNumericTraits(p_type, &p_var->numeric);
  if (p_type->op == SpvOpTypeArray) {
    ApplyArrayTraits(p_type, &p_var->array);
  }

  p_var->type_description = p_type;

  *p_has_built_in |= p_type_node_decorations->is_built_in;

  SpvReflectResult result = ParseFormat(p_var->type_description, &p_var->format);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    return result;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParseInterfaceVariables(Parser* p_parser, SpvReflectShaderModule* p_module)
{
  if (p_parser->interface_variable_count == 0) {
    return SPV_REFLECT_RESULT_SUCCESS;
  }

  p_module->input_variable_count = 0;
  p_module->output_variable_count = 0;
  for (size_t i = 0; i < p_parser->interface_variable_count; ++i) {
    uint32_t var_result_id = *(p_parser->io_vars + i);
    Node* p_node = FindNode(p_parser, var_result_id);
    if (IsNull(p_node)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }

    SpvReflectTypeDescription* p_type = FindType(p_module, p_node->type_id);
    (void)p_type;
    if (IsNull(p_node)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }

    if (p_node->storage_class == SpvStorageClassInput) {
      p_module->input_variable_count += 1;
    }
    else if (p_node->storage_class == SpvStorageClassOutput) {
      p_module->output_variable_count += 1;
    }
  }

  if (p_module->input_variable_count > 0) {
    p_module->input_variables = (SpvReflectInterfaceVariable*)calloc(p_module->input_variable_count, sizeof(*(p_module->input_variables)));
    if (IsNull(p_module->input_variables)) {
      return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
    }
  }


  if (p_module->output_variable_count > 0) {
    p_module->output_variables = (SpvReflectInterfaceVariable*)calloc(p_module->output_variable_count, sizeof(*(p_module->output_variables)));
    if (IsNull(p_module->output_variables)) {
      return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
    }
  }

  size_t input_index = 0;
  size_t output_index = 0;
  for (size_t i = 0; i < p_parser->interface_variable_count; ++i) {
    uint32_t var_result_id = *(p_parser->io_vars + i);
    Node* p_node = FindNode(p_parser, var_result_id);
    if (IsNull(p_node)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }

    SpvReflectTypeDescription* p_type = FindType(p_module, p_node->type_id);
    if (IsNull(p_node)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }
    // If the type is a pointer, resolve it
    if (p_type->op == SpvOpTypePointer) {
      // Find the type's node
      Node* p_type_node = FindNode(p_parser, p_type->id);
      if (IsNull(p_type_node)) {
        return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
      }
      // Should be the resolved type
      p_type = FindType(p_module, p_type_node->type_id);
      if (IsNull(p_type)) {
        return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
      }
    }

    Node* p_type_node = FindNode(p_parser, p_type->id);
    if (IsNull(p_type_node)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }

    SpvReflectInterfaceVariable* p_var = NULL;
    if (p_node->storage_class == SpvStorageClassInput) {
     p_var = &(p_module->input_variables[input_index]);
     p_var->storage_class = SpvStorageClassInput;
      ++input_index;
    }
    else if (p_node->storage_class == SpvStorageClassOutput) {
      p_var = &(p_module->output_variables[output_index]);
      p_var->storage_class = SpvStorageClassOutput;
      ++output_index;
    } else {
      // interface variables can only have input or output storage classes;
      // anything else is either a new addition or an error.
      assert(false && "Unsupported storage class for interface variable");
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS;
    }

    bool has_built_in = p_node->decorations.is_built_in;
    SpvReflectResult result = ParseInterfaceVariable(p_parser,
                                                     &p_type_node->decorations,
                                                     p_module,
                                                     p_type,
                                                     p_var,
                                                     &has_built_in);   
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
      return result;
    }

    // SPIR-V result id
    p_var->spirv_id = p_node->result_id;
    // Name
    p_var->name = p_node->name;
    // Semantic
    p_var->semantic = p_node->decorations.semantic.value;

    // Decorate with built-in if any member is built-in
    if (has_built_in) {
      p_var->decoration_flags |= SPV_REFLECT_DECORATION_BUILT_IN;
    }

    // Location is decorated on OpVariable node, not the type node.
    p_var->location = p_node->decorations.location.value;
    p_var->word_offset.location = p_node->decorations.location.word_offset;

    // Built in
    if (p_node->decorations.is_built_in) {
      p_var->built_in = p_node->decorations.built_in;
    }
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult ParsePushConstantBlocks(Parser* p_parser, SpvReflectShaderModule* p_module)
{
  for (size_t i = 0; i < p_parser->node_count; ++i) {
    Node* p_node = &(p_parser->nodes[i]);
    if ((p_node->op != SpvOpVariable) || (p_node->storage_class != SpvStorageClassPushConstant)) {
      continue;
    }

    p_module->push_constant_block_count += 1;
  }

  if (p_module->push_constant_block_count == 0) {
    return SPV_REFLECT_RESULT_SUCCESS;
  }

  p_module->push_constant_blocks = (SpvReflectBlockVariable*)calloc(p_module->push_constant_block_count, sizeof(*p_module->push_constant_blocks));
  if (IsNull(p_module->push_constant_blocks)) {
    return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
  }

  uint32_t push_constant_index = 0;
  for (size_t i = 0; i < p_parser->node_count; ++i) {
    Node* p_node = &(p_parser->nodes[i]);
    if ((p_node->op != SpvOpVariable) || (p_node->storage_class != SpvStorageClassPushConstant)) {
      continue;
    }

    SpvReflectTypeDescription* p_type = FindType(p_module, p_node->type_id);
    if (IsNull(p_node)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }
    // If the type is a pointer, resolve it
    if (p_type->op == SpvOpTypePointer) {
      // Find the type's node
      Node* p_type_node = FindNode(p_parser, p_type->id);
      if (IsNull(p_type_node)) {
        return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
      }
      // Should be the resolved type
      p_type = FindType(p_module, p_type_node->type_id);
      if (IsNull(p_type)) {
        return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
      }
    }

    Node* p_type_node = FindNode(p_parser, p_type->id);
    if (IsNull(p_type_node)) {
      return SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE;
    }

    SpvReflectBlockVariable* p_push_constant = &p_module->push_constant_blocks[push_constant_index];
    SpvReflectResult result = ParseDescriptorBlockVariable(p_parser, p_module, p_type, p_push_constant);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
      return result;
    }
    result = ParseDescriptorBlockVariableSizes(p_parser, p_module, p_push_constant);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
      return result;
    }

    ++push_constant_index;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static int SortCompareDescriptorSet(const void* a, const void* b)
{
  const SpvReflectDescriptorSet* p_elem_a = (const SpvReflectDescriptorSet*)a;
  const SpvReflectDescriptorSet* p_elem_b = (const SpvReflectDescriptorSet*)b;
  int value = (int)(p_elem_a->set) - (int)(p_elem_b->set);
  return value;
}

static SpvReflectResult ParseDescriptorSets(SpvReflectShaderModule* p_module)
{
  // Count the descriptors in each set
  for (uint32_t i = 0; i < p_module->descriptor_binding_count; ++i) {
    SpvReflectDescriptorBinding* p_descriptor = &(p_module->descriptor_bindings[i]);

    // Look for a target set using the descriptor's set number
    SpvReflectDescriptorSet* p_target_set = NULL;
    for (uint32_t j = 0; j < SPV_REFLECT_MAX_DESCRIPTOR_SETS; ++j) {
      SpvReflectDescriptorSet* p_set = &p_module->descriptor_sets[j];
      if (p_set->set == p_descriptor->set) {
        p_target_set = p_set;
        break;
      }
    }

    // If a target set isn't found, find the first available one.
    if (IsNull(p_target_set)) {
      for (uint32_t j = 0; j < SPV_REFLECT_MAX_DESCRIPTOR_SETS; ++j) {
        SpvReflectDescriptorSet* p_set = &p_module->descriptor_sets[j];
        if (p_set->set == (uint32_t)INVALID_VALUE) {
          p_target_set = p_set;
          p_target_set->set = p_descriptor->set;
          break;
        }
      }
    }

    if (IsNull(p_target_set)) {
      return SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR;
    }

    p_target_set->binding_count += 1;
  }

  // Count the descriptor sets
  for (uint32_t i = 0; i < SPV_REFLECT_MAX_DESCRIPTOR_SETS; ++i) {
    const SpvReflectDescriptorSet* p_set = &p_module->descriptor_sets[i];
    if (p_set->set != (uint32_t)INVALID_VALUE) {
      p_module->descriptor_set_count += 1;
    }
  }

  // Sort the descriptor sets based on numbers
  if (p_module->descriptor_set_count > 0) {
    qsort(p_module->descriptor_sets,
          p_module->descriptor_set_count,
          sizeof(*(p_module->descriptor_sets)),
          SortCompareDescriptorSet);
  }

  // Build descriptor pointer array
  for (uint32_t i = 0; i <p_module->descriptor_set_count; ++i) {
    SpvReflectDescriptorSet* p_set = &(p_module->descriptor_sets[i]);
    p_set->bindings = (SpvReflectDescriptorBinding **)calloc(p_set->binding_count, sizeof(*(p_set->bindings)));

    uint32_t descriptor_index = 0;
    for (uint32_t j = 0; j < p_module->descriptor_binding_count; ++j) {
      SpvReflectDescriptorBinding* p_descriptor = &(p_module->descriptor_bindings[j]);
      if (p_descriptor->set == p_set->set) {
        assert(descriptor_index < p_set->binding_count);
        p_set->bindings[descriptor_index] = p_descriptor;
        ++descriptor_index;
      }
    }
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult DisambiguateStorageBufferSrvUav(SpvReflectShaderModule* p_module)
{
  if (p_module->descriptor_binding_count == 0) {
    return SPV_REFLECT_RESULT_SUCCESS;
  }

  for (uint32_t descriptor_index = 0; descriptor_index < p_module->descriptor_binding_count; ++descriptor_index) {
    SpvReflectDescriptorBinding* p_descriptor = &(p_module->descriptor_bindings[descriptor_index]);
    // Skip everything that isn't a VK_DESCRIPTOR_TYPE_STORAGE_BUFFER descriptor
    if (p_descriptor->descriptor_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
      continue;
    }

    //
    // Vulkan doesn't disambiguate between SRVs and UAVs so they
    // come back as VK_DESCRIPTOR_TYPE_STORAGE_BUFFER. The block
    // parsing process will mark a block as non-writable should
    // any member of the block or its descendants are non-writable.
    //
    if (p_descriptor->block.decoration_flags & SPV_REFLECT_DECORATION_NON_WRITABLE) {
      p_descriptor->resource_type = SPV_REFLECT_RESOURCE_FLAG_SRV;
    }
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

static SpvReflectResult SynchronizeDescriptorSets(SpvReflectShaderModule* p_module)
{
  // Free and reset all descriptor set numbers
  for (uint32_t i = 0; i < SPV_REFLECT_MAX_DESCRIPTOR_SETS; ++i) {
    SpvReflectDescriptorSet* p_set = &p_module->descriptor_sets[i];
    SafeFree(p_set->bindings);
    p_set->binding_count = 0;
    p_set->set = (uint32_t)INVALID_VALUE;
  }
  // Set descriptor set count to zero
  p_module->descriptor_set_count = 0;

  SpvReflectResult result = ParseDescriptorSets(p_module);
  return result;
}

SpvReflectResult spvReflectGetShaderModule(
  size_t                   size,
  const void*              p_code,
  SpvReflectShaderModule*  p_module
)
{
  return spvReflectCreateShaderModule(size, p_code, p_module);
}

SpvReflectResult spvReflectCreateShaderModule(
  size_t                   size,
  const void*              p_code,
  SpvReflectShaderModule*  p_module
)
{
  // Initialize all module fields to zero
  memset(p_module, 0, sizeof(*p_module));

  // Allocate module internals
#ifdef __cplusplus
  p_module->_internal = (SpvReflectShaderModule::Internal*)calloc(1, sizeof(*(p_module->_internal)));
#else
  p_module->_internal = calloc(1, sizeof(*(p_module->_internal)));
#endif
  if (IsNull(p_module->_internal)) {
    return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
  }
  // Allocate SPIR-V code storage
  p_module->_internal->spirv_size = size;
  p_module->_internal->spirv_code = (uint32_t*)calloc(1, p_module->_internal->spirv_size);
  p_module->_internal->spirv_word_count = (uint32_t)(size / SPIRV_WORD_SIZE);
  if (IsNull(p_module->_internal->spirv_code)) {
    SafeFree(p_module->_internal);
    return SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED;
  }
  memcpy(p_module->_internal->spirv_code, p_code, size);

  Parser parser = { 0 };
  SpvReflectResult result = CreateParser(p_module->_internal->spirv_size,
                                         p_module->_internal->spirv_code,
                                         &parser);
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseNodes(&parser);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseMemberCounts(&parser);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseNames(&parser);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseDecorations(&parser);
  }

  // Start of reflection data parsing
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    p_module->entry_point_name = parser.entry_point_name;
    p_module->entry_point_id = parser.entry_point_id;
    p_module->spirv_execution_model = parser.spirv_execution_model;
    switch (p_module->spirv_execution_model) {
      default: break;
      case SpvExecutionModelVertex                 : p_module->vulkan_shader_stage = VK_SHADER_STAGE_VERTEX_BIT; break;
      case SpvExecutionModelTessellationControl    : p_module->vulkan_shader_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
      case SpvExecutionModelTessellationEvaluation : p_module->vulkan_shader_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
      case SpvExecutionModelGeometry               : p_module->vulkan_shader_stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
      case SpvExecutionModelFragment               : p_module->vulkan_shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
      case SpvExecutionModelGLCompute              : p_module->vulkan_shader_stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
    }

    p_module->source_language = parser.source_language;
    p_module->source_language_version = parser.source_language_version;

    // Zero out descriptor set data
    p_module->descriptor_set_count = 0;
    memset(p_module->descriptor_sets, 0, SPV_REFLECT_MAX_DESCRIPTOR_SETS * sizeof(*p_module->descriptor_sets));
    // Initialize descriptor set numbers
    for (uint32_t set_number = 0; set_number < SPV_REFLECT_MAX_DESCRIPTOR_SETS; ++set_number) {
      p_module->descriptor_sets[set_number].set = (uint32_t)INVALID_VALUE;
    }
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseTypes(&parser, p_module);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseDescriptorBindings(&parser, p_module);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseDescriptorType(p_module);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseUAVCounterBindings(p_module);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseDescriptorBlocks(&parser, p_module);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParseInterfaceVariables(&parser, p_module);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = ParsePushConstantBlocks(&parser, p_module);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = DisambiguateStorageBufferSrvUav(p_module);
  }
  if (result == SPV_REFLECT_RESULT_SUCCESS) {
    result = SynchronizeDescriptorSets(p_module);
  }

  // Destroy module if parse was not successful
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    spvReflectDestroyShaderModule(p_module);
  }

  DestroyParser(&parser);

  return result;
}

static void SafeFreeTypes(SpvReflectTypeDescription* p_type)
{
  if (IsNull(p_type)) {
    return;
  }

  if (IsNotNull(p_type->members)) {
    for (size_t i = 0; i < p_type->member_count; ++i) {
      SpvReflectTypeDescription* p_member = &p_type->members[i];
      SafeFreeTypes(p_member);
    }

    SafeFree(p_type->members);
    p_type->members = NULL;
  }
}

static void SafeFreeBlockVariables(SpvReflectBlockVariable* p_block)
{
  if (IsNull(p_block)) {
    return;
  }

  if (IsNotNull(p_block->members)) {
    for (size_t i = 0; i < p_block->member_count; ++i) {
      SpvReflectBlockVariable* p_member = &p_block->members[i];
      SafeFreeBlockVariables(p_member);
    }

    SafeFree(p_block->members);
    p_block->members = NULL;
  }
}

void spvReflectDestroyShaderModule(SpvReflectShaderModule* p_module)
{
  if (IsNull(p_module->_internal)) {
    return;
  }

  // Descriptor set bindings
  for (size_t i = 0; i < p_module->descriptor_set_count; ++i) {
    SpvReflectDescriptorSet* p_set = &p_module->descriptor_sets[i];
    free(p_set->bindings);
  }

  // Descriptor binding blocks
  for (size_t i = 0; i < p_module->descriptor_binding_count; ++i) {
    SpvReflectDescriptorBinding* p_descriptor = &p_module->descriptor_bindings[i];
    SafeFreeBlockVariables(&p_descriptor->block);
  }
  SafeFree(p_module->descriptor_bindings);

  // Input variables
  SafeFree(p_module->input_variables);

  // Output variables
  SafeFree(p_module->output_variables);

  // Push constants
  SafeFree(p_module->push_constant_blocks);

  // Type infos
  for (size_t i = 0; i < p_module->_internal->type_description_count; ++i) {
    SpvReflectTypeDescription* p_type = &p_module->_internal->type_descriptions[i];
    if (IsNotNull(p_type->members)) {
      SafeFreeTypes(p_type);
    }
    SafeFree(p_type->members);
  }
  SafeFree(p_module->_internal->type_descriptions);

  // Free SPIR-V code
  SafeFree(p_module->_internal->spirv_code);
  // Free internal
  SafeFree(p_module->_internal);
}

uint32_t spvReflectGetCodeSize(const SpvReflectShaderModule* p_module)
{
  if (IsNull(p_module)) {
    return 0;
  }

  return (uint32_t)(p_module->_internal->spirv_size);
}

const uint32_t* spvReflectGetCode(const SpvReflectShaderModule* p_module)
{
  if (IsNull(p_module)) {
    return NULL;
  }

  return p_module->_internal->spirv_code;
}

SpvReflectResult spvReflectEnumerateDescriptorBindings(
  const SpvReflectShaderModule*  p_module,
  uint32_t*                      p_count,
  SpvReflectDescriptorBinding**  pp_bindings
)
{
  if (IsNull(p_module)) {
    return SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
  }

  if (IsNotNull(pp_bindings)) {
    if (*p_count != p_module->descriptor_binding_count) {
      return SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH;
    }

    for (uint32_t index = 0; index < *p_count; ++index) {
      SpvReflectDescriptorBinding* p_bindings = (SpvReflectDescriptorBinding*)&p_module->descriptor_bindings[index];
      pp_bindings[index] = p_bindings;
    }
  }
  else {
    *p_count = p_module->descriptor_binding_count;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

SpvReflectResult spvReflectEnumerateDescriptorSets(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectDescriptorSet**     pp_sets
)
{
  if (IsNull(p_module)) {
    return SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
  }

  if (IsNotNull(pp_sets)) {
    if (*p_count != p_module->descriptor_set_count) {
      return SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH;
    }

    for (uint32_t index = 0; index < *p_count; ++index) {
      SpvReflectDescriptorSet* p_set = (SpvReflectDescriptorSet*)&p_module->descriptor_sets[index];
      pp_sets[index] = p_set;
    }
  }
  else {
    *p_count = p_module->descriptor_set_count;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

SpvReflectResult spvReflectEnumerateInputVariables(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectInterfaceVariable** pp_variables
)
{
  if (IsNull(p_module)) {
    return SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
  }

  if (IsNotNull(pp_variables)) {
    if (*p_count != p_module->input_variable_count) {
      return SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH;
    }

    for (uint32_t index = 0; index < *p_count; ++index) {
      SpvReflectInterfaceVariable* p_var = (SpvReflectInterfaceVariable*)&p_module->input_variables[index];
      pp_variables[index] = p_var;
    }
  }
  else {
    *p_count = p_module->input_variable_count;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

SpvReflectResult spvReflectEnumerateOutputVariables(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectInterfaceVariable** pp_variables
)
{
  if (IsNull(p_module)) {
    return SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
  }

  if (IsNotNull(pp_variables)) {
    if (*p_count != p_module->output_variable_count) {
      return SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH;
    }

    for (uint32_t index = 0; index < *p_count; ++index) {
      SpvReflectInterfaceVariable* p_var = (SpvReflectInterfaceVariable*)&p_module->output_variables[index];
      pp_variables[index] = p_var;
    }
  }
  else {
    *p_count = p_module->output_variable_count;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}

SpvReflectResult spvReflectEnumeratePushConstantBlocks(
  const SpvReflectShaderModule* p_module,
  uint32_t*                     p_count,
  SpvReflectBlockVariable**     pp_blocks
)
{
  if (IsNull(p_module)) {
    return SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
  }

  if (pp_blocks != NULL) {
    if (*p_count != p_module->push_constant_block_count) {
      return SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH;
    }

    for (uint32_t index = 0; index < *p_count; ++index) {
      SpvReflectBlockVariable* p_push_constant_blocks = (SpvReflectBlockVariable*)&p_module->push_constant_blocks[index];
      pp_blocks[index] = p_push_constant_blocks;
    }
  }
  else {
    *p_count = p_module->push_constant_block_count;
  }

  return SPV_REFLECT_RESULT_SUCCESS;
}
SpvReflectResult spvReflectEnumeratePushConstants(const SpvReflectShaderModule* p_module,
                                                  uint32_t*                     p_count,
                                                  SpvReflectBlockVariable**     pp_blocks
)
{
  return spvReflectEnumeratePushConstantBlocks(p_module, p_count, pp_blocks);
}

const SpvReflectDescriptorBinding* spvReflectGetDescriptorBinding(const SpvReflectShaderModule* p_module,
                                                                  uint32_t                      binding_number,
                                                                  uint32_t                      set_number,
                                                                  SpvReflectResult*             p_result)
{
  const SpvReflectDescriptorBinding* p_descriptor = NULL;
  assert(IsNotNull(p_module));
  if (IsNotNull(p_module)) {
    for (uint32_t index = 0; index < p_module->descriptor_binding_count; ++index) {
      const SpvReflectDescriptorBinding* p_potential = &p_module->descriptor_bindings[index];
      if ((p_potential->binding == binding_number) && (p_potential->set == set_number)) {
        p_descriptor = p_potential;
        break;
      }
    }
  }
  if (IsNotNull(p_result)) {
    *p_result = IsNotNull(p_descriptor)
        ?  SPV_REFLECT_RESULT_SUCCESS
        : (IsNull(p_module) ? SPV_REFLECT_RESULT_ERROR_NULL_POINTER
                            : SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  }
  return p_descriptor;
}

const SpvReflectDescriptorSet* spvReflectGetDescriptorSet(const SpvReflectShaderModule* p_module,
                                                          uint32_t                      set_number,
                                                          SpvReflectResult*             p_result)
{
  const SpvReflectDescriptorSet* p_set = NULL;
  assert(IsNotNull(p_module));
  if (IsNotNull(p_module)) {
    for (uint32_t index = 0; index < p_module->descriptor_set_count; ++index) {
      const SpvReflectDescriptorSet* p_potential = &p_module->descriptor_sets[index];
      if (p_potential->set == set_number) {
        p_set = p_potential;
      }
    }
  }
  if (IsNotNull(p_result)) {
    *p_result = IsNotNull(p_set)
        ?  SPV_REFLECT_RESULT_SUCCESS
        : (IsNull(p_module) ? SPV_REFLECT_RESULT_ERROR_NULL_POINTER
                            : SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  }
  return p_set;
}

const SpvReflectInterfaceVariable* spvReflectGetInputVariableByLocation(const SpvReflectShaderModule* p_module,
                                                                        uint32_t                      location,
                                                                        SpvReflectResult*             p_result)
{
  if (location == INVALID_VALUE) {
    *p_result = SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND;
    return NULL;
  }
  const SpvReflectInterfaceVariable* p_var = NULL;
  assert(IsNotNull(p_module));
  if (IsNotNull(p_module)) {
    for (uint32_t index = 0; index < p_module->input_variable_count; ++index) {
      const SpvReflectInterfaceVariable* p_potential = &p_module->input_variables[index];
      if (p_potential->location == location) {
        p_var = p_potential;
      }
    }
  }
  if (IsNotNull(p_result)) {
    *p_result = IsNotNull(p_var)
        ?  SPV_REFLECT_RESULT_SUCCESS
        : (IsNull(p_module) ? SPV_REFLECT_RESULT_ERROR_NULL_POINTER
                            : SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  }
  return p_var;
}
const SpvReflectInterfaceVariable* spvReflectGetInputVariable(const SpvReflectShaderModule* p_module,
                                                              uint32_t                      location,
                                                              SpvReflectResult*             p_result)
{
  return spvReflectGetInputVariableByLocation(p_module, location, p_result);
}

const SpvReflectInterfaceVariable* spvReflectGetInputVariableBySemantic(const SpvReflectShaderModule* p_module,
                                                                        const char*                   semantic,
                                                                        SpvReflectResult*             p_result)
{
  if (IsNull(semantic)) {
    *p_result = SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
    return NULL;
  }
  if (semantic[0] == '\0') {
    *p_result = SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND;
    return NULL;
  }
  const SpvReflectInterfaceVariable* p_var = NULL;
  assert(IsNotNull(p_module));
  if (IsNotNull(p_module)) {
    for (uint32_t index = 0; index < p_module->input_variable_count; ++index) {
      const SpvReflectInterfaceVariable* p_potential = &p_module->input_variables[index];
      if (p_potential->semantic != NULL && strcmp(p_potential->semantic, semantic) == 0) {
        p_var = p_potential;
      }
    }
  }
  if (IsNotNull(p_result)) {
    *p_result = IsNotNull(p_var)
      ?  SPV_REFLECT_RESULT_SUCCESS
      : (IsNull(p_module) ? SPV_REFLECT_RESULT_ERROR_NULL_POINTER
        : SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  }
  return p_var;
}

const SpvReflectInterfaceVariable* spvReflectGetOutputVariableByLocation(const SpvReflectShaderModule*  p_module,
                                                                         uint32_t                       location,
                                                                         SpvReflectResult*              p_result)
{
  if (location == INVALID_VALUE) {
    *p_result = SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND;
    return NULL;
  }
  const SpvReflectInterfaceVariable* p_var = NULL;
  assert(IsNotNull(p_module));
  if (IsNotNull(p_module)) {
    for (uint32_t index = 0; index < p_module->output_variable_count; ++index) {
      const SpvReflectInterfaceVariable* p_potential = &p_module->output_variables[index];
      if (p_potential->location == location) {
        p_var = p_potential;
      }
    }
  }
  if (IsNotNull(p_result)) {
    *p_result = IsNotNull(p_var)
        ?  SPV_REFLECT_RESULT_SUCCESS
        : (IsNull(p_module) ? SPV_REFLECT_RESULT_ERROR_NULL_POINTER
                            : SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  }
  return p_var;
}
const SpvReflectInterfaceVariable* spvReflectGetOutputVariable(const SpvReflectShaderModule*  p_module,
                                                               uint32_t                       location,
                                                               SpvReflectResult*              p_result)
{
  return spvReflectGetOutputVariableByLocation(p_module, location, p_result);
}

const SpvReflectInterfaceVariable* spvReflectGetOutputVariableBySemantic(const SpvReflectShaderModule*  p_module,
                                                                         const char*                    semantic,
                                                                         SpvReflectResult*              p_result)
{
  if (IsNull(semantic)) {
    *p_result = SPV_REFLECT_RESULT_ERROR_NULL_POINTER;
    return NULL;
  }
  if (semantic[0] == '\0') {
    *p_result = SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND;
    return NULL;
  }
  const SpvReflectInterfaceVariable* p_var = NULL;
  assert(IsNotNull(p_module));
  if (IsNotNull(p_module)) {
    for (uint32_t index = 0; index < p_module->output_variable_count; ++index) {
      const SpvReflectInterfaceVariable* p_potential = &p_module->output_variables[index];
      if (p_potential->semantic != NULL && strcmp(p_potential->semantic, semantic) == 0) {
        p_var = p_potential;
      }
    }
  }
  if (IsNotNull(p_result)) {
    *p_result = IsNotNull(p_var)
        ?  SPV_REFLECT_RESULT_SUCCESS
        : (IsNull(p_module) ? SPV_REFLECT_RESULT_ERROR_NULL_POINTER
                            : SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  }
  return p_var;
}
const SpvReflectBlockVariable* spvReflectGetPushConstantBlock(const SpvReflectShaderModule* p_module,
                                                              uint32_t                      index,
                                                              SpvReflectResult*             p_result)
{
  const SpvReflectBlockVariable* p_push_constant = NULL;
  assert(IsNotNull(p_module));
  if (IsNotNull(p_module)) {
    if (index < p_module->push_constant_block_count) {
      p_push_constant = &p_module->push_constant_blocks[index];
    }
  }
  if (IsNotNull(p_result)) {
    *p_result = IsNotNull(p_push_constant)
        ?  SPV_REFLECT_RESULT_SUCCESS
        : (IsNull(p_module) ? SPV_REFLECT_RESULT_ERROR_NULL_POINTER
                            : SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  }
  return p_push_constant;
}
const SpvReflectBlockVariable* spvReflectGetPushConstant(const SpvReflectShaderModule* p_module,
                                                         uint32_t                      index,
                                                         SpvReflectResult*             p_result)
{
  return spvReflectGetPushConstantBlock(p_module, index, p_result);
}

SpvReflectResult spvReflectChangeDescriptorBindingNumbers(SpvReflectShaderModule*            p_module,
                                                          const SpvReflectDescriptorBinding* p_binding,
                                                          uint32_t                           new_binding_number,
                                                          uint32_t                           new_set_binding)
{
  SpvReflectDescriptorBinding* p_target_descriptor = NULL;
  for (uint32_t index = 0; index < p_module->descriptor_binding_count; ++index) {
    if(&p_module->descriptor_bindings[index] == p_binding) {
      p_target_descriptor = &p_module->descriptor_bindings[index];
      break;
    }
  }

  if (IsNotNull(p_target_descriptor)) {
    if (p_target_descriptor->word_offset.binding > (p_module->_internal->spirv_word_count - 1)) {
      return SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED;
    }
    // Binding number
    if (new_binding_number != SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE) {
      uint32_t* p_code = p_module->_internal->spirv_code + p_target_descriptor->word_offset.binding;
      *p_code = new_binding_number;
      p_target_descriptor->binding = new_binding_number;
    }
    // Set number
    if (new_set_binding != SPV_REFLECT_SET_NUMBER_DONT_CHANGE) {
      uint32_t* p_code = p_module->_internal->spirv_code + p_target_descriptor->word_offset.set;
      *p_code = new_set_binding;
      p_target_descriptor->set = new_set_binding;
    }
  }

  SpvReflectResult result = SPV_REFLECT_RESULT_SUCCESS;
  if (new_set_binding != SPV_REFLECT_SET_NUMBER_DONT_CHANGE) {
    result = SynchronizeDescriptorSets(p_module);
  }
  return result;
}
SpvReflectResult spvReflectChangeDescriptorBindingNumber(SpvReflectShaderModule*            p_module,
  const SpvReflectDescriptorBinding* p_descriptor_binding,
  uint32_t                           new_binding_number,
  uint32_t                           optional_new_set_number)
{
  return spvReflectChangeDescriptorBindingNumbers(p_module,p_descriptor_binding, new_binding_number, optional_new_set_number);
}

SpvReflectResult spvReflectChangeDescriptorSetNumber(SpvReflectShaderModule*        p_module,
                                                     const SpvReflectDescriptorSet* p_set,
                                                     uint32_t                       new_set_number)
{
  SpvReflectDescriptorSet* p_target_set = NULL;
  for (uint32_t index = 0; index < SPV_REFLECT_MAX_DESCRIPTOR_SETS; ++index) {
    if (&p_module->descriptor_sets[index] == p_set) {
      p_target_set = (SpvReflectDescriptorSet*)p_set;
      break;
    }
  }

  SpvReflectResult result = SPV_REFLECT_RESULT_SUCCESS;
  if (IsNotNull(p_target_set) && new_set_number != SPV_REFLECT_SET_NUMBER_DONT_CHANGE) {
    for (uint32_t index = 0; index < p_target_set->binding_count; ++index) {
      SpvReflectDescriptorBinding* p_descriptor = p_target_set->bindings[index];
      if (p_descriptor->word_offset.set > (p_module->_internal->spirv_word_count - 1)) {
        return SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED;
      }

      uint32_t* p_code = p_module->_internal->spirv_code + p_descriptor->word_offset.set;
      *p_code = new_set_number;
      p_descriptor->set = new_set_number;
    }

    result = SynchronizeDescriptorSets(p_module);
  }

  return result;
}

static SpvReflectResult ChangeVariableLocation(SpvReflectShaderModule*      p_module,
                                               SpvReflectInterfaceVariable* p_variable,
                                               uint32_t                     new_location)
{
  if (p_variable->word_offset.location > (p_module->_internal->spirv_word_count - 1)) {
    return SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED;
  }
  uint32_t* p_code = p_module->_internal->spirv_code + p_variable->word_offset.location;
  *p_code = new_location;
  p_variable->location = new_location;
  return SPV_REFLECT_RESULT_SUCCESS;
}

SpvReflectResult spvReflectChangeInputVariableLocation(SpvReflectShaderModule*            p_module,
                                                       const SpvReflectInterfaceVariable* p_input_variable,
                                                       uint32_t                           new_location)
{
  for (uint32_t index = 0; index < p_module->input_variable_count; ++index) {
    if(&p_module->input_variables[index] == p_input_variable) {
      return ChangeVariableLocation(p_module, &p_module->input_variables[index], new_location);
    }
  }
  return SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND;
}

SpvReflectResult spvReflectChangeOutputVariableLocation(SpvReflectShaderModule*             p_module,
                                                        const SpvReflectInterfaceVariable*  p_output_variable,
                                                        uint32_t                            new_location)
{
  for (uint32_t index = 0; index < p_module->output_variable_count; ++index) {
    if(&p_module->output_variables[index] == p_output_variable) {
      return ChangeVariableLocation(p_module, &p_module->output_variables[index], new_location);
    }
  }
  return SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND;
}

const char* spvReflectSourceLanguage(SpvSourceLanguage source_lang)
{
  switch (source_lang) {
    case SpvSourceLanguageUnknown    : return "Unknown";
    case SpvSourceLanguageESSL       : return "ESSL";
    case SpvSourceLanguageGLSL       : return "GLSL";
    case SpvSourceLanguageOpenCL_C   : return "OpenCL_C";
    case SpvSourceLanguageOpenCL_CPP : return "OpenCL_CPP";
    case SpvSourceLanguageHLSL       : return "HLSL";

    case SpvSourceLanguageMax:
      break;
  }
  return "";
}
