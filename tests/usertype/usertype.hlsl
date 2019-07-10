ByteAddressBuffer   BufferIn  : register(t0);
RWByteAddressBuffer BufferOut : register(u1);
Buffer<uint>        BufferIn2 : register(t2);

[numthreads(1, 1, 1)]
void csmain(uint3 tid : SV_DispatchThreadID)
{
  uint value = BufferIn2.Load(0);
  BufferOut.Store(tid.x*4, BufferIn.Load(tid.x*4) + value);
}