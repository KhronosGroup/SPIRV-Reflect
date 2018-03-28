#ifndef LIBSPIRV_SPV_STRIP_REFLECT_
#define LIBSPIRV_SPV_STRIP_REFLECT_

#include <cstddef>
#include <cstdint>

// Strips SPIR-V reflection decorations in the SPIR-V binary module pointed by
// |spirv|, which contains |len| words, and writes the stripped binary module
// back to |spirv|. Returns the size (in words) of the processed binary module
// on success; returns -1 on failure.
int SpvStripReflect(uint32_t *spirv, size_t len);

#endif // LIBSPIRV_SPV_STRIP_REFLECT_
