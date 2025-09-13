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

float4 main(float2 _25)
{
    uint _30 = spvPackUnorm2x16(_11_testInputs.xy);
    uint xy = _30;
    uint _37 = spvPackUnorm2x16(_11_testInputs.zw);
    uint zw = _37;
    float2 _45 = abs(spvUnpackUnorm2x16(_30));
    bool _60 = false;
    if (all(bool2(_45.x < 0.015625f.xx.x, _45.y < 0.015625f.xx.y)))
    {
        float2 _54 = abs(spvUnpackUnorm2x16(_37) - float2(0.75f, 1.0f));
        _60 = all(bool2(_54.x < 0.015625f.xx.x, _54.y < 0.015625f.xx.y));
    }
    else
    {
        _60 = false;
    }
    float4 _61 = 0.0f.xxxx;
    if (_60)
    {
        _61 = _11_colorGreen;
    }
    else
    {
        _61 = _11_colorRed;
    }
    return _61;
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
