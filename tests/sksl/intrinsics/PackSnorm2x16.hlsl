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

float4 main(float2 _25)
{
    uint _30 = spvPackSnorm2x16(_11_testInputs.xy);
    uint xy = _30;
    uint _37 = spvPackSnorm2x16(_11_testInputs.zw);
    uint zw = _37;
    float2 _45 = abs(spvUnpackSnorm2x16(_30) - float2(-1.0f, 0.0f));
    bool _63 = false;
    if (all(bool2(_45.x < 0.015625f.xx.x, _45.y < 0.015625f.xx.y)))
    {
        float2 _57 = abs(spvUnpackSnorm2x16(_37) - float2(0.75f, 1.0f));
        _63 = all(bool2(_57.x < 0.015625f.xx.x, _57.y < 0.015625f.xx.y));
    }
    else
    {
        _63 = false;
    }
    float4 _64 = 0.0f.xxxx;
    if (_63)
    {
        _64 = _11_colorGreen;
    }
    else
    {
        _64 = _11_colorRed;
    }
    return _64;
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
