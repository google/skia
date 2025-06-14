cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    int check = 0;
    int _39 = 0 + ((_11_colorGreen.y == 1.0f) ? 0 : 1);
    check = _39;
    int _45 = _39 + int(_11_colorGreen.x == 1.0f);
    check = _45;
    int _56 = _45 + (all(bool2(_11_colorGreen.yx.x == _11_colorRed.xy.x, _11_colorGreen.yx.y == _11_colorRed.xy.y)) ? 0 : 1);
    check = _56;
    int _66 = _56 + int(any(bool2(_11_colorGreen.yx.x != _11_colorRed.xy.x, _11_colorGreen.yx.y != _11_colorRed.xy.y)));
    check = _66;
    float4 _68 = 0.0f.xxxx;
    if (_66 == 0)
    {
        _68 = _11_colorGreen;
    }
    else
    {
        _68 = _11_colorRed;
    }
    return _68;
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
