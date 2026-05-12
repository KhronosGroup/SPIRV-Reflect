// Compile: glslang -V --target-env vulkan1.3 -S vert -e main -IC:/code/source/nvpro-samples/nvpro_core2 -IC:/code/source/nvpro-samples/vk_mini_samples/common -o bindless.vert.spv bindless.vert.glsl
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
#version 460

#extension GL_GOOGLE_include_directive : enable

#include "shaderio.h"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;
layout(location = 2) flat out int outFaceIdx;
layout(location = 3) flat out uint outBaseFaceTexIdx;

layout(push_constant) uniform PushConstants_
{
  FrameInfo        frame;
  BindlessPushData bindless;
};

// Bindless instanced rendering: we draw all cubes in a single vkCmdDrawIndexed
// call with instanceCount = numCubes. Each instance renders the same cube mesh,
// but the instance ID lets us compute a unique world position and texture
// index. No per-cube data is bound on the CPU side — everything is derived here
// from the instance index and the grid parameters in push constants.
void main()
{
  // In this sample, gl_BaseInstance is zero but this matches slang's
  // SV_InstanceID
  int instanceID = gl_InstanceIndex - gl_BaseInstance;
  int N          = int(bindless.gridSize);
  int ix         = instanceID % N;
  int iy         = (instanceID / N) % N;
  int iz         = instanceID / (N * N);

  float spacing = 1.1;
  float off     = float(N - 1) * spacing * 0.5;
  vec3  cubePos = vec3(ix, iy, iz) * spacing - off;

  outBaseFaceTexIdx = uint(instanceID) * 6u;

  // Animation timing constants
  float cubeDelay    = 0.01;
  float fallDuration = 0.4;
  float restDuration = 1.5;

  // Compute cycle length and loop
  float totalStagger = float(frame.numCubes - 1u) * cubeDelay;
  float fallInEnd    = totalStagger + fallDuration;
  float fallOutStart = fallInEnd + restDuration;
  float cycleTime    = fallOutStart + totalStagger + fallDuration;
  float loopTime     = mod(frame.time, cycleTime);

  // Per-cube fall-in and fall-out progress
  float tIn  = clamp((loopTime - float(instanceID) * cubeDelay) / fallDuration, 0.0, 1.0);
  float tOut = clamp((loopTime - fallOutStart - float(instanceID) * cubeDelay) / fallDuration, 0.0, 1.0);

  // Invisible before fall-in or after fall-out: degenerate vertex
  bool visible = (tIn > 0.0) && (tOut < 1.0);

  // Y offset: fall in from above, then fall out below
  float h       = frame.dropHeight;
  float yOffset = h * (1.0 - tIn * tIn) - h * tOut * tOut;

  vec3 worldPos = inPosition + cubePos;
  worldPos.y += yOffset;

  gl_Position = visible ? (frame.proj * frame.view * vec4(worldPos, 1.0)) : vec4(0.0);
  outNormal   = inNormal;

  // Face index from vertex normal
  vec3 a     = abs(inNormal);
  int  axis  = (a.x > a.y && a.x > a.z) ? 0 : (a.y > a.z) ? 1 : 2;
  int  s     = inNormal[axis] > 0.0 ? 0 : 1;
  outFaceIdx = axis * 2 + s;

  // Compute UVs from the two non-dominant axes
  vec3 p = inPosition + 0.5;
  if(axis == 0)
    outUV = s == 0 ? vec2(1.0 - p.z, 1.0 - p.y) : vec2(p.z, 1.0 - p.y);
  else if(axis == 1)
    outUV = s == 0 ? vec2(p.x, 1.0 - p.z) : vec2(p.x, p.z);
  else
    outUV = s == 0 ? vec2(p.x, 1.0 - p.y) : vec2(1.0 - p.x, 1.0 - p.y);
}
