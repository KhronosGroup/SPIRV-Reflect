#include "common.h"
#include "sample_spv.h"
#include <cassert>

int main(int argn, char** argv)
{

  SpvReflectShaderModule module = {};
  SpvReflectResult result = spvReflectGetShaderModule(k_sample_spv_len, k_sample_spv, &module);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  uint32_t count = 0;
  result = spvReflectEnumerateInputVariables(&module, &count, NULL);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  std::vector<uint32_t> input_locations(count);
  result = spvReflectEnumerateInputVariables(&module, &count, input_locations.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  count = 0;
  result = spvReflectEnumerateOutputVariables(&module, &count, NULL);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  std::vector<uint32_t> output_locations(count);
  result = spvReflectEnumerateOutputVariables(&module, &count, output_locations.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  const char* t  = "  ";
  const char* tt = "    ";

  PrintModuleInfo(std::cout, module);
  std::cout << "\n\n";

  std::cout << "Input variables:" << "\n";
  for (size_t index = 0; index < input_locations.size(); ++index) {
    uint32_t location = input_locations[index];
    auto p_var = spvReflectGetInputVariable(&module, location, &result);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::cout << t << index << ":" << "\n";
    PrintInterfaceVariable(std::cout, module.source_language, *p_var, tt);
    std::cout << "\n\n";
  }

  std::cout << "Output variables:" << "\n";
  for (size_t index = 0; index < output_locations.size(); ++index) {
    uint32_t location = output_locations[index];
    auto p_var = spvReflectGetOutputVariable(&module, location, &result);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::cout << t << index << ":" << "\n";
    PrintInterfaceVariable(std::cout, module.source_language, *p_var, tt);
    std::cout << "\n\n";
  }

  spvReflectDestroyShaderModule(&module);
  
  return 0;
}