Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

struct FragmentIn
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 fs_main(FragmentIn i) : SV_TARGET
{
    float3 color = srv.Sample(samplerState, i.uv).rgb;
    return float4(color, 1.0f);
}
