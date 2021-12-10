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
    bool _62 = false;
    if (ok)
    {
        float2x2 _44 = float2x2(float2(_10_testInputs.xy), float2(_10_testInputs.zw));
        float2x2 _51 = float2x2(float2(-1.25f, 0.0f), float2(0.75f, 2.25f));
        float2 _53 = _44[0];
        float2 _54 = _51[0];
        float2 _57 = _44[1];
        float2 _58 = _51[1];
        _62 = all(bool2(_53.x == _54.x, _53.y == _54.y)) && all(bool2(_57.x == _58.x, _57.y == _58.y));
    }
    else
    {
        _62 = false;
    }
    ok = _62;
    bool _87 = false;
    if (ok)
    {
        float2x2 _74 = float2x2(float2(_10_testInputs.xy), float2(_10_testInputs.zw));
        float2x2 _77 = float2x2(float2(-1.25f, 0.0f), float2(0.75f, 2.25f));
        float2 _78 = _74[0];
        float2 _79 = _77[0];
        float2 _82 = _74[1];
        float2 _83 = _77[1];
        _87 = all(bool2(_78.x == _79.x, _78.y == _79.y)) && all(bool2(_82.x == _83.x, _82.y == _83.y));
    }
    else
    {
        _87 = false;
    }
    ok = _87;
    bool _114 = false;
    if (ok)
    {
        float2x2 _100 = float2x2(float2(_10_colorGreen.xy), float2(_10_colorGreen.zw));
        float2x2 _104 = float2x2(float2(0.0f, 1.0f), float2(0.0f, 1.0f));
        float2 _105 = _100[0];
        float2 _106 = _104[0];
        float2 _109 = _100[1];
        float2 _110 = _104[1];
        _114 = all(bool2(_105.x == _106.x, _105.y == _106.y)) && all(bool2(_109.x == _110.x, _109.y == _110.y));
    }
    else
    {
        _114 = false;
    }
    ok = _114;
    bool _139 = false;
    if (ok)
    {
        float2x2 _126 = float2x2(float2(_10_colorGreen.xy), float2(_10_colorGreen.zw));
        float2x2 _129 = float2x2(float2(0.0f, 1.0f), float2(0.0f, 1.0f));
        float2 _130 = _126[0];
        float2 _131 = _129[0];
        float2 _134 = _126[1];
        float2 _135 = _129[1];
        _139 = all(bool2(_130.x == _131.x, _130.y == _131.y)) && all(bool2(_134.x == _135.x, _134.y == _135.y));
    }
    else
    {
        _139 = false;
    }
    ok = _139;
    bool _183 = false;
    if (ok)
    {
        int4 _153 = int4(int(_10_colorGreen.x), int(_10_colorGreen.y), int(_10_colorGreen.z), int(_10_colorGreen.w));
        float4 _163 = float4(float(_153.x), float(_153.y), float(_153.z), float(_153.w));
        float2x2 _170 = float2x2(float2(_163.xy), float2(_163.zw));
        float2x2 _173 = float2x2(float2(0.0f, 1.0f), float2(0.0f, 1.0f));
        float2 _174 = _170[0];
        float2 _175 = _173[0];
        float2 _178 = _170[1];
        float2 _179 = _173[1];
        _183 = all(bool2(_174.x == _175.x, _174.y == _175.y)) && all(bool2(_178.x == _179.x, _178.y == _179.y));
    }
    else
    {
        _183 = false;
    }
    ok = _183;
    bool _208 = false;
    if (ok)
    {
        float2x2 _195 = float2x2(float2(_10_colorGreen.xy), float2(_10_colorGreen.zw));
        float2x2 _198 = float2x2(float2(0.0f, 1.0f), float2(0.0f, 1.0f));
        float2 _199 = _195[0];
        float2 _200 = _198[0];
        float2 _203 = _195[1];
        float2 _204 = _198[1];
        _208 = all(bool2(_199.x == _200.x, _199.y == _200.y)) && all(bool2(_203.x == _204.x, _203.y == _204.y));
    }
    else
    {
        _208 = false;
    }
    ok = _208;
    bool _233 = false;
    if (ok)
    {
        float2x2 _220 = float2x2(float2(_10_colorGreen.xy), float2(_10_colorGreen.zw));
        float2x2 _223 = float2x2(float2(0.0f, 1.0f), float2(0.0f, 1.0f));
        float2 _224 = _220[0];
        float2 _225 = _223[0];
        float2 _228 = _220[1];
        float2 _229 = _223[1];
        _233 = all(bool2(_224.x == _225.x, _224.y == _225.y)) && all(bool2(_228.x == _229.x, _228.y == _229.y));
    }
    else
    {
        _233 = false;
    }
    ok = _233;
    bool _277 = false;
    if (ok)
    {
        bool4 _247 = bool4(_10_colorGreen.x != 0.0f, _10_colorGreen.y != 0.0f, _10_colorGreen.z != 0.0f, _10_colorGreen.w != 0.0f);
        float4 _257 = float4(float(_247.x), float(_247.y), float(_247.z), float(_247.w));
        float2x2 _264 = float2x2(float2(_257.xy), float2(_257.zw));
        float2x2 _267 = float2x2(float2(0.0f, 1.0f), float2(0.0f, 1.0f));
        float2 _268 = _264[0];
        float2 _269 = _267[0];
        float2 _272 = _264[1];
        float2 _273 = _267[1];
        _277 = all(bool2(_268.x == _269.x, _268.y == _269.y)) && all(bool2(_272.x == _273.x, _272.y == _273.y));
    }
    else
    {
        _277 = false;
    }
    ok = _277;
    float4 _279 = 0.0f.xxxx;
    if (ok)
    {
        _279 = _10_colorGreen;
    }
    else
    {
        _279 = _10_colorRed;
    }
    return _279;
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
