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

float4 main(float2 _21)
{
    uint _26 = spvPackSnorm2x16(_7_testInputs.xy);
    uint xy = _26;
    uint _34 = spvPackSnorm2x16(_7_testInputs.zw);
    uint zw = _34;
    float2 _42 = abs(spvUnpackSnorm2x16(_26) - float2(-1.0f, 0.0f));
    bool _60 = false;
    if (all(bool2(_42.x < 0.015625f.xx.x, _42.y < 0.015625f.xx.y)))
    {
        float2 _54 = abs(spvUnpackSnorm2x16(_34) - float2(0.75f, 1.0f));
        _60 = all(bool2(_54.x < 0.015625f.xx.x, _54.y < 0.015625f.xx.y));
    }
    else
    {
        _60 = false;
    }
    float4 _61 = 0.0f.xxxx;
    if (_60)
    {
        _61 = _7_colorGreen;
    }
    else
    {
        _61 = _7_colorRed;
    }
    return _61;
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
