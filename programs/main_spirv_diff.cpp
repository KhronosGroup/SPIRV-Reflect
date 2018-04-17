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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>

#include "spirv_reflect.h"
#include "common/arg_parser.h"
#include "common/common.h"
#include "common/to_string.h"

enum DiffOptions{
  DIFF_OPTIONS_IO_VARS      = 0x1,
  DIFF_OPTIONS_DESCRIPTORS  = 0x2,
};

enum DiffResult {
  DIFF_RESULT_NO_DIFFERENCE = 0x0,
  DIFF_RESULT_IO_VARS       = 0x1,
  DIFF_RESULT_DESCRIPTORS   = 0x2,
};

enum {
  DIFF_FAILURE    = -1,
  DIFF_SAME       =  0,
  DIFF_DIFFERENCE =  1
};

bool GetInputVariables(
  const spv_reflect::ShaderModule&            reflection, 
  std::vector<SpvReflectInterfaceVariable*>&  variables
)
{
  uint32_t count = 0;
  SpvReflectResult result = reflection.EnumerateInputVariables(&count, nullptr);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    return false;
  }
  variables.resize(count);
  result = reflection.EnumerateInputVariables(&count, variables.data());
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    return false;
  }
  return true;
}

bool GetOutputVariables(
  const spv_reflect::ShaderModule&            reflection, 
  std::vector<SpvReflectInterfaceVariable*>&  variables
)
{
  uint32_t count = 0;
  SpvReflectResult result = reflection.EnumerateOutputVariables(&count, nullptr);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    return false;
  }
  variables.resize(count);
  result = reflection.EnumerateOutputVariables(&count, variables.data());
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    return false;
  }
  return true;
}

int DiffIoVars(
  SpvStorageClass                             storage_class_a,
  std::vector<SpvReflectInterfaceVariable*>&  variables_a,
  const spv_reflect::ShaderModule&            reflection_a,
  SpvStorageClass                             storage_class_b,
  std::vector<SpvReflectInterfaceVariable*>&  variables_b,
  const spv_reflect::ShaderModule&            reflection_b,
  std::ostream&                               os
)
{
  int result = DIFF_SAME;

  std::set<uint32_t> locations;
  std::set<SpvBuiltIn> built_ins;
  
  std::string storage_class_a_str = ToStringStorageClass(storage_class_a);
  std::string storage_class_b_str = ToStringStorageClass(storage_class_b);

  // A
  for (auto& p_var : variables_a) {      
    if (p_var->location == UINT32_MAX) {
      built_ins.insert(p_var->built_in);
    }
    else {
      locations.insert(p_var->location);
    }
  }

  // B
  for (auto& p_var : variables_b) {
    if (p_var->location == UINT32_MAX) {
      built_ins.insert(p_var->built_in);
    }
    else {
      locations.insert(p_var->location);
    }
  }

  // Locations
  for (auto& location : locations) {
    // A
    const SpvReflectInterfaceVariable* p_var_a = nullptr;
    if (storage_class_a == SpvStorageClassInput) {
      p_var_a = reflection_a.GetInputVariableByLocation(location);
    }
    else if (storage_class_a == SpvStorageClassOutput) {
      p_var_a = reflection_a.GetOutputVariableByLocation(location);
    }
    else {
      return DIFF_FAILURE;
    }
    // B
    const SpvReflectInterfaceVariable* p_var_b = nullptr;
    if (storage_class_b == SpvStorageClassInput) {
      p_var_b = reflection_b.GetInputVariableByLocation(location);
    }
    else if (storage_class_b == SpvStorageClassOutput) {
      p_var_b = reflection_b.GetOutputVariableByLocation(location);
    }
    else {
      return DIFF_FAILURE;
    }

    if ((p_var_a == nullptr) && (p_var_b == nullptr)) {
      return DIFF_FAILURE;
    }

    bool is_in_a = (p_var_a != nullptr);
    bool is_in_b = (p_var_b != nullptr);
    if (is_in_a && is_in_b) {
      if (p_var_a->format != p_var_b->format) {
        result = DIFF_DIFFERENCE;
        os << "\n";
        os << "location=" << location << " has format difference" << "\n";
        os << "< A=" << ToStringFormat(p_var_a->format) << " (" << storage_class_a_str << ")" << "\n";
        os << "> B=" << ToStringFormat(p_var_b->format) << " (" << storage_class_b_str << ")" << "\n";
      }
    }
    else if (is_in_a && !is_in_b) {
      result = DIFF_DIFFERENCE;
      os << "\n";
      os << "location=" << location << " is in A but not in B" << "\n";
      os << "< A=" << "TRUE" << " (" << storage_class_a_str << ")" << "\n";
      os << "> B=" << "false" << " (" << storage_class_b_str << ")" << "\n";
    }
    else if (!is_in_a && is_in_b) {
      result = DIFF_DIFFERENCE;
      os << "\n";
      os << "location=" << location << " is in B but not in A" << "\n";
      os << "< A=" << "false" << " (" << storage_class_a_str << ")" << "\n";
      os << "> B=" << "TRUE" << " (" << storage_class_b_str << ")" << "\n";
    }
    else {
      // Should never get here
      assert(false && "unreachable code path");
      return DIFF_FAILURE;

    }
  }

  // Built-ins
  for (auto& built_in : built_ins) {
    // A
    const SpvReflectInterfaceVariable* p_var_a = nullptr;
    if (storage_class_a == SpvStorageClassInput) {
      p_var_a = reflection_a.GetInputVariableByBuiltIn(built_in);
    }
    else if (storage_class_a == SpvStorageClassOutput) {
      p_var_a = reflection_a.GetOutputVariableByBuiltIn(built_in);
    }
    else {
      return DIFF_FAILURE;
    }
    // B
    const SpvReflectInterfaceVariable* p_var_b = nullptr;
    if (storage_class_b == SpvStorageClassInput) {
      p_var_b = reflection_b.GetInputVariableByBuiltIn(built_in);
    }
    else if (storage_class_b == SpvStorageClassOutput) {
      p_var_b = reflection_b.GetOutputVariableByBuiltIn(built_in);
    }
    else {
      return DIFF_FAILURE;
    }

    if ((p_var_a == nullptr) && (p_var_b == nullptr)) {
      return DIFF_FAILURE;
    }
    
    std::string built_in_str = ToStringBuiltIn(built_in) + " (BuiltIn)";

    bool is_in_a = (p_var_a != nullptr);
    bool is_in_b = (p_var_b != nullptr);
    if (is_in_a && is_in_b) {
      if (p_var_a->format != p_var_b->format) {
        result = DIFF_DIFFERENCE;
        os << "\n";
        os << "location=" << built_in_str << " has format difference" << "\n";
        os << "< A=" << ToStringFormat(p_var_a->format) << " (" << storage_class_a_str << ")" << "\n";
        os << "> B=" << ToStringFormat(p_var_b->format) << " (" << storage_class_b_str << ")" << "\n";
      }
    }
    else if (is_in_a && !is_in_b) {
      result = DIFF_DIFFERENCE;
      os << "\n";
      os << "location=" << built_in_str << " is in A but not in B" << "\n";
      os << "< A=" << "TRUE" << " (" << storage_class_a_str << ")" << "\n";
      os << "> B=" << "false" << " (" << storage_class_b_str << ")" << "\n";
    }
    else if (!is_in_a && is_in_b) {
      result = DIFF_DIFFERENCE;
      os << "\n";
      os << "location=" << built_in_str << " is in B but not in A" << "\n";
      os << "< A=" << "false" << " (" << storage_class_a_str << ")" << "\n";
      os << "> B=" << "TRUE" << " (" << storage_class_b_str << ")" << "\n";
    }
    else {
      // Should never get here
      assert(false && "unreachable code path");
      return DIFF_FAILURE;
    }
  }
 
  return result;
}

int DiffIoVars(
  const spv_reflect::ShaderModule&  reflection_a,
  const spv_reflect::ShaderModule&  reflection_b,
  std::ostream&                     os
)
{
  int result = DIFF_SAME;
  bool is_stage_same = (reflection_a.GetShaderStage() == reflection_b.GetShaderStage());
  if (is_stage_same) {
    std::vector<SpvReflectInterfaceVariable*> variables_a;
    std::vector<SpvReflectInterfaceVariable*> variables_b;

    // Input
    {
      if (!GetInputVariables(reflection_a, variables_a)) {
        return DIFF_FAILURE;
      }

      if (!GetInputVariables(reflection_b, variables_b)) {
        return DIFF_FAILURE;
      }

      if (!variables_a.empty() && !variables_b.empty()) {
        std::stringstream ss;
        int result_vars = DiffIoVars(SpvStorageClassInput,
                                     variables_a,
                                     reflection_a,
                                     SpvStorageClassInput,
                                     variables_b,
                                     reflection_b,
                                     ss);
        if (result_vars == DIFF_FAILURE) {
          return DIFF_FAILURE;
        }
        if (result_vars == DIFF_DIFFERENCE) {
          result =  DIFF_DIFFERENCE;
          os << "\n";
          os << "// -----------------------------------------------------------------------------" << "\n";
          os << "// Input Variables" << "\n";
          os << "// -----------------------------------------------------------------------------" << "\n";
          os << ss.str();
        }
      }
    }

    // Output
    variables_a.clear();
    variables_b.clear();
    {
      if (!GetOutputVariables(reflection_a, variables_a)) {
        return DIFF_FAILURE;
      }

      if (!GetOutputVariables(reflection_b, variables_b)) {
        return DIFF_FAILURE;
      }

      if (!variables_a.empty() && !variables_b.empty()) {
        std::stringstream ss;
        int result_vars = DiffIoVars(SpvStorageClassOutput,
                                     variables_a,
                                     reflection_a,
                                     SpvStorageClassOutput,
                                     variables_b,
                                     reflection_b,
                                     ss);
        if (result_vars == DIFF_FAILURE) {
          return DIFF_FAILURE;
        }
        if (result_vars == DIFF_DIFFERENCE) {
          result =  DIFF_DIFFERENCE;
          os << "\n";
          os << "// -----------------------------------------------------------------------------" << "\n";
          os << "// Output Variables" << "\n";
          os << "// -----------------------------------------------------------------------------" << "\n";
          os << ss.str();
        }
      }
    }

  }
  else {
    // Not implemented yet
    assert(false && "not implemented");
    return DIFF_FAILURE;
  }

  return result;
}

bool GetSets(
  const spv_reflect::ShaderModule&        reflection, 
  std::vector<SpvReflectDescriptorSet*>&  sets
)
{
  uint32_t count = 0;
  SpvReflectResult result = reflection.EnumerateDescriptorSets(&count, nullptr);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    return false;
  }
  sets.resize(count);
  result = reflection.EnumerateDescriptorSets(&count, sets.data());
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    return false;
  }
  return true;
}

int DiffDescriptor(
  uint32_t                            set_number,
  uint32_t                            binding_number,
  const SpvReflectDescriptorBinding*  p_binding_a,
  const SpvReflectDescriptorBinding*  p_binding_b,
  std::ostream&                       os
)
{
  int result = DIFF_SAME;
  bool is_in_a = (p_binding_a != nullptr);
  bool is_in_b = (p_binding_b != nullptr);
  if (is_in_a && is_in_b) {
    if (p_binding_a->descriptor_type != p_binding_b->descriptor_type) {
      os << "\n";
      os << "descriptor " << set_number << "." << binding_number << " has descriptor type difference" << "\n";
      os << "< A=" << ToStringDescriptorType(p_binding_a->descriptor_type) << ") " << "\n";
      os << "> B=" << ToStringDescriptorType(p_binding_b->descriptor_type) << ") " << "\n";
    }
  }
  else if (is_in_a && !is_in_b) {
    result = DIFF_DIFFERENCE;
    os << "\n";
    os << "descriptor " << set_number << "." << binding_number << " is in A but not in B" << "\n";
    os << "< A=" << "TRUE" << " (" << ToStringDescriptorType(p_binding_a->descriptor_type) << ") " << "\n";
    os << "> B=" << "false" "\n";
  }
  else if (!is_in_a && is_in_b) {
    result = DIFF_DIFFERENCE;
    os << "\n";
    os << "descriptor " << set_number << "." << binding_number << " is in B but not in A" << "\n";
    os << "< A=" << "false" << "\n";
    os << "> B=" << "TRUE" << " (" << ToStringDescriptorType(p_binding_b->descriptor_type) << ") " << "\n";
  }
  else {
    // Should never get here
    assert(false && "unreachable code path");
    return DIFF_FAILURE;
  }
  return result;
}

int DiffDescriptors(
  uint32_t                          set_number,
  const SpvReflectDescriptorSet*    p_set_a,
  const spv_reflect::ShaderModule&  reflection_a,
  const SpvReflectDescriptorSet*    p_set_b,
  const spv_reflect::ShaderModule&  reflection_b,
  std::ostream&                     os
)
{
  int result = DIFF_SAME;

  bool is_in_a = (p_set_a != nullptr);
  bool is_in_b = (p_set_b != nullptr);

  std::set<uint32_t> binding_numbers;
  if (is_in_a && is_in_b) {
    // A
    for (uint32_t i = 0; i < p_set_a->binding_count; ++i) {
      auto& p_binding = p_set_a->bindings[i];
      uint32_t number = p_binding->binding;
      binding_numbers.insert(number);
    }
    // B
    for (uint32_t i = 0; i < p_set_b->binding_count; ++i) {
      auto& p_binding = p_set_b->bindings[i];
      uint32_t number = p_binding->binding;
      binding_numbers.insert(number);
    }
  }
  else if (is_in_a && !is_in_b) {
    for (uint32_t i = 0; i < p_set_a->binding_count; ++i) {
      auto& p_binding = p_set_a->bindings[i];
      uint32_t number = p_binding->binding;
      binding_numbers.insert(number);
    }
  }
  else if (!is_in_a && is_in_b) {
    for (uint32_t i = 0; i < p_set_b->binding_count; ++i) {
      auto& p_binding = p_set_b->bindings[i];
      uint32_t number = p_binding->binding;
      binding_numbers.insert(number);
    }
  }
  else {
    // Should never get here
    assert(false && "unreachable code path");
    return DIFF_FAILURE;
  }

  for (auto& binding_number : binding_numbers) {
    auto p_binding_a = reflection_a.GetDescriptorBinding(binding_number, set_number);
    auto p_binding_b = reflection_b.GetDescriptorBinding(binding_number, set_number);
    int result_binding = DiffDescriptor(set_number,
                                        binding_number,
                                        p_binding_a,
                                        p_binding_b,
                                        os);
    if (result_binding == DIFF_FAILURE) {
      return DIFF_FAILURE;
    }
    if (result_binding == DIFF_DIFFERENCE) {
      result =  DIFF_DIFFERENCE;
    }
  }

  return result;
}

int DiffDescriptors(
  const spv_reflect::ShaderModule&  reflection_a,
  const spv_reflect::ShaderModule&  reflection_b,
  std::ostream&                     os
)
{
  int result = DIFF_SAME;
  bool is_stage_same = (reflection_a.GetShaderStage() == reflection_b.GetShaderStage());
  if (is_stage_same) {
    std::vector<SpvReflectDescriptorSet*> sets_a;
    if (!GetSets(reflection_a, sets_a)) {
      return DIFF_FAILURE;
    }

    std::vector<SpvReflectDescriptorSet*> sets_b;
    if (!GetSets(reflection_b, sets_b)) {
      return DIFF_FAILURE;
    }

    // A
    std::set<uint32_t> set_numbers;
    for (auto& p_set : sets_a) {
      uint32_t number = p_set->set;
      set_numbers.insert(number);
    }

    // B
    for (auto& p_set : sets_b) {
      uint32_t number = p_set->set;
      set_numbers.insert(number);
    }

    for (auto& set_number : set_numbers) {
      const SpvReflectDescriptorSet* p_set_a = reflection_a.GetDescriptorSet(set_number);
      const SpvReflectDescriptorSet* p_set_b = reflection_b.GetDescriptorSet(set_number);
      std::stringstream ss;
      int result_sets = DiffDescriptors(set_number, 
                                        p_set_a,
                                        reflection_a,
                                        p_set_b,
                                        reflection_b,
                                        ss);
      if (result_sets == DIFF_FAILURE) {
        return DIFF_FAILURE;
      }
      if (result_sets == DIFF_DIFFERENCE) {
        result =  DIFF_DIFFERENCE;
        os << "\n";
        os << "// -----------------------------------------------------------------------------" << "\n";
        os << "// Set " << set_number << "\n";
        os << "// -----------------------------------------------------------------------------" << "\n";
        os << ss.str();
      }
    }
  }
  else {
    // Not implemented yet
    assert(false && "not implemented");
    return DIFF_FAILURE;
  }

  return result;
}

int Diff(
  const spv_reflect::ShaderModule&  reflection_a,
  const spv_reflect::ShaderModule&  reflection_b,
  uint32_t                          diff_options,
  std::ostream&                     os
)
{ 
  int result = DIFF_RESULT_NO_DIFFERENCE;
  // Input/output variables
  int result_diff = DiffIoVars(reflection_a, reflection_b, os);
  if (result_diff == DIFF_FAILURE) {
    return DIFF_FAILURE;
  }
  result |= (result_diff == DIFF_DIFFERENCE) ? DIFF_RESULT_IO_VARS : DIFF_RESULT_NO_DIFFERENCE;
  // Descriptors
  result_diff = DiffDescriptors(reflection_a, reflection_b, os);
  if (result_diff == DIFF_FAILURE) {
    return DIFF_FAILURE;
  }
  result |= (result_diff == DIFF_DIFFERENCE) ? DIFF_RESULT_DESCRIPTORS : DIFF_RESULT_NO_DIFFERENCE;  
  return result;
}

// =================================================================================================
// PrintUsage()
// =================================================================================================
void PrintUsage()
{
  std::cout
    << "Usage: spirv-diff [OPTIONS] A.spv B.spv" << std::endl
    << "Diffs interface variables and bindings for two SPIR-V files." << std::endl
    << "Options:" << std::endl
    << " -h,--help        Display this message" << std::endl
    << " -i,--iovars      Diffs input and output variables." << std::endl
    << "                  If A.spv and B.spv are the same shader stage, both input" << std::endl
    << "                     and output variables will be compared." << std::endl
    << "                  If A.spv and B.spv are the different shader stage, output" << std::endl
    << "                     variables of the earlier stage will be compared against" << std::endl
    << "                     input variables of the later stage." << std::endl
    << "-d,--descriptors  Diffs descriptor bindings" << std::endl
    << "                  If A.spv and B.spv are the same shader stage, all" << std::endl
    << "                     bindings in all sets will be compared." << std::endl
    << "                  If A.spv and B.spv are the different shader stage, only" << std::endl
    << "                     bindings tghat exist in both files will be compared." << std::endl;
}

// =================================================================================================
// ReadFile(const std::string& path, std::vector<char>& data)
// =================================================================================================
bool ReadFile(const std::string& input_spv_path, std::vector<char>& spv_data)
{
  std::ifstream spv_ifstream(input_spv_path.c_str(), std::ios::binary);
  if (!spv_ifstream.is_open()) {
    std::cerr << "ERROR: could not open '" << input_spv_path << "' for reading" << std::endl;
    return false;
  }

  // File size
  spv_ifstream.seekg(0, std::ios::end);
  size_t size = spv_ifstream.tellg();
  spv_ifstream.seekg(0, std::ios::beg);

  // Read data
  spv_data.resize(size);
  spv_ifstream.read(spv_data.data(), size);
  
  return true;
}

// =================================================================================================
// main()
// =================================================================================================
int main(int argn, char** argv)
{
  ArgParser arg_parser;
  arg_parser.AddFlag("h", "help", "");
  arg_parser.AddFlag("i", "iovars", "");
  arg_parser.AddFlag("d", "descriptors", "");
  if (!arg_parser.Parse(argn, argv, std::cerr)) {
    PrintUsage();
    return EXIT_FAILURE;
  }

  uint32_t options = 0;
  if (arg_parser.GetFlag("i", "iovars")) {
    options |= DIFF_OPTIONS_IO_VARS;
  }
  if (arg_parser.GetFlag("d", "descriptors")) {
    options |= DIFF_OPTIONS_DESCRIPTORS;
  }
  if (options == 0) {
    options = DIFF_OPTIONS_IO_VARS | DIFF_OPTIONS_DESCRIPTORS;
  }
  
	std::string input_spv_path_a;
	if (!arg_parser.GetArg(0, &input_spv_path_a)) {
    std::cerr << "ERROR: Invalid number of arguments provided." << std::endl
              << "       Usage: spirv-diff [OPTIONS] A.spv B.spv" << std::endl;
    return EXIT_FAILURE;
	}

	std::string input_spv_path_b;
	if (!arg_parser.GetArg(1, &input_spv_path_b)) {
    std::cerr << "ERROR: Invalid number of arguments provided." << std::endl
              << "       Usage: spirv-diff [OPTIONS] A.spv B.spv" << std::endl;
    return EXIT_FAILURE;
	}
  
  std::vector<char> spv_data_a;
  if (!ReadFile(input_spv_path_a, spv_data_a)) {
    return EXIT_FAILURE;
  }

  std::vector<char> spv_data_b;
  if (!ReadFile(input_spv_path_b, spv_data_b)) {
    return EXIT_FAILURE;
  }

  spv_reflect::ShaderModule reflection_a(spv_data_a.size(), spv_data_a.data());
  if (reflection_a.GetResult() != SPV_REFLECT_RESULT_SUCCESS) {
    std::cerr << "ERROR: could not process '" << input_spv_path_a
      << "' (is it a valid SPIR-V bytecode?)" << std::endl;
    return EXIT_FAILURE;
  }

  spv_reflect::ShaderModule reflection_b(spv_data_b.size(), spv_data_b.data());
  if (reflection_b.GetResult() != SPV_REFLECT_RESULT_SUCCESS) {
    std::cerr << "ERROR: could not process '" << input_spv_path_b
      << "' (is it a valid SPIR-V bytecode?)" << std::endl;
    return EXIT_FAILURE;
  }

  std::stringstream ss;
  ss << "A=" << input_spv_path_a << "\n";
  ss << "B=" << input_spv_path_b << "\n";
  int result = Diff(reflection_a,
                    reflection_b,
                    options,
                    ss);
  if (result != DIFF_RESULT_NO_DIFFERENCE) {
    std::cout << ss.str();
    std::cout << std::endl;
    return result;
  }

  return EXIT_SUCCESS;
}
