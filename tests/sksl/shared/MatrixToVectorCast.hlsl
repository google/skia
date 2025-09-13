cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    row_major float2x2 _11_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    bool ok = true;
    bool _52 = false;
    if (true)
    {
        float4 _43 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y);
        _52 = all(bool4(_43.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _43.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _43.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _43.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _52 = false;
    }
    ok = _52;
    bool _64 = false;
    if (_52)
    {
        float4 _61 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y);
        _64 = all(bool4(_61.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _61.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _61.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _61.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _64 = false;
    }
    ok = _64;
    bool _86 = false;
    if (_64)
    {
        int4 _79 = int4(int(_11_testMatrix2x2[0].x), int(_11_testMatrix2x2[0].y), int(_11_testMatrix2x2[1].x), int(_11_testMatrix2x2[1].y));
        _86 = all(bool4(_79.x == int4(1, 2, 3, 4).x, _79.y == int4(1, 2, 3, 4).y, _79.z == int4(1, 2, 3, 4).z, _79.w == int4(1, 2, 3, 4).w));
    }
    else
    {
        _86 = false;
    }
    ok = _86;
    bool _104 = false;
    if (_86)
    {
        bool4 _100 = bool4(_11_testMatrix2x2[0].x != 0.0f, _11_testMatrix2x2[0].y != 0.0f, _11_testMatrix2x2[1].x != 0.0f, _11_testMatrix2x2[1].y != 0.0f);
        _104 = all(bool4(_100.x == bool4(true, true, true, true).x, _100.y == bool4(true, true, true, true).y, _100.z == bool4(true, true, true, true).z, _100.w == bool4(true, true, true, true).w));
    }
    else
    {
        _104 = false;
    }
    ok = _104;
    float4 _105 = 0.0f.xxxx;
    if (_104)
    {
        _105 = _11_colorGreen;
    }
    else
    {
        _105 = _11_colorRed;
    }
    return _105;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
