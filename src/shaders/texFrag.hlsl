Texture2DArray srv : register(t0);
SamplerState samplerState : register(s0);

struct FragmentIn
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    float3 fragPos : TEXCOORD1;
    float3 viewDir : TEXCOORD2;
    unsigned int    tex     : TEXCOORD3;
};

float4 fs_main(FragmentIn i) : SV_TARGET
{

    float3 color = srv.Sample(samplerState, float3(i.uv, i.tex)).rgb;

    float3 viewPos = i.viewDir;
    float3 lightColor = float3(1.0f, 1.0f, 1.0f);
    float3 lightPos = float3(0, 20, 40);

    float ambientStrength = 0.2f;
    float3 ambient = mul(lightColor, ambientStrength);
    // diffuse
    float3 norm = normalize(i.nor);
    float3 lightDir = normalize(lightPos - i.fragPos);
    float diffuseStrength = max(dot(norm, lightDir), 0.0f);
    float3 diffuse = mul(diffuseStrength, lightColor);
    // specular
    float specularStrength = 0.5f;
    float3 viewDir = normalize(viewPos - i.fragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f); 
    float3 specular = mul(lightColor * spec, specularStrength);

    float3 result = (ambient + diffuse + specular) * color;

    return float4(result, 1.0f);

    //float3 lightDir = normalize(float3(0.2, 1.0f, 0));

    //float4 surfaceColor = srv.Sample(samplerState, i.uv);
    //float3 coolColor = float3(0.0f, 0.0f, 0.55f) + mul(surfaceColor.rgb, 0.25f);
    //float3 warmColor = float3(0.3f, 0.3f, 0.0f) + mul(surfaceColor.rgb, 0.25f);
    //float3 highlight = float3(0.8f, 0.8f, 0.5f);
    //float t = (dot(i.nor, lightDir) + 1.0f) / 2.0f;
    //float3 r = mul(2.0f*dot(i.nor, lightDir), i.nor) - lightDir;
    //float s = saturate(100.0f*dot(r, i.viewDir)-97.0f);
    //float3 finalColor = mul(highlight, s) +  mul(mul(warmColor, t) + mul(coolColor, (1.0f - t)), 1.0f - s);
    //return float4(finalColor, surfaceColor.w);

}
