struct FragmentIn
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float3 nor : NORMAL;
    float3 fragPos : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
};

float4 fs_main(FragmentIn i) : SV_TARGET
{
    float3 lightPos = float3(0, 20, 40);
    float3 lightDir = normalize(lightPos - i.fragPos);
    //float3 lightDir = normalize(float3(0.2, 1.0f, 0));

    float4 surfaceColor = i.col;
    float3 coolColor = float3(0.0f, 0.0f, 0.55f) + mul(surfaceColor.rgb, 0.25f);
    float3 warmColor = float3(0.3f, 0.3f, 0.0f) + mul(surfaceColor.rgb, 0.25f);
    float3 highlight = float3(0.8f, 0.8f, 0.5f);
    float t = (dot(i.nor, lightDir) + 1.0f) / 2.0f;
    float3 r = mul(2.0f*dot(i.nor, lightDir), i.nor) - lightDir;
    float s = saturate(100.0f*dot(r, i.viewDir)-97.0f);
    float3 finalColor = mul(highlight, s) +  mul(mul(warmColor, t) + mul(coolColor, (1.0f - t)), 1.0f - s);
    return float4(finalColor, surfaceColor.w);
}
