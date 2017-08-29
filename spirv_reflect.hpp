#ifndef SPIRV_REFLECT_HPP
#define SPIRV_REFLECT_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <spirv.hpp>
#include <vulkan.h>

namespace spirv_ref {

enum Result {
  SUCCESS,
  NOT_READY,
  ERROR_FAILED_PARSE,
  ERROR_INVALID_SPIRV_SIZE,
  ERROR_UNEXPECTED_EOF,
  ERROR_INVALID_ID_REFERENCE,
  ERROR_UNEXPECTED_OP,
  ERROR_DUPLICATE_TYPE,
  ERROR_DUPLICATE_BINDING,
};

enum TypeFlag {
  TYPE_FLAG_NONE                    = 0x00000000,
  // Types that are void or have machine width
  TYPE_FLAG_COMPONENT_VOID          = 0x00000001,
  TYPE_FLAG_COMPONENT_BOOL          = 0x00000002,
  TYPE_FLAG_COMPONENT_INT           = 0x00000004,
  TYPE_FLAG_COMPONENT_FLOAT         = 0x00000008,
  TYPE_FLAG_COMPONENT               = 0x0000000F,
  // Types that are external resources such as buffer, image, or sampler.
  TYPE_FLAG_EXTERNAL_SAMPLER        = 0x00000010,
  TYPE_FLAG_EXTERNAL_IMAGE          = 0x00000020,
  TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE  = 0x00000030,
  TYPE_FLAG_EXTERNAL_BUFFER         = 0x00000080,
  TYPE_FLAG_EXTERNAL                = 0x000000F0,
  // Types that are composed of components or elements
  TYPE_FLAG_COMPOSITE_ARRAY         = 0x00010000,
  TYPE_FLAG_COMPOSITE_RUNTIME_ARRAY = 0x00020000,
  TYPE_FLAG_COMPOSITE_VECTOR        = 0x00040000,
  TYPE_FLAG_COMPOSITE_MATRIX        = 0x00080000,
  TYPE_FLAG_COMPOSITE               = 0x000F0000,
  // Types that are structures, pointer
  TYPE_FLAG_STRUCTURE               = 0x01000000,
  TYPE_FLAG_POINTER                 = 0x10000000
};

enum TypeAttr {
  TYPE_ATTR_NONE                    = 0x00000000,
  TYPE_ATTR_BLOCK                   = 0x00000001,
  TYPE_ATTR_BUFFER_BLOCK            = 0x00000002,
  TYPE_ATTR_ROW_MAJOR               = 0x00000004,
  TYPE_ATTR_COL_MAJOR               = 0x00000008,
  TYPE_ATTR_BUILT_IN                = 0x00000010,
  TYPE_ATTR_NO_PERSPECTIVE          = 0x00000020,
  TYPE_ATTR_FLAT                    = 0x00000040,
};

class BlockVariable;
class Block;
class Descriptor;
class DescriptorSet;
class PushConstant;
class IoVariable;
class ShaderReflection;

namespace detail {

class Parser;

template <typename T>
T InvalidValue() 
{
  return static_cast<T>(UINT32_MAX);
}

//! \class Type
//!
//! Vulkan descriptor type mappings:
//!   VK_DESCRIPTOR_TYPE_SAMPLER:
//!     m_type_flags = TYPE_FLAG_EXTERNAL_SAMPLER
//!
//!   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
//!     m_type_flags = TYPE_FLAG_EXTERNAL_IMAGE | TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE
//!
//!   VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
//!     m_type_flags = TYPE_FLAG_EXTERNAL_IMAGE
//!     m_image_traits.dim = ! spv::DimBuffer
//!     m_image_traits.image_format = spv::ImageFormatUnknown
//!
//!   VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
//!     m_type_flags = TYPE_FLAG_EXTERNAL_IMAGE
//!     m_image_traits.dim = ! spv::DimBuffer
//!     m_image_traits.image_format = ! spv::ImageFormatUnknown
//!
//!   VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
//!     m_type_flags = TYPE_FLAG_EXTERNAL_IMAGE
//!     m_image_traits.dim = spv::DimBuffer
//!     m_image_traits.image_format = spv::ImageFormatUnknown
//!
//!   VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
//!     m_type_flags = TYPE_FLAG_EXTERNAL_IMAGE
//!     m_image_traits.dim = spv::DimBuffer
//!     m_image_traits.image_format = ! spv::ImageFormatUnknown
//!
//!   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
//!     m_type_flags = TYPE_FLAG_EXTERNAL_BUFFER
//!     m_type_attrs = TYPE_ATTR_BLOCK
//!
//!   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
//!     m_type_flags = TYPE_FLAG_EXTERNAL_BUFFER
//!     m_type_attrs = TYPE_ATTR_BUFFER_BLOCK
//!
class Type {
public:
  struct ScalarTraits {
    uint32_t          width;
    uint32_t          signedness;
  };

  struct VectorTraits {
    uint32_t          component_count;
  };

  struct MatrixTraits {
    uint32_t          column_count;
    uint32_t          row_count;
    uint32_t          stride;
  };

  struct ImageTraits {
    spv::Dim          dim;
    uint32_t          depth;
    uint32_t          arrayed;
    uint32_t          ms;
    uint32_t          sampled;
    spv::ImageFormat  image_format;
  };

  Type();
  ~Type();

  uint32_t                        GetTypeFlags() const { return m_type_flags; }
  uint32_t                        GetTypeAttrs() const { return m_type_attrs; }
  const std::string&              GetTypeName() const { return m_type_name; }
  const std::string&              GetArrayDimString() const { return m_array_dim_string; }
  const std::string&              GetBlockMemberName() const { return m_block_member_name; }

  VkDescriptorType                GetVkDescriptorType() const { return m_vk_descriptor_type; }

  std::string                     GetInfo(const std::string& indent = "") const;

private:
  uint32_t                        m_id = 0;
  spv::Op                         m_op = spv::OpNop;
  ShaderReflection*               m_module = nullptr;
  uint32_t                        m_type_flags = TYPE_FLAG_NONE;
  uint32_t                        m_type_attrs = TYPE_ATTR_NONE;
  spv::StorageClass               m_storage_class = detail::InvalidValue<spv::StorageClass>();
  ScalarTraits                    m_scalar_traits = {};
  union {
    VectorTraits                  m_vector_traits;
    MatrixTraits                  m_matrix_traits;
    ImageTraits                   m_image_traits = {};
  };
  std::vector<uint32_t>           m_array;
  mutable std::string             m_type_name;
  mutable std::string             m_array_dim_string;
  std::string                     m_block_member_name;
  std::vector<Type>               m_members;
  VkDescriptorType                m_vk_descriptor_type = detail::InvalidValue<VkDescriptorType>();

private:
  Type(ShaderReflection* p_module, uint32_t id, spv::Op op);
  void ParseVkDesciptorType();
  void ParseTypeStrings();
  void WriteBlockContent(std::ostream& os, const std::string& indent = "") const;
  friend class detail::Parser;
  friend class ShaderReflection;
  friend class Descriptor;
};

//! \class Variable
//!
//!
class Variable {
public:
  Variable(ShaderReflection* p_module = nullptr);
  ~Variable();

  uint32_t                        GetOffset() const { return m_offset; }
  const std::string&              GetName() const { return m_name; }

protected:
  ShaderReflection*               m_module = nullptr;
  uint32_t                        m_offset = 0;
  std::string                     m_name;
  const detail::Type*             m_type = nullptr;

private:
  friend class detail::Parser;
};

} // namespace detail

//! \class BlockVariable
//!
//!
class BlockVariable : public detail::Variable {
public:
  BlockVariable();
  ~BlockVariable();

  size_t                          GetMemberCount() const { return m_members.size(); }
  BlockVariable* const            GetMember(size_t index);

protected:
  std::vector<BlockVariable>      m_members;

protected:
  BlockVariable(ShaderReflection* p_module);
  friend class detail::Parser;
};

//! \class Block
//!
//!
class Block : public BlockVariable {
public:
  Block();
  ~Block();

protected:
  Block(ShaderReflection* p_module);
  friend class detail::Parser;
};

//! \class DescriptorBinding
//!
//!
class Descriptor {
public:
  Descriptor();
  ~Descriptor();

  uint32_t                        GetBindingNumber() const;
  void                            SetBindingNumber(uint32_t binding_number);
  uint32_t                        GetSetNumber() const;
  void                            SetSetNumber(uint32_t set_number);
  const std::string&              GetName() const { return m_name; }
  std::string                     GetInfo(const std::string& indent = "") const;

  VkDescriptorType                GetVkDescriptorType() const;

private:
  ShaderReflection*               m_module = nullptr;
  const detail::Type*             m_type = nullptr;
  uint32_t                        m_binding_number = detail::InvalidValue<uint32_t>();
  uint32_t                        m_binding_number_word_offset = 0;
  uint32_t                        m_set_number = detail::InvalidValue<uint32_t>();
  uint32_t                        m_set_number_word_offset = 0;
  Block                           m_block;
  std::string                     m_name;

private:
  Descriptor(ShaderReflection* p_module, uint32_t binding_number_word_offset, uint32_t set_number_word_offset);
  friend class detail::Parser;
};

//! \class DescriptorSet
//!
//!
class DescriptorSet {
public:
  DescriptorSet();
  ~DescriptorSet();

  uint32_t                        GetSetNumber() const;
  void                            SetSetNumber(uint32_t set_number);
  size_t                          GetBindingCount() const { return m_bindings.size(); }  
  Descriptor* const               GetBinding(size_t index);

private:
  ShaderReflection*               m_module = nullptr;
  uint32_t                        m_set_number = detail::InvalidValue<uint32_t>();
  std::vector<Descriptor*>        m_bindings;

private:
  DescriptorSet(ShaderReflection* p_module, uint32_t set_number);  
  friend class detail::Parser;
  friend class ShaderReflection;
};

//! \class PushConstant
//!
//!
class PushConstant : public Block {
public:
  PushConstant();
  ~PushConstant();

private:
  PushConstant(ShaderReflection* p_module);
  friend class detail::Parser;
};

//! \class IoVariable
//!
//!
class IoVariable : public detail::Variable {
public:
  IoVariable();
  ~IoVariable();

  size_t                          GetMemberCount() const { return m_members.size(); }
  IoVariable* const               GetMember(size_t index);

private:
  uint32_t                        m_location_number_word_offset = 0;
  uint32_t                        m_location_number = detail::InvalidValue<uint32_t>();
  std::vector<IoVariable>         m_members;

private:
  IoVariable(ShaderReflection* p_module, uint32_t location_number_word_offset);
  friend class detail::Parser;
};

//! \class Module
//!
//!
class ShaderReflection {
public:
  ShaderReflection();
  ShaderReflection(size_t size, void* p_code);
  ~ShaderReflection();

  size_t                          GetSpirvSize() const { return m_spirv_size; }
  const uint32_t*                 GetSpirvCode() const { return m_spirv_code; }
  uint32_t                        GetSpirvWordCount() const { return m_spirv_word_count; }

  spv::ExecutionModel             GetExecutionModel() const { return m_execution_model; }

  size_t                          GetDescriptorCount() const { return m_descriptors.size(); }
  Descriptor* const               GetDescriptor(size_t index);

  size_t                          GetDescriptorSetCount() const { return m_descriptor_sets.size(); }
  DescriptorSet* const            GetDescriptorSet(size_t index);

  size_t                          GetInputVariableCount() const { return m_input_variables.size(); }
  IoVariable* const               GetInputVariable(size_t index);

  size_t                          GetOutputVariableCount() const { return m_output_variables.size(); }
  IoVariable* const               GetOutputVariable(size_t index);

  VkShaderStageFlagBits           GetVkShaderStage() const;

private:
  size_t                          m_spirv_size = 0;
  uint32_t*                       m_spirv_code = nullptr;
  uint32_t                        m_spirv_word_count = 0;
  spv::ExecutionModel             m_execution_model = detail::InvalidValue<spv::ExecutionModel>();
  std::vector<detail::Type>       m_types;
  std::vector<Descriptor>         m_descriptors;
  std::vector<DescriptorSet*>     m_descriptor_sets;
  std::vector<IoVariable>         m_input_variables;
  std::vector<IoVariable>         m_output_variables;

private:
  Result SyncDescriptorSets();
  detail::Type* const FindType(uint32_t type_id);
  friend class detail::Parser;
  friend class DescriptorSet;
};

// \fn ParseShaderReflection
Result ParseShaderReflection(size_t size, void* p_code, ShaderReflection* p_module);

namespace detail {
 
//! \class Parser
//!
//!
class Parser {
public:
  Parser(size_t size, const uint32_t* p_spirv_code, ShaderReflection* p_module);
  ~Parser();

  Result  GetResult() const { return m_result; }
 
private:
  uint32_t              m_spirv_word_count = 0;
  const uint32_t*       m_spirv_code = nullptr;
  Result                m_result = NOT_READY;
  ShaderReflection*     m_module = nullptr;
  std::vector<uint32_t> m_io_var_ids;

  struct NumberDecoration {
    uint32_t  word_offset;
    uint32_t  value;
  };

  struct TypeDecoration {
    uint32_t  attrs;
    uint32_t  array_stride;
    uint32_t  matrix_stride;
  };

  struct ArrayTraits {
    uint32_t  element_type_id;
    uint32_t  length_id;
  };

  struct ImageTraits {
    uint32_t          sampled_type_id;
    spv::Dim          dim;
    uint32_t          depth;
    uint32_t          arrayed;
    uint32_t          ms;
    uint32_t          sampled;
    spv::ImageFormat  image_format;
  };
 
  struct VariableDecorations {
    TypeDecoration    type       = {};
    NumberDecoration  set        = { 0, detail::InvalidValue<uint32_t>() };
    NumberDecoration  binding    = { 0, detail::InvalidValue<uint32_t>() };
    NumberDecoration  location   = { 0, detail::InvalidValue<uint32_t>() };
    NumberDecoration  offset     = { 0, detail::InvalidValue<uint32_t>() };
  };

  struct Node {
    uint32_t                                result_id       = 0;
    spv::Op                                 op              = detail::InvalidValue<spv::Op>();
    uint32_t                                result_type_id  = 0;
    uint32_t                                type_id         = 0;
    spv::StorageClass                       storage_class   = detail::InvalidValue<spv::StorageClass>();
    uint32_t                                word_offset     = 0;
    uint32_t                                word_count      = 0;
    bool                                    is_type         = false;
    ArrayTraits                             array_traits    = {};
    uint32_t                                image_type_id   = 0;
    ImageTraits                             image_traits    = {};
    VariableDecorations                     decorations;
    std::map<uint32_t, VariableDecorations> member_decorations;
    std::string                             name;
    std::map<uint32_t, std::string>         member_names;
  };

  std::vector<Node>     m_nodes;
  std::vector<uint32_t> m_input_variable_ids;
  std::vector<uint32_t> m_output_variable_ids;

private:
  Node*   FindNode(uint32_t node_id);
  Result  ParseNodes();
  Result  ParseNames();
  Result  ParseDecorations();
  Result  ParseTypes(std::vector<Type>* p_types);
  Result  ParseType(Node* p_node, Type* p_type);
  Result  ParseDescriptors(std::vector<Descriptor>* p_descriptors);
};

} // namespace detail
} // namespace spirv_ref

#endif // SPIRV_REFLECT_HPP