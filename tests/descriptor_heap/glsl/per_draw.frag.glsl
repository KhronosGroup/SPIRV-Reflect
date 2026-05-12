// Compile: glslang -V --target-env vulkan1.3 -S frag -e main -IC:/code/source/nvpro-samples/nvpro_core2 -IC:/code/source/nvpro-samples/vk_mini_samples/common -o per_draw.frag.spv per_draw.frag.glsl
/*
 * Copyright (c) 2024-2026, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2024-2026, NVIDIA CORPORATION.
 * SPDX-License-Identifier: Apache-2.0
 */
#version 450

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "shaderio.h"

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) flat in int inFaceIdx;

layout(location = 0) out vec4 outColor;

// Per-draw mode: 6 face textures mapped via the descriptor heap mapping API.
// The mapping uses HEAP_WITH_PUSH_INDEX: the driver reads baseFaceTexIdx from
// push data at pushOffset to compute the heap index. The shader accesses
// faceTextures[faceIdx] and the mapping resolves it to the correct heap slot.
// baseFaceTexIdx is NOT read by this shader — the mapping consumes it.
layout(set = 0, binding = 0) uniform texture2D faceTextures[6];
layout(set = 0, binding = 1) uniform sampler samp;

layout(push_constant) uniform PushConstants_
{
  FrameInfo frame;
  DrawData  draw;
};

vec3 unpackColor(uint c)
{
  return vec3(float(c & 0xFFu), float((c >> 8) & 0xFFu), float((c >> 16) & 0xFFu)) / 255.0;
}

void main()
{
  vec4 texColor = texture(sampler2D(faceTextures[inFaceIdx], samp), inUV);

  // Replace border pixels with per-draw hashed color
  float borderWidth = 1.0 / 48.0;
  if(inUV.x < borderWidth || inUV.x > 1.0 - borderWidth || inUV.y < borderWidth || inUV.y > 1.0 - borderWidth)
  {
    texColor.rgb = unpackColor(draw.borderColor);
  }

  vec3  N     = normalize(inNormal);
  float NdotL = max(dot(N, normalize(frame.lightDir)), 0.0);
  outColor    = vec4(texColor.rgb * (0.3 + 0.7 * NdotL), 1.0);
}
