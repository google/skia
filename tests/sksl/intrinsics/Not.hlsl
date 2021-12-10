cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputH4 : packoffset(c0);
    float4 _10_expectedH4 : packoffset(c1);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool4 inputVal = bool4(_10_inputH4.x != 0.0f, _10_inputH4.y != 0.0f, _10_inputH4.z != 0.0f, _10_inputH4.w != 0.0f);
    bool4 expected = bool4(_10_expectedH4.x != 0.0f, _10_expectedH4.y != 0.0f, _10_expectedH4.z != 0.0f, _10_expectedH4.w != 0.0f);
    bool2 _57 = bool2(!inputVal.xy.x, !inputVal.xy.y);
    bool _75 = false;
    if (all(bool2(_57.x == expected.xy.x, _57.y == expected.xy.y)))
    {
        bool3 _67 = bool3(!inputVal.xyz.x, !inputVal.xyz.y, !inputVal.xyz.z);
        _75 = all(bool3(_67.x == expected.xyz.x, _67.y == expected.xyz.y, _67.z == expected.xyz.z));
    }
    else
    {
        _75 = false;
    }
    bool _83 = false;
    if (_75)
    {
        bool4 _78 = bool4(!inputVal.x, !inputVal.y, !inputVal.z, !inputVal.w);
        _83 = all(bool4(_78.x == expected.x, _78.y == expected.y, _78.z == expected.z, _78.w == expected.w));
    }
    else
    {
        _83 = false;
    }
    bool _92 = false;
    if (_83)
    {
        _92 = all(bool2(bool2(false, true).x == expected.xy.x, bool2(false, true).y == expected.xy.y));
    }
    else
    {
        _92 = false;
    }
    bool _100 = false;
    if (_92)
    {
        _100 = all(bool3(bool3(false, true, false).x == expected.xyz.x, bool3(false, true, false).y == expected.xyz.y, bool3(false, true, false).z == expected.xyz.z));
    }
    else
    {
        _100 = false;
    }
    bool _107 = false;
    if (_100)
    {
        _107 = all(bool4(bool4(false, true, false, true).x == expected.x, bool4(false, true, false, true).y == expected.y, bool4(false, true, false, true).z == expected.z, bool4(false, true, false, true).w == expected.w));
    }
    else
    {
        _107 = false;
    }
    float4 _108 = 0.0f.xxxx;
    if (_107)
    {
        _108 = _10_colorGreen;
    }
    else
    {
        _108 = _10_colorRed;
    }
    return _108;
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
