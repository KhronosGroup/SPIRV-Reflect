// Copyright (c) 2019 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "arg_parser.h"
#include "spirv_reflect.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <unordered_map>

using SpirvReflection = spv_reflect::ShaderModule;
using HashResult      = std::size_t; // as defined for std hash.

std::ostream& log() {
  return std::cerr;
}

bool g_log_file_loads = false;

class Timer {
 public:
  Timer()
      : start_(std::chrono::high_resolution_clock::now()) {
  }

  void reset() {
    start_ = std::chrono::high_resolution_clock::now();
  }

  float elapsed() {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    return duration_cast<milliseconds>(now - start_).count() / 1000.f;
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class HashedSpvVariableList {
 public:
  HashedSpvVariableList(
      std::vector<SpvReflectInterfaceVariable*> const& variables) {
    hash_interface(variables);
  }

  HashResult hash() const {
    return hash_;
  }

  friend bool operator==(
      HashedSpvVariableList const& a, HashedSpvVariableList const& b) {
    return a.bytes_ == b.bytes_;
  }

 private:
  template <typename T>
  void copy_bytes(T const& value, std::true_type) {
    std::copy(
        reinterpret_cast<char const*>(&value),
        reinterpret_cast<char const*>(&value) + sizeof(value),
        std::back_inserter(bytes_));
  }

  void copy_bytes(std::string const& value, std::false_type) {
    std::copy(value.begin(), value.end(), std::back_inserter(bytes_));
  }

  template <typename T>
  void hash_value(T const& value) {
    hash_ = std::hash<T>{}(value) ^ (hash_ << 1);
    copy_bytes(value, typename std::is_pod<T>::type());
  }

  void hash_variable(SpvReflectInterfaceVariable const& item) {
    hash_value(item.location);
    if(item.semantic) {
      hash_value(std::string(item.semantic));
    }
    hash_value(item.decoration_flags);
    hash_value(item.numeric.scalar.width);
    hash_value(item.numeric.scalar.signedness);
    hash_value(item.numeric.vector.component_count);
    hash_value(item.numeric.matrix.column_count);
    hash_value(item.numeric.matrix.row_count);
    hash_value(item.numeric.matrix.stride);
    hash_value(item.array.dims_count);
    for(std::uint32_t i = 0; i < item.array.dims_count; ++i) {
      hash_value(item.array.dims[i]);
    }
    hash_value(item.array.stride);
    hash_value(item.format);
  }

  void hash_variable_list(
      std::uint32_t count, SpvReflectInterfaceVariable const* items) {
    for(std::uint32_t i = 0; i < count; ++i) {
      auto&& item = items[i];
      if(item.location == 0xffffffff) {
        continue;
      }
      hash_variable(item);
      hash_variable_list(item.member_count, item.members);
    }
  }

  void hash_indirect_variable_list(
      std::uint32_t count, SpvReflectInterfaceVariable const* const* items) {
    for(std::uint32_t i = 0; i < count; ++i) {
      auto&& item = *items[i];
      if(item.location == 0xffffffff) {
        continue;
      }
      hash_variable(item);
      hash_variable_list(item.member_count, item.members);
    }
  }

  void hash_interface(
      std::vector<SpvReflectInterfaceVariable*> const& variables) {
    hash_indirect_variable_list(
        static_cast<std::uint32_t>(variables.size()), variables.data());
  }

  HashResult hash_ = std::hash<HashResult>{}(1);
  std::vector<char> bytes_;
};

namespace std {
template <>
struct hash<HashedSpvVariableList> {
  std::size_t operator()(HashedSpvVariableList const& item) const {
    return item.hash();
  }
};
} // namespace std

struct HashedReflectedSpirvFile {
  std::string file;
  HashedSpvVariableList input_hash;
  HashedSpvVariableList output_hash;
};

using SpirvModuleList = std::vector<HashedReflectedSpirvFile>;
using SpirvInterfaceMap =
    std::unordered_map<HashedSpvVariableList, SpirvModuleList>;

class HashedSpirvMap {
 public:
  void add_module(std::string path, SpirvReflection const& module) {
    ++module_count_;
    total_entry_points_ += module.GetEntryPointCount();
    for(std::uint32_t ep = 0; ep < module.GetEntryPointCount(); ++ep) {
      char const* entry_point = module.GetEntryPointName(ep);

      get_module_inputs(module, entry_point, input_variable_cache_);
      HashedSpvVariableList input_hash(input_variable_cache_);

      get_module_outputs(module, entry_point, output_variable_cache_);
      HashedSpvVariableList output_hash(output_variable_cache_);

      auto&& spirvs_for_input = spirv_by_input_[input_hash];
      spirvs_for_input.push_back({std::move(path), input_hash, output_hash});

      auto shader_stage = module.GetShaderStage();
      bool is_initial_stage =
          (shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) ||
          (shader_stage & SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT);

      if(is_initial_stage) {
        entry_points_.push_back(spirvs_for_input.back());
      }
    }
  }

  std::size_t module_count() {
    return module_count_;
  }

  SpirvModuleList const& entry_points() const {
    return entry_points_;
  }

  SpirvInterfaceMap const& spirv_by_input() const {
    return spirv_by_input_;
  }

 private:
  static void get_module_inputs(
      SpirvReflection const& spv,
      char const* entry_point,
      std::vector<SpvReflectInterfaceVariable*>& inputs) {
    std::uint32_t count     = 0;
    SpvReflectResult result = SPV_REFLECT_RESULT_SUCCESS;
    result =
        spv.EnumerateEntryPointInputVariables(entry_point, &count, nullptr);
    if(result != SPV_REFLECT_RESULT_SUCCESS) {
      std::stringstream msg;
      msg << "Failed to retrieve input for " << entry_point;
      throw std::runtime_error(msg.str());
    }
    inputs.clear();
    inputs.resize(count);
    result = spv.EnumerateEntryPointInputVariables(
        entry_point, &count, inputs.data());
    if(result != SPV_REFLECT_RESULT_SUCCESS) {
      std::stringstream msg;
      msg << "Failed to retrieve input for " << entry_point;
      throw std::runtime_error(msg.str());
    }

    sort_indirect(inputs);
  }

  static void get_module_outputs(
      SpirvReflection const& spv,
      char const* entry_point,
      std::vector<SpvReflectInterfaceVariable*>& outputs) {
    std::uint32_t count     = 0;
    SpvReflectResult result = SPV_REFLECT_RESULT_SUCCESS;
    result =
        spv.EnumerateEntryPointOutputVariables(entry_point, &count, nullptr);
    if(result != SPV_REFLECT_RESULT_SUCCESS) {
      std::stringstream msg;
      msg << "Failed to retrieve output for " << entry_point;
      throw std::runtime_error(msg.str());
    }
    outputs.clear();
    outputs.resize(count);
    result = spv.EnumerateEntryPointOutputVariables(
        entry_point, &count, outputs.data());
    if(result != SPV_REFLECT_RESULT_SUCCESS) {
      std::stringstream msg;
      msg << "Failed to retrieve output for " << entry_point;
      throw std::runtime_error(msg.str());
    }

    sort_indirect(outputs);
  }

  static void sort_indirect(std::vector<SpvReflectInterfaceVariable*>& vars) {
    std::sort(
        vars.begin(), vars.end(),
        [](SpvReflectInterfaceVariable const* a,
           SpvReflectInterfaceVariable const* b) {
          return a->location < b->location;
        });
  }

  SpirvInterfaceMap spirv_by_input_;
  SpirvModuleList entry_points_;
  std::vector<SpvReflectInterfaceVariable*> input_variable_cache_;
  std::vector<SpvReflectInterfaceVariable*> output_variable_cache_;
  std::size_t total_entry_points_ = 0;
  std::size_t module_count_       = 0;
};

void load_file(std::string const& file_path, std::vector<uint8_t>& buffer) {
  if(g_log_file_loads) {
    log() << "Loading " << file_path << std::endl;
  }

  std::ifstream fin(file_path, std::ios::binary);
  if(fin.bad()) {
    std::stringstream msg;
    msg << "Failed to open " << file_path;
    throw std::runtime_error(msg.str());
  }
  fin.seekg(0, std::ios::end);
  auto size = fin.tellg();
  fin.seekg(0, std::ios::beg);
  buffer.resize(size);
  fin.read(reinterpret_cast<char*>(buffer.data()), size);
}

template <typename DirectoryIterator>
HashedSpirvMap load_spirvs_impl(
    DirectoryIterator iter, std::string const& extension) {
  HashedSpirvMap map;
  std::vector<uint8_t> spirv;
  for(auto&& dir_entry : iter) {
    if(dir_entry.is_regular_file()) {
      if(dir_entry.path().extension() == extension) {
        auto path = dir_entry.path();
        load_file(dir_entry.path(), spirv);
        SpirvReflection reflection(spirv);
        if(reflection.GetResult() == SPV_REFLECT_RESULT_SUCCESS) {
          map.add_module(std::move(path), reflection);
        }
        else {
          log() << "Failed to reflect " << path << " : "
                << reflection.GetResult();
        }
      }
    }
  }

  return map;
}

template <typename FileIterator>
HashedSpirvMap load_spirvs_from_list(FileIterator iter) {
  HashedSpirvMap map;
  std::vector<std::uint8_t> spirv;
  auto end = FileIterator();
  while(iter != end) {
    auto path = *iter++;
    load_file(path, spirv);
    SpirvReflection reflection(spirv);
    if(reflection.GetResult() == SPV_REFLECT_RESULT_SUCCESS) {
      map.add_module(std::move(path), reflection);
    }
    else {
      log() << "Failed to reflect " << path << " : " << reflection.GetResult()
            << std::endl;
    }
  }

  return map;
}

void output_pipeline(std::ostream& out, SpirvModuleList const& pipeline) {
  std::transform(
      pipeline.begin(), pipeline.end(),
      std::ostream_iterator<std::string>(out, " "),
      [](HashedReflectedSpirvFile const& node) { return node.file; });
  out << '\n';
}

void build_pipeline_recursive(
    std::size_t& count,
    SpirvModuleList& pipeline,
    SpirvInterfaceMap const& spirv_by_input) {
  auto input_hash  = pipeline.back().output_hash;
  bool is_complete = std::any_of(
      pipeline.begin(), pipeline.end(),
      [input_hash](HashedReflectedSpirvFile const& Spirv) {
        return Spirv.input_hash == input_hash;
      });

  auto matching_nexts = spirv_by_input.find(pipeline.back().output_hash);
  if(matching_nexts == spirv_by_input.end()) {
    is_complete = true;
  }

  if(is_complete) {
    ++count;
    output_pipeline(std::cout, pipeline);
    return;
  }

  for(auto&& next_stage : matching_nexts->second) {
    pipeline.push_back(next_stage);
    build_pipeline_recursive(count, pipeline, spirv_by_input);
    pipeline.pop_back();
  }
}

int main(int argc, char** argv) {
  ArgParser arg_parser;
  arg_parser.AddFlag("h", "help", "Prints this help message");
  arg_parser.AddOptionString(
      "l", "list",
      "Source file with list of spirv. Defaults to stdin if not specified.");
  arg_parser.AddFlag("v", "verbose", "Verbose output");

  if(!arg_parser.Parse(argc, argv, std::cerr)) {
    std::cerr << "Use --help to see available options" << std::endl;
    return -1;
  }

  if(arg_parser.GetFlag("h", "help")) {
    std::cout << "find_pipelines" << std::endl;
    arg_parser.PrintHelp(std::cout);
    return 0;
  }

  bool verbose = arg_parser.GetFlag("v", "verbose");
  if(verbose) {
    g_log_file_loads = true;
  }

  std::istream* src    = &std::cin;
  char const* src_name = "stdin";
  std::ifstream file_src;
  std::string list_file = "";
  if(arg_parser.GetString("l", "list", &list_file)) {
    file_src.open(list_file);
    if(file_src.bad()) {
      std::stringstream msg;
      msg << "Failed to open file " << list_file;
      throw std::runtime_error(msg.str());
    }
    src      = &file_src;
    src_name = list_file.c_str();
  }

  Timer timer;
  log() << "Loading spir-v from " << src_name << std::endl;

  HashedSpirvMap map =
      load_spirvs_from_list(std::istream_iterator<std::string>(*src));

  log() << " (" << map.module_count() << " in " << timer.elapsed()
        << " seconds)" << std::endl;

  log() << "Matching pipelines " << std::endl;
  timer.reset();

  SpirvModuleList pipeline;
  auto&& spirv_by_input = map.spirv_by_input();
  std::size_t count     = 0;
  for(auto&& entry_point : map.entry_points()) {
    pipeline.push_back(entry_point);
    build_pipeline_recursive(count, pipeline, spirv_by_input);
    pipeline.pop_back();
  }

  log() << "(" << count << " in " << timer.elapsed() << " seconds)"
        << std::endl;

  return 0;
}
