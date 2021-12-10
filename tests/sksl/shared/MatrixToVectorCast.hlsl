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
    bool _52 = false;
    if (ok)
    {
        float4 _43 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
        _52 = all(bool4(_43.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _43.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _43.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _43.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _52 = false;
    }
    ok = _52;
    bool _65 = false;
    if (ok)
    {
        float4 _62 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
        _65 = all(bool4(_62.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _62.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _62.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _62.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _65 = false;
    }
    ok = _65;
    bool _92 = false;
    if (ok)
    {
        float4 _75 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
        int4 _84 = int4(int(_75.x), int(_75.y), int(_75.z), int(_75.w));
        _92 = all(bool4(_84.x == int4(1, 2, 3, 4).x, _84.y == int4(1, 2, 3, 4).y, _84.z == int4(1, 2, 3, 4).z, _84.w == int4(1, 2, 3, 4).w));
    }
    else
    {
        _92 = false;
    }
    ok = _92;
    bool _115 = false;
    if (ok)
    {
        float4 _102 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
        bool4 _111 = bool4(_102.x != 0.0f, _102.y != 0.0f, _102.z != 0.0f, _102.w != 0.0f);
        _115 = all(bool4(_111.x == bool4(true, true, true, true).x, _111.y == bool4(true, true, true, true).y, _111.z == bool4(true, true, true, true).z, _111.w == bool4(true, true, true, true).w));
    }
    else
    {
        _115 = false;
    }
    ok = _115;
    float4 _117 = 0.0f.xxxx;
    if (ok)
    {
        _117 = _10_colorGreen;
    }
    else
    {
        _117 = _10_colorRed;
    }
    return _117;
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
