#include "common.h"
#include "sample_spv.h"
#include <cassert>

int main(int argn, char** argv)
{

  SpvReflectShaderModule module = {};
  SpvReflectResult result = spvReflectGetShaderModule(k_sample_spv_len, k_sample_spv, &module);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  uint32_t count = 0;
  result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  std::vector<uint32_t> set_numbers(count);
  result = spvReflectEnumerateDescriptorSets(&module, &count, set_numbers.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  const char* t  = "  ";
  const char* tt = "    ";

  PrintModuleInfo(std::cout, module);
  std::cout << "\n\n";

  std::cout << "Descriptor sets:" << "\n";
  for (size_t index = 0; index < set_numbers.size(); ++index) {
    uint32_t set_number = set_numbers[index];
    auto p_set = spvReflectGetDescriptorSet(&module, set_number, &result);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::cout << t << index << ":" << "\n";
    PrintDescriptorSet(std::cout, *p_set, tt);
    std::cout << "\n\n";
  }

  spvReflectDestroyShaderModule(&module);
  
  return 0;
}