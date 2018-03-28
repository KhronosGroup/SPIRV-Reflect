#include "io.h"
#include "stripper.h"

int main(int argc, char** argv) {
  const char* inFile = nullptr;
  const char* outFile = nullptr;
  for (int argi = 1; argi < argc; ++argi) {
    if ('-' == argv[argi][0]) {
      switch (argv[argi][1]) {
        case 'o': {
          if (!outFile && argi + 1 < argc) {
            outFile = argv[++argi];
          } else {
            fprintf(stderr, "error: -o option error\n");
            return 1;
          }
        } break;
        case 0: {
          // Setting a filename of "-" to indicate stdin.
          if (!inFile) {
            inFile = argv[argi];
          } else {
            fprintf(stderr, "error: more than one input file specified\n");
            return 1;
          }
        } break;
        default:
          fprintf(stderr,
                  "error: unrecognized option: %s (only -o supported)\n\n",
                  argv[argi]);
          return 1;
      }
    } else {
      if (!inFile) {
        inFile = argv[argi];
      } else {
        fprintf(stderr, "error: more than one input file specified\n");
        return 1;
      }
    }
  }

  if (!outFile) {
    outFile = "out.spv";
  }

  std::vector<uint32_t> contents;
  if (!ReadFile<uint32_t>(inFile, "rb", &contents)) {
    fprintf(stderr, "error: failed to read\n");
    return 1;
  }

  const auto size = SpvStripReflect(contents.data(), contents.size());
  if (size < 0) {
    fprintf(stderr, "error: failed to strip\n");
    return -1;
  }
  contents.resize(size);

  if (!WriteFile<uint32_t>(outFile, "wb", contents.data(), size)) {
    fprintf(stderr, "error: failed to write\n");
    return 1;
  }

  return 0;
}
