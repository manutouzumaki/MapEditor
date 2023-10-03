cbuffer CBuffer : register(b0)
{
    matrix proj;
    matrix view;
    matrix world;
    float3 viewDir;
    float pad;
}

struct VS_Input
{
    float3 pos : POSITION;
    float4 col : TEXCOORD0;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

PS_Input vs_main(VS_Input vertex)
{
    PS_Input vsOut = (PS_Input)0;
    float4 viewPos = mul(float4(vertex.pos, 1.0f), view);
    vsOut.pos = mul(viewPos, proj);
    vsOut.col = vertex.col;
    return vsOut;
}
