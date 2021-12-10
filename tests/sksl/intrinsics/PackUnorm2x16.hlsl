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

float4 main(float2 _24)
{
    uint xy = spvPackUnorm2x16(_10_testInputs.xy);
    uint zw = spvPackUnorm2x16(_10_testInputs.zw);
    float2 tolerance = 0.015625f.xx;
    float2 _47 = abs(spvUnpackUnorm2x16(xy));
    bool _64 = false;
    if (all(bool2(_47.x < tolerance.x, _47.y < tolerance.y)))
    {
        float2 _56 = abs(spvUnpackUnorm2x16(zw) - float2(0.75f, 1.0f));
        _64 = all(bool2(_56.x < tolerance.x, _56.y < tolerance.y));
    }
    else
    {
        _64 = false;
    }
    float4 _65 = 0.0f.xxxx;
    if (_64)
    {
        _65 = _10_colorGreen;
    }
    else
    {
        _65 = _10_colorRed;
    }
    return _65;
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
