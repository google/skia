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

float4 main(float2 _24)
{
    bool ok = true;
    bool _57 = false;
    if (true)
    {
        float2 _41 = float2(_10_testInputs.xy);
        float2 _42 = float2(_10_testInputs.zw);
        _57 = all(bool2(_41.x == float2(-1.25f, 0.0f).x, _41.y == float2(-1.25f, 0.0f).y)) && all(bool2(_42.x == float2(0.75f, 2.25f).x, _42.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _57 = false;
    }
    ok = _57;
    bool _74 = false;
    if (_57)
    {
        float2 _66 = float2(_10_testInputs.xy);
        float2 _67 = float2(_10_testInputs.zw);
        _74 = all(bool2(_66.x == float2(-1.25f, 0.0f).x, _66.y == float2(-1.25f, 0.0f).y)) && all(bool2(_67.x == float2(0.75f, 2.25f).x, _67.y == float2(0.75f, 2.25f).y));
    }
    else
    {
        _74 = false;
    }
    ok = _74;
    bool _95 = false;
    if (_74)
    {
        float2 _84 = float2(_10_colorGreen.xy);
        float2 _85 = float2(_10_colorGreen.zw);
        _95 = all(bool2(_84.x == float2(0.0f, 1.0f).x, _84.y == float2(0.0f, 1.0f).y)) && all(bool2(_85.x == float2(0.0f, 1.0f).x, _85.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _95 = false;
    }
    ok = _95;
    bool _112 = false;
    if (_95)
    {
        float2 _104 = float2(_10_colorGreen.xy);
        float2 _105 = float2(_10_colorGreen.zw);
        _112 = all(bool2(_104.x == float2(0.0f, 1.0f).x, _104.y == float2(0.0f, 1.0f).y)) && all(bool2(_105.x == float2(0.0f, 1.0f).x, _105.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _112 = false;
    }
    ok = _112;
    bool _148 = false;
    if (_112)
    {
        int4 _126 = int4(int(_10_colorGreen.x), int(_10_colorGreen.y), int(_10_colorGreen.z), int(_10_colorGreen.w));
        float4 _135 = float4(float(_126.x), float(_126.y), float(_126.z), float(_126.w));
        float2 _140 = float2(_135.xy);
        float2 _141 = float2(_135.zw);
        _148 = all(bool2(_140.x == float2(0.0f, 1.0f).x, _140.y == float2(0.0f, 1.0f).y)) && all(bool2(_141.x == float2(0.0f, 1.0f).x, _141.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _148 = false;
    }
    ok = _148;
    bool _165 = false;
    if (_148)
    {
        float2 _157 = float2(_10_colorGreen.xy);
        float2 _158 = float2(_10_colorGreen.zw);
        _165 = all(bool2(_157.x == float2(0.0f, 1.0f).x, _157.y == float2(0.0f, 1.0f).y)) && all(bool2(_158.x == float2(0.0f, 1.0f).x, _158.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _165 = false;
    }
    ok = _165;
    bool _182 = false;
    if (_165)
    {
        float2 _174 = float2(_10_colorGreen.xy);
        float2 _175 = float2(_10_colorGreen.zw);
        _182 = all(bool2(_174.x == float2(0.0f, 1.0f).x, _174.y == float2(0.0f, 1.0f).y)) && all(bool2(_175.x == float2(0.0f, 1.0f).x, _175.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _182 = false;
    }
    ok = _182;
    bool _218 = false;
    if (_182)
    {
        bool4 _196 = bool4(_10_colorGreen.x != 0.0f, _10_colorGreen.y != 0.0f, _10_colorGreen.z != 0.0f, _10_colorGreen.w != 0.0f);
        float4 _205 = float4(float(_196.x), float(_196.y), float(_196.z), float(_196.w));
        float2 _210 = float2(_205.xy);
        float2 _211 = float2(_205.zw);
        _218 = all(bool2(_210.x == float2(0.0f, 1.0f).x, _210.y == float2(0.0f, 1.0f).y)) && all(bool2(_211.x == float2(0.0f, 1.0f).x, _211.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _218 = false;
    }
    ok = _218;
    bool _242 = false;
    if (_218)
    {
        float4 _226 = _10_colorGreen - _10_colorRed;
        float2 _231 = float2(_226.xy);
        float2 _232 = float2(_226.zw);
        _242 = all(bool2(_231.x == float2(-1.0f, 1.0f).x, _231.y == float2(-1.0f, 1.0f).y)) && all(bool2(_232.x == 0.0f.xx.x, _232.y == 0.0f.xx.y));
    }
    else
    {
        _242 = false;
    }
    ok = _242;
    bool _265 = false;
    if (_242)
    {
        float4 _249 = _10_colorGreen + 5.0f.xxxx;
        float2 _254 = float2(_249.xy);
        float2 _255 = float2(_249.zw);
        _265 = all(bool2(_254.x == float2(5.0f, 6.0f).x, _254.y == float2(5.0f, 6.0f).y)) && all(bool2(_255.x == float2(5.0f, 6.0f).x, _255.y == float2(5.0f, 6.0f).y));
    }
    else
    {
        _265 = false;
    }
    ok = _265;
    float4 _266 = 0.0f.xxxx;
    if (_265)
    {
        _266 = _10_colorGreen;
    }
    else
    {
        _266 = _10_colorRed;
    }
    return _266;
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
