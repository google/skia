cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float _35[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float f[4] = _35;
    float h[4] = _35;
    f = _35;
    h = _35;
    int3 _47[3] = { int3(1, 1, 1), int3(2, 2, 2), int3(3, 3, 3) };
    int3 i3[3] = _47;
    int3 s3[3] = _47;
    i3 = _47;
    s3 = _47;
    float2x2 _63[2] = { float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)), float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f)) };
    float2x2 h2x2[2] = _63;
    float2x2 f2x2[2] = _63;
    f2x2 = _63;
    h2x2 = _63;
    bool _75 = false;
    if (true && (true && (true && true)))
    {
        _75 = true && (true && true);
    }
    else
    {
        _75 = false;
    }
    bool _90 = false;
    if (_75)
    {
        _90 = (all(bool2(float2(5.0f, 6.0f).x == float2(5.0f, 6.0f).x, float2(5.0f, 6.0f).y == float2(5.0f, 6.0f).y)) && all(bool2(float2(7.0f, 8.0f).x == float2(7.0f, 8.0f).x, float2(7.0f, 8.0f).y == float2(7.0f, 8.0f).y))) && (all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y)));
    }
    else
    {
        _90 = false;
    }
    float4 _91 = 0.0f.xxxx;
    if (_90)
    {
        _91 = _11_colorGreen;
    }
    else
    {
        _91 = _11_colorRed;
    }
    return _91;
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
