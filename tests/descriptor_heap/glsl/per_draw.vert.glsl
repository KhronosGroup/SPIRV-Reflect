// Compile: glslang -V --target-env vulkan1.3 -S vert -e main -o per_draw.vert.spv per_draw.vert.glsl
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

#include "shaderio.h"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;
layout(location = 2) flat out int outFaceIdx;

layout(push_constant) uniform PushConstants_
{
  FrameInfo frame;
  DrawData  draw;
};

// Per-draw mode: the CPU issues one draw call per cube, pushing DrawData each
// time. The transform and cubeIndex come from push data; baseFaceTexIdx is
// consumed by the descriptor heap mapping (not read here) to resolve
// faceTextures[0..5].
void main()
{
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
  float tIn  = clamp((loopTime - float(draw.cubeIndex) * cubeDelay) / fallDuration, 0.0, 1.0);
  float tOut = clamp((loopTime - fallOutStart - float(draw.cubeIndex) * cubeDelay) / fallDuration, 0.0, 1.0);

  // Invisible before fall-in or after fall-out: degenerate vertex
  bool visible = (tIn > 0.0) && (tOut < 1.0);

  // Y offset: fall in from above, then fall out below
  float h       = frame.dropHeight;
  float yOffset = h * (1.0 - tIn * tIn) - h * tOut * tOut;

  vec3 worldPos = (draw.transform * vec4(inPosition, 1.0)).xyz;
  worldPos.y += yOffset;

  gl_Position = visible ? (frame.proj * frame.view * vec4(worldPos, 1.0)) : vec4(0.0);
  outNormal   = mat3(draw.transform) * inNormal;

  // Face index from vertex normal (exact on cube faces)
  vec3 a     = abs(inNormal);
  int  axis  = (a.x > a.y && a.x > a.z) ? 0 : (a.y > a.z) ? 1 : 2;
  int  s     = inNormal[axis] > 0.0 ? 0 : 1;
  outFaceIdx = axis * 2 + s;  // 0..5: +X,-X,+Y,-Y,+Z,-Z

  // Compute UVs from the two non-dominant axes
  vec3 p = inPosition + 0.5;  // nvutils::createCube is [-0.5, 0.5]
  if(axis == 0)
    outUV = s == 0 ? vec2(1.0 - p.z, 1.0 - p.y) : vec2(p.z, 1.0 - p.y);
  else if(axis == 1)
    outUV = s == 0 ? vec2(p.x, 1.0 - p.z) : vec2(p.x, p.z);
  else
    outUV = s == 0 ? vec2(p.x, 1.0 - p.y) : vec2(1.0 - p.x, 1.0 - p.y);
}
