Texture2D<float4> t : register(t0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

uint2 spvTextureSize(Texture2D<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

void frag_main()
{
    uint _21_dummy_parameter;
    uint2 _21 = spvTextureSize(t, uint(0), _21_dummy_parameter);
    uint2 dims = _21;
    uint _24 = _21.x;
    uint _26 = _21.y;
    sk_FragColor = float4(float2(float(_24), float(_26)), float2(float(_24), float(_26)));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
