cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    float4 _11_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

uint spvPackHalf2x16(float2 value)
{
    uint2 Packed = f32tof16(value);
    return Packed.x | (Packed.y << 16);
}

float2 spvUnpackHalf2x16(uint value)
{
    return f16tof32(uint2(value & 0xffff, value >> 16));
}

float4 main(float2 _25)
{
    uint _30 = spvPackHalf2x16(_11_testInputs.xy);
    uint xy = _30;
    uint _37 = spvPackHalf2x16(_11_testInputs.zw);
    uint zw = _37;
    float2 _43 = spvUnpackHalf2x16(_30);
    bool _57 = false;
    if (all(bool2(_43.x == float2(-1.25f, 0.0f).x, _43.y == float2(-1.25f, 0.0f).y)))
    {
        float2 _51 = spvUnpackHalf2x16(_37);
        _57 = all(bool2(_51.x == float2(0.75f, 2.25f).x, _51.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _57 = false;
    }
    float4 _58 = 0.0f.xxxx;
    if (_57)
    {
        _58 = _11_colorGreen;
    }
    else
    {
        _58 = _11_colorRed;
    }
    return _58;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
