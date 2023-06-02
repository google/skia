cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool ok = true;
    bool _51 = false;
    if (true)
    {
        float4 _42 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
        _51 = all(bool4(_42.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _42.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _42.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _42.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _51 = false;
    }
    ok = _51;
    bool _63 = false;
    if (_51)
    {
        float4 _60 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
        _63 = all(bool4(_60.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _60.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _60.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _60.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _63 = false;
    }
    ok = _63;
    bool _85 = false;
    if (_63)
    {
        int4 _78 = int4(int(_10_testMatrix2x2[0].x), int(_10_testMatrix2x2[0].y), int(_10_testMatrix2x2[1].x), int(_10_testMatrix2x2[1].y));
        _85 = all(bool4(_78.x == int4(1, 2, 3, 4).x, _78.y == int4(1, 2, 3, 4).y, _78.z == int4(1, 2, 3, 4).z, _78.w == int4(1, 2, 3, 4).w));
    }
    else
    {
        _85 = false;
    }
    ok = _85;
    bool _103 = false;
    if (_85)
    {
        bool4 _99 = bool4(_10_testMatrix2x2[0].x != 0.0f, _10_testMatrix2x2[0].y != 0.0f, _10_testMatrix2x2[1].x != 0.0f, _10_testMatrix2x2[1].y != 0.0f);
        _103 = all(bool4(_99.x == bool4(true, true, true, true).x, _99.y == bool4(true, true, true, true).y, _99.z == bool4(true, true, true, true).z, _99.w == bool4(true, true, true, true).w));
    }
    else
    {
        _103 = false;
    }
    ok = _103;
    float4 _104 = 0.0f.xxxx;
    if (_103)
    {
        _104 = _10_colorGreen;
    }
    else
    {
        _104 = _10_colorRed;
    }
    return _104;
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
