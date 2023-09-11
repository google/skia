cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float4 _7_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

uint spvPackUnorm2x16(float2 value)
{
    uint2 Packed = uint2(round(saturate(value) * 65535.0));
    return Packed.x | (Packed.y << 16);
}

float2 spvUnpackUnorm2x16(uint value)
{
    uint2 Packed = uint2(value & 0xffff, value >> 16);
    return float2(Packed) / 65535.0;
}

float4 main(float2 _21)
{
    uint _26 = spvPackUnorm2x16(_7_testInputs.xy);
    uint xy = _26;
    uint _34 = spvPackUnorm2x16(_7_testInputs.zw);
    uint zw = _34;
    float2 _42 = abs(spvUnpackUnorm2x16(_26));
    bool _57 = false;
    if (all(bool2(_42.x < 0.015625f.xx.x, _42.y < 0.015625f.xx.y)))
    {
        float2 _51 = abs(spvUnpackUnorm2x16(_34) - float2(0.75f, 1.0f));
        _57 = all(bool2(_51.x < 0.015625f.xx.x, _51.y < 0.015625f.xx.y));
    }
    else
    {
        _57 = false;
    }
    float4 _58 = 0.0f.xxxx;
    if (_57)
    {
        _58 = _7_colorGreen;
    }
    else
    {
        _58 = _7_colorRed;
    }
    return _58;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
