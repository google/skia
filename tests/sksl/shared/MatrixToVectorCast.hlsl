cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    bool ok = true;
    bool _49 = false;
    if (true)
    {
        float4 _40 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y);
        _49 = all(bool4(_40.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _40.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _40.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _40.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _49 = false;
    }
    ok = _49;
    bool _61 = false;
    if (_49)
    {
        float4 _58 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y);
        _61 = all(bool4(_58.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _58.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _58.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _58.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _61 = false;
    }
    ok = _61;
    bool _83 = false;
    if (_61)
    {
        int4 _76 = int4(int(_7_testMatrix2x2[0].x), int(_7_testMatrix2x2[0].y), int(_7_testMatrix2x2[1].x), int(_7_testMatrix2x2[1].y));
        _83 = all(bool4(_76.x == int4(1, 2, 3, 4).x, _76.y == int4(1, 2, 3, 4).y, _76.z == int4(1, 2, 3, 4).z, _76.w == int4(1, 2, 3, 4).w));
    }
    else
    {
        _83 = false;
    }
    ok = _83;
    bool _101 = false;
    if (_83)
    {
        bool4 _97 = bool4(_7_testMatrix2x2[0].x != 0.0f, _7_testMatrix2x2[0].y != 0.0f, _7_testMatrix2x2[1].x != 0.0f, _7_testMatrix2x2[1].y != 0.0f);
        _101 = all(bool4(_97.x == bool4(true, true, true, true).x, _97.y == bool4(true, true, true, true).y, _97.z == bool4(true, true, true, true).z, _97.w == bool4(true, true, true, true).w));
    }
    else
    {
        _101 = false;
    }
    ok = _101;
    float4 _102 = 0.0f.xxxx;
    if (_101)
    {
        _102 = _7_colorGreen;
    }
    else
    {
        _102 = _7_colorRed;
    }
    return _102;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
