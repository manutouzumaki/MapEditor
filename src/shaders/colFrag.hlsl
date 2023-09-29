struct FragmentIn
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float3 nor : NORMAL;
    float3 fragPos : TEXCOORD0;
    float3 viewPos : TEXCOORD1;
};

float4 fs_main(FragmentIn i) : SV_TARGET
{
    float3 color = i.col.rgb;
    float3 lightPos = i.viewPos; 
    float3 lightColor = float3(1.0f, 1.0f, 1.0f);
    float3 lightDir = normalize(float3(-0.2, -0.5, 0.6));
    //float3 lightDir = normalize(i.fragPos - lightPos);

    // ambient
    float ambientStrength = 0.2f;
    float3 ambient = mul(lightColor, ambientStrength);
    // diffuse
    float diffuseStrength = max(dot(i.nor, lightDir), 0.0f);
    float3 diffuse = mul(lightColor, diffuseStrength);
    // specular
    float specularStrength = 1.0f;
    float3 viewDir = normalize(i.fragPos - i.viewPos);
    float3 reflectDir = reflect(-lightDir, i.nor);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f); 
    float3 specular = mul(spec * lightColor, specularStrength);

    //float3 result = (ambient + diffuse + specular) * color;
    float3 result = (ambient + diffuse) * color;

    return float4(result, 1.0f);
}
