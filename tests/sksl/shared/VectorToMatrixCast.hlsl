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

float4 main(float2 _21)
{
    bool ok = true;
    bool _55 = false;
    if (true)
    {
        float2 _39 = float2(_7_testInputs.xy);
        float2 _40 = float2(_7_testInputs.zw);
        _55 = all(bool2(_39.x == float2(-1.25f, 0.0f).x, _39.y == float2(-1.25f, 0.0f).y)) && all(bool2(_40.x == float2(0.75f, 2.25f).x, _40.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _55 = false;
    }
    ok = _55;
    bool _72 = false;
    if (_55)
    {
        float2 _64 = float2(_7_testInputs.xy);
        float2 _65 = float2(_7_testInputs.zw);
        _72 = all(bool2(_64.x == float2(-1.25f, 0.0f).x, _64.y == float2(-1.25f, 0.0f).y)) && all(bool2(_65.x == float2(0.75f, 2.25f).x, _65.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _72 = false;
    }
    ok = _72;
    bool _93 = false;
    if (_72)
    {
        float2 _82 = float2(_7_colorGreen.xy);
        float2 _83 = float2(_7_colorGreen.zw);
        _93 = all(bool2(_82.x == float2(0.0f, 1.0f).x, _82.y == float2(0.0f, 1.0f).y)) && all(bool2(_83.x == float2(0.0f, 1.0f).x, _83.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _93 = false;
    }
    ok = _93;
    bool _110 = false;
    if (_93)
    {
        float2 _102 = float2(_7_colorGreen.xy);
        float2 _103 = float2(_7_colorGreen.zw);
        _110 = all(bool2(_102.x == float2(0.0f, 1.0f).x, _102.y == float2(0.0f, 1.0f).y)) && all(bool2(_103.x == float2(0.0f, 1.0f).x, _103.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _110 = false;
    }
    ok = _110;
    bool _146 = false;
    if (_110)
    {
        int4 _124 = int4(int(_7_colorGreen.x), int(_7_colorGreen.y), int(_7_colorGreen.z), int(_7_colorGreen.w));
        float4 _133 = float4(float(_124.x), float(_124.y), float(_124.z), float(_124.w));
        float2 _138 = float2(_133.xy);
        float2 _139 = float2(_133.zw);
        _146 = all(bool2(_138.x == float2(0.0f, 1.0f).x, _138.y == float2(0.0f, 1.0f).y)) && all(bool2(_139.x == float2(0.0f, 1.0f).x, _139.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _146 = false;
    }
    ok = _146;
    bool _163 = false;
    if (_146)
    {
        float2 _155 = float2(_7_colorGreen.xy);
        float2 _156 = float2(_7_colorGreen.zw);
        _163 = all(bool2(_155.x == float2(0.0f, 1.0f).x, _155.y == float2(0.0f, 1.0f).y)) && all(bool2(_156.x == float2(0.0f, 1.0f).x, _156.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _163 = false;
    }
    ok = _163;
    bool _180 = false;
    if (_163)
    {
        float2 _172 = float2(_7_colorGreen.xy);
        float2 _173 = float2(_7_colorGreen.zw);
        _180 = all(bool2(_172.x == float2(0.0f, 1.0f).x, _172.y == float2(0.0f, 1.0f).y)) && all(bool2(_173.x == float2(0.0f, 1.0f).x, _173.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _180 = false;
    }
    ok = _180;
    bool _216 = false;
    if (_180)
    {
        bool4 _194 = bool4(_7_colorGreen.x != 0.0f, _7_colorGreen.y != 0.0f, _7_colorGreen.z != 0.0f, _7_colorGreen.w != 0.0f);
        float4 _203 = float4(float(_194.x), float(_194.y), float(_194.z), float(_194.w));
        float2 _208 = float2(_203.xy);
        float2 _209 = float2(_203.zw);
        _216 = all(bool2(_208.x == float2(0.0f, 1.0f).x, _208.y == float2(0.0f, 1.0f).y)) && all(bool2(_209.x == float2(0.0f, 1.0f).x, _209.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _216 = false;
    }
    ok = _216;
    bool _240 = false;
    if (_216)
    {
        float4 _224 = _7_colorGreen - _7_colorRed;
        float2 _229 = float2(_224.xy);
        float2 _230 = float2(_224.zw);
        _240 = all(bool2(_229.x == float2(-1.0f, 1.0f).x, _229.y == float2(-1.0f, 1.0f).y)) && all(bool2(_230.x == 0.0f.xx.x, _230.y == 0.0f.xx.y));
    }
    else
    {
        _240 = false;
    }
    ok = _240;
    bool _263 = false;
    if (_240)
    {
        float4 _247 = _7_colorGreen + 5.0f.xxxx;
        float2 _252 = float2(_247.xy);
        float2 _253 = float2(_247.zw);
        _263 = all(bool2(_252.x == float2(5.0f, 6.0f).x, _252.y == float2(5.0f, 6.0f).y)) && all(bool2(_253.x == float2(5.0f, 6.0f).x, _253.y == float2(5.0f, 6.0f).y));
    }
    else
    {
        _263 = false;
    }
    ok = _263;
    float4 _264 = 0.0f.xxxx;
    if (_263)
    {
        _264 = _7_colorGreen;
    }
    else
    {
        _264 = _7_colorRed;
    }
    return _264;
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
