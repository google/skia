cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 expected = 0.0f.xxxx;
    bool _48 = false;
    if (ddx(_10_testInputs.x) == 0.0f)
    {
        float2 _40 = ddx(_10_testInputs.xy);
        _48 = all(bool2(_40.x == 0.0f.xxxx.xy.x, _40.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _48 = false;
    }
    bool _60 = false;
    if (_48)
    {
        float3 _51 = ddx(_10_testInputs.xyz);
        _60 = all(bool3(_51.x == 0.0f.xxxx.xyz.x, _51.y == 0.0f.xxxx.xyz.y, _51.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _60 = false;
    }
    bool _69 = false;
    if (_60)
    {
        float4 _63 = ddx(_10_testInputs);
        _69 = all(bool4(_63.x == 0.0f.xxxx.x, _63.y == 0.0f.xxxx.y, _63.z == 0.0f.xxxx.z, _63.w == 0.0f.xxxx.w));
    }
    else
    {
        _69 = false;
    }
    bool _80 = false;
    if (_69)
    {
        float2 _72 = sign(fwidth(_24.xx));
        _80 = all(bool2(_72.x == 1.0f.xx.x, _72.y == 1.0f.xx.y));
    }
    else
    {
        _80 = false;
    }
    bool _91 = false;
    if (_80)
    {
        float2 _83 = sign(fwidth(float2(_24.x, 1.0f)));
        _91 = all(bool2(_83.x == float2(1.0f, 0.0f).x, _83.y == float2(1.0f, 0.0f).y));
    }
    else
    {
        _91 = false;
    }
    bool _100 = false;
    if (_91)
    {
        float2 _94 = sign(fwidth(_24.yy));
        _100 = all(bool2(_94.x == 1.0f.xx.x, _94.y == 1.0f.xx.y));
    }
    else
    {
        _100 = false;
    }
    bool _111 = false;
    if (_100)
    {
        float2 _103 = sign(fwidth(float2(0.0f, _24.y)));
        _111 = all(bool2(_103.x == float2(0.0f, 1.0f).x, _103.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _111 = false;
    }
    bool _119 = false;
    if (_111)
    {
        float2 _114 = sign(fwidth(_24));
        _119 = all(bool2(_114.x == 1.0f.xx.x, _114.y == 1.0f.xx.y));
    }
    else
    {
        _119 = false;
    }
    float4 _120 = 0.0f.xxxx;
    if (_119)
    {
        _120 = _10_colorGreen;
    }
    else
    {
        _120 = _10_colorRed;
    }
    return _120;
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
