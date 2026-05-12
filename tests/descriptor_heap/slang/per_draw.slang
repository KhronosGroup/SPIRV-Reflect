// Compile (vertex):   slangc per_draw.slang -target spirv -entry vertexMain   -stage vertex   -o per_draw.vert.spv
// Compile (fragment): slangc per_draw.slang -target spirv -entry fragmentMain -stage fragment -o per_draw.frag.spv
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

#include "shaderio.h"

// Push constants
struct PushBlock
{
  FrameInfo frame;
  DrawData  draw;
};
[[vk::push_constant]]
ConstantBuffer<PushBlock> push;

// Per-draw mode: 6 face textures mapped via the descriptor heap mapping API.
// The mapping uses HEAP_WITH_PUSH_INDEX: the driver reads baseFaceTexIdx from
// push data at pushOffset to compute the heap index. The shader accesses
// faceTextures[faceIdx] and the mapping resolves it to the correct heap slot.
// baseFaceTexIdx is NOT read by this shader — the mapping consumes it.
[[vk::binding(0, 0)]]
Texture2D faceTextures[6];
[[vk::binding(1, 0)]]
SamplerState samp;

// Vertex input
struct VSin
{
  [[vk::location(0)]]
  float3 position;
  [[vk::location(1)]]
  float3 normal;
};

// Vertex-to-fragment interpolants
struct VSout
{
  float2              uv : TEXCOORD0;
  float3              normal : NORMAL;
  nointerpolation int faceIdx : TEXCOORD1;
  float4              sv_position : SV_Position;
};

// Fragment output
struct PSout
{
  float4 color : SV_Target;
};

// Per-draw mode: the CPU issues one draw call per cube, pushing DrawData each
// time. The transform and cubeIndex come from push data; baseFaceTexIdx is
// consumed by the descriptor heap mapping (not read here) to resolve
// faceTextures[0..5].
[shader("vertex")] VSout vertexMain(VSin input) {
  // Animation timing constants
  float cubeDelay    = 0.01;
  float fallDuration = 0.4;
  float restDuration = 1.5;

  // Compute cycle length and loop
  float totalStagger = float(push.frame.numCubes - 1u) * cubeDelay;
  float fallInEnd    = totalStagger + fallDuration;
  float fallOutStart = fallInEnd + restDuration;
  float cycleTime    = fallOutStart + totalStagger + fallDuration;
  float loopTime     = fmod(push.frame.time, cycleTime);

  // Per-cube fall-in and fall-out progress
  float tIn  = clamp((loopTime - float(push.draw.cubeIndex) * cubeDelay) / fallDuration, 0.0, 1.0);
  float tOut = clamp((loopTime - fallOutStart - float(push.draw.cubeIndex) * cubeDelay) / fallDuration, 0.0, 1.0);

  // Invisible before fall-in or after fall-out: degenerate vertex
  bool visible = (tIn > 0.0) && (tOut < 1.0);

  // Y offset: fall in from above, then fall out below
  float h       = push.frame.dropHeight;
  float yOffset = h * (1.0 - tIn * tIn) - h * tOut * tOut;

  float3 worldPos = mul(float4(input.position, 1.0), push.draw.transform).xyz;
  worldPos.y += yOffset;

  VSout output;
  output.sv_position = visible ? mul(mul(float4(worldPos, 1.0), push.frame.view), push.frame.proj) : float4(0.0);
  output.normal      = mul(input.normal, (float3x3)push.draw.transform);

  // Face index from vertex normal (exact on cube faces)
  float3 a       = abs(input.normal);
  int    axis    = (a.x > a.y && a.x > a.z) ? 0 : (a.y > a.z) ? 1 : 2;
  int    s       = input.normal[axis] > 0.0 ? 0 : 1;
  output.faceIdx = axis * 2 + s;  // 0..5: +X,-X,+Y,-Y,+Z,-Z

  // Compute UVs from the two non-dominant axes
  float3 p = input.position + 0.5;  // nvutils::createCube is [-0.5, 0.5]
  if(axis == 0)
    output.uv = s == 0 ? float2(1.0 - p.z, 1.0 - p.y) : float2(p.z, 1.0 - p.y);
  else if(axis == 1)
    output.uv = s == 0 ? float2(p.x, 1.0 - p.z) : float2(p.x, p.z);
  else
    output.uv = s == 0 ? float2(p.x, 1.0 - p.y) : float2(1.0 - p.x, 1.0 - p.y);

  return output;
}

float3 unpackColor(uint c)
{
  return float3(float(c & 0xFFu), float((c >> 8) & 0xFFu), float((c >> 16) & 0xFFu)) / 255.0;
}

[shader("pixel")] PSout fragmentMain(VSout stage) {
  float4 texColor = faceTextures[stage.faceIdx].Sample(samp, stage.uv);

  // Replace border pixels with per-draw hashed color
  float borderWidth = 1.0 / 48.0;
  if(stage.uv.x < borderWidth || stage.uv.x > 1.0 - borderWidth || stage.uv.y < borderWidth || stage.uv.y > 1.0 - borderWidth)
  {
    texColor.rgb = unpackColor(push.draw.borderColor);
  }

  float3 N     = normalize(stage.normal);
  float  NdotL = max(dot(N, normalize(push.frame.lightDir)), 0.0);

  PSout output;
  output.color = float4(texColor.rgb * (0.3 + 0.7 * NdotL), 1.0);

  return output;
}
