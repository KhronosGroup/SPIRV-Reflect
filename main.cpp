#include <cassert>
#include <iostream>
#include <fstream>

#include "spirv_reflect.hpp"

int main(int argn, char** argv)
{
  std::string file_path = "C:\\Users\\haing\\code\\SPIRV-Vars\\tests\\all.ps.spv";

  std::ifstream is(file_path.c_str(), std::ios::binary);
  assert(is.is_open());

  is.seekg(0, std::ios::end);
  size_t size = is.tellg();
  is.seekg(0, std::ios::beg);  

  std::vector<char> data(size);
  is.read(data.data(), size);

  spirv_ref::ShaderReflection reflection;
  spirv_ref::ParseShaderReflection(data.size(), data.data(), &reflection);

  auto count = reflection.GetTypeCount();
  for (auto i = 0; i < count; ++i) {
    auto elem = reflection.GetType(i);
    std::cout << elem->GetInfo("  ") << std::endl;
  }

  count = reflection.GetDescriptorCount();
  for (auto i = 0; i < count; ++i) {
    auto elem = reflection.GetDescriptor(i);
    std::cout << elem->GetInfo() << std::endl;
  }

  //reflection.GetDescriptorSet(0)->GetBinding(0)->SetBindingNumber(2);

  return 0;
}