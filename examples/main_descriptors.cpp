#include "common.h"
#include "sample_spv.h"
#include <cassert>

int main(int argn, char** argv)
{
  SpvReflectShaderModule module = {};
  SpvReflectResult result = spvReflectCreateShaderModule(sizeof(k_sample_spv), k_sample_spv, &module);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  uint32_t count = 0;
  result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  std::vector<SpvReflectDescriptorSet*> sets(count);
  result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  const char* t  = "  ";
  const char* tt = "    ";

  PrintModuleInfo(std::cout, module);
  std::cout << "\n\n";

  std::cout << "Descriptor sets:" << "\n";
  for (size_t index = 0; index < sets.size(); ++index) {
    auto p_set = sets[index];

    // descriptor sets can also be retrieved directly from the module, by set index
    auto p_set2 = spvReflectGetDescriptorSet(&module, p_set->set, &result);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    assert(p_set == p_set2);
    (void)p_set2;

    std::cout << t << index << ":" << "\n";
    PrintDescriptorSet(std::cout, *p_set, tt);
    std::cout << "\n\n";
  }

  spvReflectDestroyShaderModule(&module);
  
  return 0;
}