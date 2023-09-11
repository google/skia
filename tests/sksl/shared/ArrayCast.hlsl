cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float _32[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float f[4] = _32;
    float h[4] = _32;
    f = _32;
    h = _32;
    int3 _44[3] = { int3(1, 1, 1), int3(2, 2, 2), int3(3, 3, 3) };
    int3 i3[3] = _44;
    int3 s3[3] = _44;
    i3 = _44;
    s3 = _44;
    float2x2 _60[2] = { float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)), float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f)) };
    float2x2 h2x2[2] = _60;
    float2x2 f2x2[2] = _60;
    f2x2 = _60;
    h2x2 = _60;
    bool _72 = false;
    if (true && (true && (true && true)))
    {
        _72 = true && (true && true);
    }
    else
    {
        _72 = false;
    }
    bool _87 = false;
    if (_72)
    {
        _87 = (all(bool2(float2(5.0f, 6.0f).x == float2(5.0f, 6.0f).x, float2(5.0f, 6.0f).y == float2(5.0f, 6.0f).y)) && all(bool2(float2(7.0f, 8.0f).x == float2(7.0f, 8.0f).x, float2(7.0f, 8.0f).y == float2(7.0f, 8.0f).y))) && (all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y)));
    }
    else
    {
        _87 = false;
    }
    float4 _88 = 0.0f.xxxx;
    if (_87)
    {
        _88 = _7_colorGreen;
    }
    else
    {
        _88 = _7_colorRed;
    }
    return _88;
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
