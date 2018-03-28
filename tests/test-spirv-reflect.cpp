#include "../examples/common.h"
#include "spirv_reflect.h"

#include "gtest/gtest.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#if defined(_MSC_VER)
#include <direct.h>
#define posix_chdir(d) _chdir(d)
#else
#include <unistd.h>
#define posix_chdir(d) chdir(d)
#endif

TEST(SpirvReflectTestCase, SourceLanguage) {
  EXPECT_STREQ(spvReflectSourceLanguage(SpvSourceLanguageESSL), "ESSL");
  EXPECT_STREQ(spvReflectSourceLanguage(SpvSourceLanguageGLSL), "GLSL");
  EXPECT_STREQ(spvReflectSourceLanguage(SpvSourceLanguageOpenCL_C), "OpenCL_C");
  EXPECT_STREQ(spvReflectSourceLanguage(SpvSourceLanguageOpenCL_CPP),
               "OpenCL_CPP");
  EXPECT_STREQ(spvReflectSourceLanguage(SpvSourceLanguageHLSL), "HLSL");

  EXPECT_STREQ(spvReflectSourceLanguage(SpvSourceLanguageUnknown), "Unknown");
  EXPECT_STREQ(spvReflectSourceLanguage(SpvSourceLanguageMax), "");
  EXPECT_STREQ(spvReflectSourceLanguage(
                   static_cast<SpvSourceLanguage>(SpvSourceLanguageMax - 1)),
               "");
}

class SpirvReflectTest : public ::testing::TestWithParam<const char *> {
public:
  // optional: initialize static data to be shared by all tests in this test
  // case. Note that for parameterized tests, the specific parameter value is a
  // non-static member data
  static void SetUpTestCase() {
    FILE *f = fopen("tests/glsl/built_in_format.spv", "r");
    if (!f) {
      posix_chdir("..");
      f = fopen("tests/glsl/built_in_format.spv", "r");
    }
    EXPECT_NE(f, nullptr) << "Couldn't find test shaders!";
    if (f) {
      fclose(f);
    }
  }
  static void TearDownTestCase() {}

protected:
  SpirvReflectTest() {
    // set-up work for each test
  }

  ~SpirvReflectTest() override {
    // clean-up work that doesn't throw exceptions
  }

  // optional: called after constructor & before destructor, respectively.
  // Used if you have initialization steps that can throw exceptions or must
  // otherwise be deferred.
  void SetUp() override {
    // called after constructor before each test
    spirv_path_ = GetParam();
    std::ifstream spirv_file(spirv_path_, std::ios::binary | std::ios::ate);
    std::streampos spirv_file_nbytes = spirv_file.tellg();
    spirv_file.seekg(0);
    spirv_.resize(spirv_file_nbytes);
    spirv_file.read(reinterpret_cast<char *>(spirv_.data()), spirv_.size());

    SpvReflectResult result =
        spvReflectCreateShaderModule(spirv_.size(), spirv_.data(), &module_);
    ASSERT_EQ(SPV_REFLECT_RESULT_SUCCESS, result)
        << "spvReflectCreateShaderModule() failed";
  }

  // optional:
  void TearDown() override {
    // called before destructor after all tests
    spvReflectDestroyShaderModule(&module_);
  }

  // members here will be accessible in all tests in this test case
  std::string spirv_path_;
  std::vector<uint8_t> spirv_;
  SpvReflectShaderModule module_;

  // static members will be accessible to all tests in this test case
  static std::string test_shaders_dir;
};

TEST_P(SpirvReflectTest, GetCodeSize) {
  EXPECT_EQ(spvReflectGetCodeSize(&module_), spirv_.size());
}

TEST_P(SpirvReflectTest, GetCode) {
  int code_compare =
      memcmp(spvReflectGetCode(&module_), spirv_.data(), spirv_.size());
  EXPECT_EQ(code_compare, 0);
}

TEST_P(SpirvReflectTest, GetDescriptorBinding) {
  uint32_t binding_count = 0;
  SpvReflectResult result;
  result =
      spvReflectEnumerateDescriptorBindings(&module_, &binding_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  std::vector<SpvReflectDescriptorBinding *> bindings(binding_count);
  result = spvReflectEnumerateDescriptorBindings(&module_, &binding_count,
                                                 bindings.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  for (const auto *db : bindings) {
    const SpvReflectDescriptorBinding *also_db =
        spvReflectGetDescriptorBinding(&module_, db->binding, db->set, &result);
    EXPECT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
    EXPECT_EQ(db, also_db);
  }
}

TEST_P(SpirvReflectTest, GetDescriptorSet) {
  uint32_t set_count = 0;
  SpvReflectResult result;
  result = spvReflectEnumerateDescriptorSets(&module_, &set_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  std::vector<SpvReflectDescriptorSet *> sets(set_count);
  result = spvReflectEnumerateDescriptorSets(&module_, &set_count, sets.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  for (const auto *ds : sets) {
    const SpvReflectDescriptorSet *also_ds =
        spvReflectGetDescriptorSet(&module_, ds->set, &result);
    EXPECT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
    EXPECT_EQ(ds, also_ds);
  }
}

TEST_P(SpirvReflectTest, GetInputVariableByLocation) {
  uint32_t iv_count = 0;
  SpvReflectResult result;
  result = spvReflectEnumerateInputVariables(&module_, &iv_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  std::vector<SpvReflectInterfaceVariable *> ivars(iv_count);
  result = spvReflectEnumerateInputVariables(&module_, &iv_count, ivars.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  for (const auto *iv : ivars) {
    const SpvReflectInterfaceVariable *also_iv =
        spvReflectGetInputVariableByLocation(&module_, iv->location, &result);
    if (iv->location == UINT32_MAX) {
      // Not all elements have valid locations.
      EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
      EXPECT_EQ(also_iv, nullptr);
    } else {
      EXPECT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
      EXPECT_EQ(iv, also_iv);
    }
  }
}

TEST_P(SpirvReflectTest, GetInputVariableBySemantic) {
  uint32_t iv_count = 0;
  SpvReflectResult result;
  result = spvReflectEnumerateInputVariables(&module_, &iv_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  std::vector<SpvReflectInterfaceVariable *> ivars(iv_count);
  result = spvReflectEnumerateInputVariables(&module_, &iv_count, ivars.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  for (const auto *iv : ivars) {
    const SpvReflectInterfaceVariable *also_iv =
        spvReflectGetInputVariableBySemantic(&module_, iv->semantic, &result);
    if (iv->semantic == nullptr) {
      // Not all elements have valid semantics
      EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_NULL_POINTER);
      EXPECT_EQ(also_iv, nullptr);
    } else if (iv->semantic[0] == '\0') {
      // Not all elements have valid semantics
      EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
      EXPECT_EQ(also_iv, nullptr);
    } else {
      EXPECT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
      EXPECT_EQ(iv, also_iv);
    }
  }
}

TEST_P(SpirvReflectTest, GetOutputVariableByLocation) {
  uint32_t ov_count = 0;
  SpvReflectResult result;
  result = spvReflectEnumerateOutputVariables(&module_, &ov_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  std::vector<SpvReflectInterfaceVariable *> ovars(ov_count);
  result =
      spvReflectEnumerateOutputVariables(&module_, &ov_count, ovars.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  for (const auto *ov : ovars) {
    const SpvReflectInterfaceVariable *also_ov =
        spvReflectGetOutputVariableByLocation(&module_, ov->location, &result);
    if (ov->location == UINT32_MAX) {
      // Not all elements have valid locations.
      EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
      EXPECT_EQ(also_ov, nullptr);
    } else {
      EXPECT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
      EXPECT_EQ(ov, also_ov);
    }
  }
}

TEST_P(SpirvReflectTest, GetOutputVariableBySemantic) {
  uint32_t ov_count = 0;
  SpvReflectResult result;
  result = spvReflectEnumerateOutputVariables(&module_, &ov_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  std::vector<SpvReflectInterfaceVariable *> ovars(ov_count);
  result =
      spvReflectEnumerateOutputVariables(&module_, &ov_count, ovars.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  for (const auto *ov : ovars) {
    const SpvReflectInterfaceVariable *also_ov =
        spvReflectGetOutputVariableBySemantic(&module_, ov->semantic, &result);
    if (ov->semantic == nullptr) {
      // Not all elements have valid semantics
      EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_NULL_POINTER);
      EXPECT_EQ(also_ov, nullptr);
    } else if (ov->semantic[0] == '\0') {
      // Not all elements have valid semantics
      EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
      EXPECT_EQ(also_ov, nullptr);
    } else {
      EXPECT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
      EXPECT_EQ(ov, also_ov);
    }
  }
}

TEST_P(SpirvReflectTest, GetPushConstantBlock) {
  uint32_t block_count = 0;
  SpvReflectResult result;
  result =
      spvReflectEnumeratePushConstantBlocks(&module_, &block_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  std::vector<SpvReflectBlockVariable *> blocks(block_count);
  result = spvReflectEnumeratePushConstantBlocks(&module_, &block_count,
                                                 blocks.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  for (uint32_t i = 0; i < block_count; ++i) {
    const SpvReflectBlockVariable *b = blocks[i];
    const SpvReflectBlockVariable *also_b =
        spvReflectGetPushConstantBlock(&module_, i, &result);
    EXPECT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
    EXPECT_EQ(b, also_b);
  }
}

TEST_P(SpirvReflectTest, ChangeDescriptorBindingNumber) {
  uint32_t binding_count = 0;
  SpvReflectResult result;
  result =
      spvReflectEnumerateDescriptorBindings(&module_, &binding_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  if (binding_count == 0) {
    return; // can't change binding numbers if there are no bindings!
  }
  std::vector<SpvReflectDescriptorBinding *> bindings(binding_count);
  result = spvReflectEnumerateDescriptorBindings(&module_, &binding_count,
                                                 bindings.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  uint32_t set_count = 0;
  result = spvReflectEnumerateDescriptorSets(&module_, &set_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_GT(set_count, 0U);
  std::vector<SpvReflectDescriptorSet *> sets(set_count);
  result = spvReflectEnumerateDescriptorSets(&module_, &set_count, sets.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  SpvReflectDescriptorBinding *b = bindings[0];
  const uint32_t new_binding_number = 1000;
  const uint32_t set_number = b->set;
  // Make sure no binding exists at the binding number we're about to change to.
  ASSERT_EQ(spvReflectGetDescriptorBinding(&module_, new_binding_number,
                                           set_number, &result),
            nullptr);
  EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  // Modify the binding number (leaving the set number unchanged)
  result = spvReflectChangeDescriptorBindingNumbers(
      &module_, b, new_binding_number, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  // We should now be able to retrieve the binding at the new number
  const SpvReflectDescriptorBinding *new_binding =
      spvReflectGetDescriptorBinding(&module_, new_binding_number, set_number,
                                     &result);
  ASSERT_NE(new_binding, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(new_binding->binding, new_binding_number);
  EXPECT_EQ(new_binding->set, set_number);
  // The set count & sets contents should not have changed, since we didn't
  // change the set number.
  result = spvReflectEnumerateDescriptorSets(&module_, &set_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(set_count, sets.size());
  std::vector<SpvReflectDescriptorSet *> new_sets(set_count);
  result =
      spvReflectEnumerateDescriptorSets(&module_, &set_count, new_sets.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(sets, new_sets);

  // TODO: confirm that the modified SPIR-V code is still valid, either by
  // running spirv-val or calling vkCreateShaderModule().
}

TEST_P(SpirvReflectTest, ChangeDescriptorSetNumber) {
  uint32_t binding_count = 0;
  SpvReflectResult result;
  result =
      spvReflectEnumerateDescriptorBindings(&module_, &binding_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  if (binding_count == 0) {
    return; // can't change set numbers if there are no bindings!
  }
  std::vector<SpvReflectDescriptorBinding *> bindings(binding_count);
  result = spvReflectEnumerateDescriptorBindings(&module_, &binding_count,
                                                 bindings.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  uint32_t set_count = 0;
  result = spvReflectEnumerateDescriptorSets(&module_, &set_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_GT(set_count, 0U);
  std::vector<SpvReflectDescriptorSet *> sets(set_count);
  result = spvReflectEnumerateDescriptorSets(&module_, &set_count, sets.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  SpvReflectDescriptorSet *s = sets[0];

  const uint32_t new_set_number = 13;
  const uint32_t set_binding_count = s->binding_count;
  // Make sure no set exists at the binding number we're about to change to.
  ASSERT_EQ(spvReflectGetDescriptorSet(&module_, new_set_number, &result),
            nullptr);
  EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  // Modify the set number
  result = spvReflectChangeDescriptorSetNumber(&module_, s, new_set_number);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  // We should now be able to retrieve the set at the new number
  const SpvReflectDescriptorSet *new_set =
      spvReflectGetDescriptorSet(&module_, new_set_number, &result);
  ASSERT_NE(new_set, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(new_set->set, new_set_number);
  EXPECT_EQ(new_set->binding_count, set_binding_count);
  // The set count should not have changed, since we didn't
  // change the set number.
  result = spvReflectEnumerateDescriptorSets(&module_, &set_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(set_count, sets.size());

  // TODO: confirm that the modified SPIR-V code is still valid, either by
  // running spirv-val or calling vkCreateShaderModule().
}

TEST_P(SpirvReflectTest, ChangeInputVariableLocation) {
  uint32_t iv_count = 0;
  SpvReflectResult result;
  result = spvReflectEnumerateInputVariables(&module_, &iv_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  if (iv_count == 0) {
    return; // can't change variable locations if there are no variables!
  }
  std::vector<SpvReflectInterfaceVariable *> ivars(iv_count);
  result = spvReflectEnumerateInputVariables(&module_, &iv_count, ivars.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  SpvReflectInterfaceVariable *iv = ivars[0];

  const uint32_t new_location = 37;
  // Make sure no var exists at the location we're about to change to.
  ASSERT_EQ(
      spvReflectGetInputVariableByLocation(&module_, new_location, &result),
      nullptr);
  EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  // Modify the location
  result = spvReflectChangeInputVariableLocation(&module_, iv, new_location);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  // We should now be able to retrieve the variable at its new location
  const SpvReflectInterfaceVariable *new_iv =
      spvReflectGetInputVariableByLocation(&module_, new_location, &result);
  ASSERT_NE(new_iv, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(new_iv->location, new_location);
  // The variable count should not have changed
  result = spvReflectEnumerateInputVariables(&module_, &iv_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(iv_count, ivars.size());

  // TODO: confirm that the modified SPIR-V code is still valid, either by
  // running spirv-val or calling vkCreateShaderModule().
}

TEST_P(SpirvReflectTest, ChangeOutputVariableLocation) {
  uint32_t ov_count = 0;
  SpvReflectResult result;
  result = spvReflectEnumerateOutputVariables(&module_, &ov_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  if (ov_count == 0) {
    return; // can't change variable locations if there are no variables!
  }
  std::vector<SpvReflectInterfaceVariable *> ovars(ov_count);
  result =
      spvReflectEnumerateOutputVariables(&module_, &ov_count, ovars.data());
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);

  SpvReflectInterfaceVariable *ov = ovars[0];

  const uint32_t new_location = 37;
  // Make sure no var exists at the location we're about to change to.
  ASSERT_EQ(
      spvReflectGetOutputVariableByLocation(&module_, new_location, &result),
      nullptr);
  EXPECT_EQ(result, SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND);
  // Modify the location
  result = spvReflectChangeOutputVariableLocation(&module_, ov, new_location);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  // We should now be able to retrieve the variable at its new location
  const SpvReflectInterfaceVariable *new_ov =
      spvReflectGetOutputVariableByLocation(&module_, new_location, &result);
  ASSERT_NE(new_ov, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(new_ov->location, new_location);
  // The variable count should not have changed
  result = spvReflectEnumerateOutputVariables(&module_, &ov_count, nullptr);
  ASSERT_EQ(result, SPV_REFLECT_RESULT_SUCCESS);
  EXPECT_EQ(ov_count, ovars.size());

  // TODO: confirm that the modified SPIR-V code is still valid, either by
  // running spirv-val or calling vkCreateShaderModule().
}

TEST_P(SpirvReflectTest, CheckYamlOutput) {
  const uint32_t yaml_verbosity = 1;
  SpvReflectToYaml yamlizer(module_, yaml_verbosity);
  std::stringstream test_yaml;
  test_yaml << yamlizer;
  std::string test_yaml_str = test_yaml.str();

  std::string golden_yaml_path = spirv_path_ + ".yaml";
  std::ifstream golden_yaml_file(golden_yaml_path);
  ASSERT_TRUE(golden_yaml_file.good());
  std::stringstream golden_yaml;
  golden_yaml << golden_yaml_file.rdbuf();
  golden_yaml_file.close();
  std::string golden_yaml_str = golden_yaml.str();
  ASSERT_EQ(test_yaml_str.size(), golden_yaml_str.size());

  // TODO: I wish there were a better way to show what changed, but the
  // differences (if any) tend to be pretty large, even for small changes.
  bool yaml_contents_match = (test_yaml_str == golden_yaml_str);
  EXPECT_TRUE(yaml_contents_match)
      << "YAML output mismatch; try regenerating the golden YAML with "
         "\"tests/build_golden_yaml.py\" and see what changed.";
}

namespace {
const std::vector<const char *> all_spirv_paths = {
    // clang-format off
    "../tests/glsl/built_in_format.spv",
    "../tests/glsl/input_attachment.spv",
    "../tests/glsl/texel_buffer.spv",
    "../tests/hlsl/append_consume.spv",
    "../tests/hlsl/binding_array.spv",
    "../tests/hlsl/binding_types.spv",
    "../tests/hlsl/cbuffer.spv",
    "../tests/hlsl/counter_buffers.spv",
    "../tests/hlsl/semantics.spv",
    // clang-format on
};
} // namespace
INSTANTIATE_TEST_CASE_P(ForAllShaders, SpirvReflectTest,
                        ::testing::ValuesIn(all_spirv_paths));
