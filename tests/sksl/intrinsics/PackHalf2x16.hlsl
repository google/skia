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

uint spvPackHalf2x16(float2 value)
{
    uint2 Packed = f32tof16(value);
    return Packed.x | (Packed.y << 16);
}

float2 spvUnpackHalf2x16(uint value)
{
    return f16tof32(uint2(value & 0xffff, value >> 16));
}

float4 main(float2 _21)
{
    uint _26 = spvPackHalf2x16(_7_testInputs.xy);
    uint xy = _26;
    uint _34 = spvPackHalf2x16(_7_testInputs.zw);
    uint zw = _34;
    float2 _40 = spvUnpackHalf2x16(_26);
    bool _54 = false;
    if (all(bool2(_40.x == float2(-1.25f, 0.0f).x, _40.y == float2(-1.25f, 0.0f).y)))
    {
        float2 _48 = spvUnpackHalf2x16(_34);
        _54 = all(bool2(_48.x == float2(0.75f, 2.25f).x, _48.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _54 = false;
    }
    float4 _55 = 0.0f.xxxx;
    if (_54)
    {
        _55 = _7_colorGreen;
    }
    else
    {
        _55 = _7_colorRed;
    }
    return _55;
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
