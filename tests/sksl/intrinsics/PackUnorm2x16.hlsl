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
    uint _29 = spvPackUnorm2x16(_10_testInputs.xy);
    uint xy = _29;
    uint _37 = spvPackUnorm2x16(_10_testInputs.zw);
    uint zw = _37;
    float2 _44 = abs(spvUnpackUnorm2x16(_29));
    bool _59 = false;
    if (all(bool2(_44.x < 0.015625f.xx.x, _44.y < 0.015625f.xx.y)))
    {
        float2 _53 = abs(spvUnpackUnorm2x16(_37) - float2(0.75f, 1.0f));
        _59 = all(bool2(_53.x < 0.015625f.xx.x, _53.y < 0.015625f.xx.y));
    }
    else
    {
        _59 = false;
    }
    bool4 _61 = _59.xxxx;
    return float4(_61.x ? _10_colorGreen.x : _10_colorRed.x, _61.y ? _10_colorGreen.y : _10_colorRed.y, _61.z ? _10_colorGreen.z : _10_colorRed.z, _61.w ? _10_colorGreen.w : _10_colorRed.w);
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
