struct PS_Input 
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

float4 fs_main(PS_Input frag) : SV_TARGET
{
    return frag.col;
}
