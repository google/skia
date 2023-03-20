cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    int check = 0;
    int _38 = 0 + ((_10_colorGreen.y == 1.0f) ? 0 : 1);
    check = _38;
    int _44 = _38 + int(_10_colorGreen.x == 1.0f);
    check = _44;
    int _55 = _44 + (all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y)) ? 0 : 1);
    check = _55;
    int _65 = _55 + int(any(bool2(_10_colorGreen.yx.x != _10_colorRed.xy.x, _10_colorGreen.yx.y != _10_colorRed.xy.y)));
    check = _65;
    float4 _67 = 0.0f.xxxx;
    if (_65 == 0)
    {
        _67 = _10_colorGreen;
    }
    else
    {
        _67 = _10_colorRed;
    }
    return _67;
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
