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

float4 main(float2 _25)
{
    bool ok = true;
    bool _58 = false;
    if (true)
    {
        float2 _42 = float2(_11_testInputs.xy);
        float2 _43 = float2(_11_testInputs.zw);
        _58 = all(bool2(_42.x == float2(-1.25f, 0.0f).x, _42.y == float2(-1.25f, 0.0f).y)) && all(bool2(_43.x == float2(0.75f, 2.25f).x, _43.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _58 = false;
    }
    ok = _58;
    bool _75 = false;
    if (_58)
    {
        float2 _67 = float2(_11_testInputs.xy);
        float2 _68 = float2(_11_testInputs.zw);
        _75 = all(bool2(_67.x == float2(-1.25f, 0.0f).x, _67.y == float2(-1.25f, 0.0f).y)) && all(bool2(_68.x == float2(0.75f, 2.25f).x, _68.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _75 = false;
    }
    ok = _75;
    bool _96 = false;
    if (_75)
    {
        float2 _85 = float2(_11_colorGreen.xy);
        float2 _86 = float2(_11_colorGreen.zw);
        _96 = all(bool2(_85.x == float2(0.0f, 1.0f).x, _85.y == float2(0.0f, 1.0f).y)) && all(bool2(_86.x == float2(0.0f, 1.0f).x, _86.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _96 = false;
    }
    ok = _96;
    bool _113 = false;
    if (_96)
    {
        float2 _105 = float2(_11_colorGreen.xy);
        float2 _106 = float2(_11_colorGreen.zw);
        _113 = all(bool2(_105.x == float2(0.0f, 1.0f).x, _105.y == float2(0.0f, 1.0f).y)) && all(bool2(_106.x == float2(0.0f, 1.0f).x, _106.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _113 = false;
    }
    ok = _113;
    bool _149 = false;
    if (_113)
    {
        int4 _127 = int4(int(_11_colorGreen.x), int(_11_colorGreen.y), int(_11_colorGreen.z), int(_11_colorGreen.w));
        float4 _136 = float4(float(_127.x), float(_127.y), float(_127.z), float(_127.w));
        float2 _141 = float2(_136.xy);
        float2 _142 = float2(_136.zw);
        _149 = all(bool2(_141.x == float2(0.0f, 1.0f).x, _141.y == float2(0.0f, 1.0f).y)) && all(bool2(_142.x == float2(0.0f, 1.0f).x, _142.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _149 = false;
    }
    ok = _149;
    bool _166 = false;
    if (_149)
    {
        float2 _158 = float2(_11_colorGreen.xy);
        float2 _159 = float2(_11_colorGreen.zw);
        _166 = all(bool2(_158.x == float2(0.0f, 1.0f).x, _158.y == float2(0.0f, 1.0f).y)) && all(bool2(_159.x == float2(0.0f, 1.0f).x, _159.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _166 = false;
    }
    ok = _166;
    bool _183 = false;
    if (_166)
    {
        float2 _175 = float2(_11_colorGreen.xy);
        float2 _176 = float2(_11_colorGreen.zw);
        _183 = all(bool2(_175.x == float2(0.0f, 1.0f).x, _175.y == float2(0.0f, 1.0f).y)) && all(bool2(_176.x == float2(0.0f, 1.0f).x, _176.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _183 = false;
    }
    ok = _183;
    bool _219 = false;
    if (_183)
    {
        bool4 _197 = bool4(_11_colorGreen.x != 0.0f, _11_colorGreen.y != 0.0f, _11_colorGreen.z != 0.0f, _11_colorGreen.w != 0.0f);
        float4 _206 = float4(float(_197.x), float(_197.y), float(_197.z), float(_197.w));
        float2 _211 = float2(_206.xy);
        float2 _212 = float2(_206.zw);
        _219 = all(bool2(_211.x == float2(0.0f, 1.0f).x, _211.y == float2(0.0f, 1.0f).y)) && all(bool2(_212.x == float2(0.0f, 1.0f).x, _212.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _219 = false;
    }
    ok = _219;
    bool _243 = false;
    if (_219)
    {
        float4 _227 = _11_colorGreen - _11_colorRed;
        float2 _232 = float2(_227.xy);
        float2 _233 = float2(_227.zw);
        _243 = all(bool2(_232.x == float2(-1.0f, 1.0f).x, _232.y == float2(-1.0f, 1.0f).y)) && all(bool2(_233.x == 0.0f.xx.x, _233.y == 0.0f.xx.y));
    }
    else
    {
        _243 = false;
    }
    ok = _243;
    bool _266 = false;
    if (_243)
    {
        float4 _250 = _11_colorGreen + 5.0f.xxxx;
        float2 _255 = float2(_250.xy);
        float2 _256 = float2(_250.zw);
        _266 = all(bool2(_255.x == float2(5.0f, 6.0f).x, _255.y == float2(5.0f, 6.0f).y)) && all(bool2(_256.x == float2(5.0f, 6.0f).x, _256.y == float2(5.0f, 6.0f).y));
    }
    else
    {
        _266 = false;
    }
    ok = _266;
    float4 _267 = 0.0f.xxxx;
    if (_266)
    {
        _267 = _11_colorGreen;
    }
    else
    {
        _267 = _11_colorRed;
    }
    return _267;
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
