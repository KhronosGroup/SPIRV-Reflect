# Usage:
#   Prereq: build spirv-reflect
#   Prereq: build shader SPVs
#   cd $SPIRV_REFLECT_ROOT/tests
#   python build_golden_yaml.py
import argparse
import os
import os.path
import shutil
import subprocess
import sys

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Generate golden YAML from test shader .spv files")
  parser.add_argument("--verbose", "-v", help="enable verbose output", action='store_true')
  args = parser.parse_args()

  print("""\
WARNING: This script regenerates the golden YAML output for all test shaders.
The new YAML will be considered the expected correct output for future test
runs. Before commiting the updated YAML to GitHub, it is therefore critical
to carefully inspect the diffs between the old and new YAML output, to ensure
that all differences can be traced back to intentional changes to either the
reflection code or the test shaders.
""")

  spirv_reflect_exe_paths = [
      "../bin/Debug/spirv-reflect.exe",
      "../bin/Release/spirv-reflect.exe",
      "../bin/spirv-reflect",
  ]
  for path in spirv_reflect_exe_paths:
    if os.path.isfile(path):
      spirv_reflect_exe = path
      break

  spv_paths = []
  for root, dirs, files in os.walk("."):
    for f in files:
      base, ext = os.path.splitext(f)
      if ext.lower() == ".spv":
        spv_paths.append(os.path.normpath(os.path.join(root, f)))

  for spv_path in spv_paths:
    yaml_path = spv_path + ".yaml"
    try:
      # TODO Replace hard-coded EXE path with something less brittle.
      yaml_cmd_args = [spirv_reflect_exe, "-y", "-v", "1", spv_path]
      if args.verbose:
        print(" ".join(yaml_cmd_args))
      subprocess.call(yaml_cmd_args, stdout=file(yaml_path, "w"))
      print("%s -> %s" % (spv_path, yaml_path))
    except NameError:
      print("spirv-reflect application not found; did you build it first?")
      sys.exit()
    except subprocess.CalledProcessError as error:
      print("YAML generation failed with error code %d:\n%s" % (error.returncode, error.output.decode('utf-8')))
