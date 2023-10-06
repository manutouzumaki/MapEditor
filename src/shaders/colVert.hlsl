cbuffer CBuffer : register(b0)
{
    matrix proj;
    matrix view;
    matrix wolrd;
    float3 viewDir;
    float pad;
}

struct VertexIn
{
    float3 pos    : POSITION;
    float3 nor    : NORMAL;
    float4 col    : COLOR;
    float2 uv     : TEXCOORD;
    unsigned int    tex    : TEXCOORD1;

};

struct VertexOut
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float3 nor : NORMAL;
    float3 fragPos : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
};

VertexOut vs_main(VertexIn i)
{

    VertexOut o = (VertexOut)0;

    float4 wPos =  mul(float4(i.pos, 1.0f), wolrd);
    wPos = mul(wPos, view);
    wPos = mul(wPos, proj);

    float3 wNor = mul(i.nor, (float3x3)wolrd);
    wNor = normalize(wNor);

    o.pos = wPos;
    o.col = i.col;
    o.nor = wNor;
    o.fragPos = wPos.xyz;
    o.viewDir = viewDir;

    return o;
}
