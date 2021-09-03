#include "stripper.h"

#include <cstdio>
#include <cstring>
#include <vector>

int SpvStripReflect(uint32_t* data, size_t len) {
  const uint32_t kHeaderLength = 5;
  const uint32_t kMagicNumber = 0x07230203u;
  const uint32_t kExtensionOpcode = 10;
  const uint32_t kSourceContinuedOpcode = 2;
  const uint32_t kSourceOpcode = 3;
  const uint32_t kStringOpcode = 7;
  const uint32_t kLineOpcode = 8;
  const uint32_t kModuleProcessedOpcode = 330;
  const uint32_t kDecorateIdOpcode = 332;
  const uint32_t kDecorateStringOpcode = 5632;
  const uint32_t kMemberDecorateStringOpcode = 5633;
  const uint32_t kCounterBufferDecoration = 5634;
  const uint32_t kUserTypeGOOGLEDecoration = 5636;

  // Make sure we at least have a header and the magic number is correct
  if (!data || len < kHeaderLength || data[0] != kMagicNumber)
    return -1;

  std::vector<uint32_t> spirv;
  spirv.reserve(len);

  for (uint32_t i = 0; i < kHeaderLength; ++i)
    spirv.push_back(data[i]);

  for (uint32_t pos = kHeaderLength; pos < len;) {
    const uint32_t inst_len = (data[pos] >> 16);
    const uint32_t opcode = data[pos] & 0x0000ffffu;

    bool skip = false;
    if (opcode == kDecorateStringOpcode ||
        opcode == kMemberDecorateStringOpcode ||
        opcode == kSourceContinuedOpcode ||
        opcode == kSourceOpcode ||
        opcode == kStringOpcode ||
        opcode == kLineOpcode ||
        opcode == kModuleProcessedOpcode) {
      skip = true;
    } else if (opcode == kDecorateIdOpcode) {
      if (pos + 2 >= len)
        return -1;
      if (data[pos + 2] == kCounterBufferDecoration ||
          data[pos + 2] == kUserTypeGOOGLEDecoration) {
        skip = true;
      }
    } else if (opcode == kExtensionOpcode) {
      if (pos + 1 >= len)
        return -1;
      const char *ext_name = reinterpret_cast<const char *>(&data[pos + 1]);
      if (0 == std::strcmp(ext_name, "SPV_GOOGLE_decorate_string") ||
          0 == std::strcmp(ext_name, "SPV_GOOGLE_hlsl_functionality1") ||
          0 == std::strcmp(ext_name, "SPV_GOOGLE_user_type"))
        skip = true;
    }

    if (!skip)
      for (uint32_t i = 0; i < inst_len; ++i)
        spirv.push_back(data[pos + i]);
    pos += inst_len;
  }

  for (uint32_t i = 0; i < spirv.size(); ++i)
    data[i] = spirv[i];

  return static_cast<int>(spirv.size());
}
