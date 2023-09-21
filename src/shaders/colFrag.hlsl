struct FragmentIn
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

float4 fs_main(FragmentIn i) : SV_TARGET
{
    //return i.col;
    return float4(0.8, 0.8, 0.8, 1);
}
