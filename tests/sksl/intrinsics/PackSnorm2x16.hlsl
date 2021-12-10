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

uint spvPackSnorm2x16(float2 value)
{
    int2 Packed = int2(round(clamp(value, -1.0, 1.0) * 32767.0)) & 0xffff;
    return uint(Packed.x | (Packed.y << 16));
}

float2 spvUnpackSnorm2x16(uint value)
{
    int SignedValue = int(value);
    int2 Packed = int2(SignedValue << 16, SignedValue) >> 16;
    return clamp(float2(Packed) / 32767.0, -1.0, 1.0);
}

float4 main(float2 _24)
{
    uint _29 = spvPackSnorm2x16(_10_testInputs.xy);
    uint xy = _29;
    uint _37 = spvPackSnorm2x16(_10_testInputs.zw);
    uint zw = _37;
    float2 _44 = abs(spvUnpackSnorm2x16(_29) - float2(-1.0f, 0.0f));
    bool _62 = false;
    if (all(bool2(_44.x < 0.015625f.xx.x, _44.y < 0.015625f.xx.y)))
    {
        float2 _56 = abs(spvUnpackSnorm2x16(_37) - float2(0.75f, 1.0f));
        _62 = all(bool2(_56.x < 0.015625f.xx.x, _56.y < 0.015625f.xx.y));
    }
    else
    {
        _62 = false;
    }
    float4 _63 = 0.0f.xxxx;
    if (_62)
    {
        _63 = _10_colorGreen;
    }
    else
    {
        _63 = _10_colorRed;
    }
    return _63;
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
