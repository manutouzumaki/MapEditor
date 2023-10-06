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
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    float3 fragPos : TEXCOORD1;
    float3 viewDir : TEXCOORD2;
    unsigned int    tex     : TEXCOORD3;
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
    o.uv = i.uv;
    o.nor = wNor;
    o.fragPos = wPos.xyz;
    o.viewDir = viewDir;
    o.tex = i.tex;

    return o;
}
