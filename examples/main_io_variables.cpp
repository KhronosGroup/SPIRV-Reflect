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

  std::vector<SpvReflectInterfaceVariable*> input_vars(count);
  result = spvReflectEnumerateInputVariables(&module, &count, input_vars.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  count = 0;
  result = spvReflectEnumerateOutputVariables(&module, &count, NULL);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  std::vector<SpvReflectInterfaceVariable*> output_vars(count);
  result = spvReflectEnumerateOutputVariables(&module, &count, output_vars.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  const char* t  = "  ";
  const char* tt = "    ";

  PrintModuleInfo(std::cout, module);
  std::cout << "\n\n";

  std::cout << "Input variables:" << "\n";
  for (size_t index = 0; index < input_vars.size(); ++index) {
    auto p_var = input_vars[index];

    // input variables can also be retrieved directly from the module, by location
    auto p_var2 = spvReflectGetInputVariable(&module, p_var->location, &result);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    assert(p_var == p_var2);
    (void)p_var2;

    std::cout << t << index << ":" << "\n";
    PrintInterfaceVariable(std::cout, module.source_language, *p_var, tt);
    std::cout << "\n\n";
  }

  std::cout << "Output variables:" << "\n";
  for (size_t index = 0; index < output_vars.size(); ++index) {
    auto p_var = output_vars[index];

    // output variables can also be retrieved directly from the module, by location
    auto p_var2 = spvReflectGetOutputVariable(&module, p_var->location, &result);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    assert(p_var == p_var2);
    (void)p_var2;

    std::cout << t << index << ":" << "\n";
    PrintInterfaceVariable(std::cout, module.source_language, *p_var, tt);
    std::cout << "\n\n";
  }

  spvReflectDestroyShaderModule(&module);
  
  return 0;
}