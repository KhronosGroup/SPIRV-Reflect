# Usage:
#   cd $SPIRV_REFLECT_ROOT/tests
#   python build_all_shaders.py
import argparse
import os
import os.path
import shutil
import subprocess
import sys

def my_which(cmd):
  if sys.hexversion >= 0x03030000:
    return shutil.which(cmd)
  else:
    try:
      subprocess.check_output([cmd], stderr=subprocess.STDOUT)
      return cmd
    except subprocess.CalledProcessError:
      return cmd # that's fine, it exists though
    except OSError:
      return None

shaders = [
  {'source':"glsl/built_in_format.glsl", 'entry':"main", 'stage':'vert'},
  {'source':"glsl/input_attachment.glsl", 'entry':"main", 'stage':'frag'},
  {'source':"glsl/texel_buffer.glsl", 'entry':"main", 'stage':'vert'},

  {'source':"hlsl/append_consume.hlsl", 'entry':"main", 'profile':'ps_6_0', 'stage':'frag'},
  {'source':"hlsl/binding_array.hlsl", 'entry':"main", 'profile':'ps_6_0', 'stage':'frag'},
  {'source':"hlsl/binding_types.hlsl", 'entry':"main", 'profile':'ps_6_0', 'stage':'frag'},
  {'source':"hlsl/cbuffer.hlsl", 'entry':"main", 'profile':'vs_6_0', 'stage':'vert'},
  {'source':"hlsl/counter_buffers.hlsl", 'entry':"main", 'profile':'ps_6_0', 'stage':'frag'},
  {'source':"hlsl/semantics.hlsl", 'entry':"main", 'profile':'ps_6_0', 'stage':'frag'},
]

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Compile test shaders")
  parser.add_argument("--glslc", help="path to glslc compiler", default=my_which("glslc"))
  parser.add_argument("--dxc", help="path to dxc compiler", default=my_which("dxc"))
  parser.add_argument("--verbose", "-v", help="enable verbose output", action='store_true')
  args = parser.parse_args()

  if not args.dxc:
    print("WARNING: dxc not found in PATH; HLSL shaders will be compiled with glslc.")
  if not args.glslc:
    print("WARNING: glslc not found in PATH. This is a bad sign.")
  for shader in shaders:
    src_path = shader['source']
    base, ext = os.path.splitext(src_path)
    spv_path = base + ".spv"
    if ext.lower() == ".glsl" or (ext.lower() == ".hlsl" and not args.dxc):
      compile_cmd_args = [args.glslc, "-fshader-stage=" + shader['stage'], "-fentry-point=" + shader['entry'], "-o", spv_path, src_path]
    elif ext.lower() == ".hlsl":
      compile_cmd_args = [args.dxc, "-spirv", "-fspv-reflect", "-O0", "-T", shader['profile'], "-E", shader['entry'], "-Fo", spv_path, src_path]

    if args.verbose:
      print(" ".join(compile_cmd_args))

    try:
      compile_cmd_output = subprocess.check_output(compile_cmd_args, stderr = subprocess.STDOUT)
      print("%s -> %s" % (src_path, spv_path))
    except subprocess.CalledProcessError as error:
      print("Compilation failed with error code %d:\n%s" % (error.returncode, error.output.decode('utf-8')))
