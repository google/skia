cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float4 _10_testInputs : packoffset(c2);
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

float4 main(float2 _24)
{
    uint _29 = spvPackHalf2x16(_10_testInputs.xy);
    uint xy = _29;
    uint _37 = spvPackHalf2x16(_10_testInputs.zw);
    uint zw = _37;
    float2 _42 = spvUnpackHalf2x16(_29);
    bool _56 = false;
    if (all(bool2(_42.x == float2(-1.25f, 0.0f).x, _42.y == float2(-1.25f, 0.0f).y)))
    {
        float2 _50 = spvUnpackHalf2x16(_37);
        _56 = all(bool2(_50.x == float2(0.75f, 2.25f).x, _50.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _56 = false;
    }
    float4 _57 = 0.0f.xxxx;
    if (_56)
    {
        _57 = _10_colorGreen;
    }
    else
    {
        _57 = _10_colorRed;
    }
    return _57;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
