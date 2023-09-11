cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    int check = 0;
    int _36 = 0 + ((_7_colorGreen.y == 1.0f) ? 0 : 1);
    check = _36;
    int _42 = _36 + int(_7_colorGreen.x == 1.0f);
    check = _42;
    int _53 = _42 + (all(bool2(_7_colorGreen.yx.x == _7_colorRed.xy.x, _7_colorGreen.yx.y == _7_colorRed.xy.y)) ? 0 : 1);
    check = _53;
    int _63 = _53 + int(any(bool2(_7_colorGreen.yx.x != _7_colorRed.xy.x, _7_colorGreen.yx.y != _7_colorRed.xy.y)));
    check = _63;
    float4 _65 = 0.0f.xxxx;
    if (_63 == 0)
    {
        _65 = _7_colorGreen;
    }
    else
    {
        _65 = _7_colorRed;
    }
    return _65;
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
