cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float _35[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float f[4] = _35;
    float h[4] = f;
    f = h;
    h = f;
    int3 _51[3] = { int3(1, 1, 1), int3(2, 2, 2), int3(3, 3, 3) };
    int3 i3[3] = _51;
    int3 s3[3] = i3;
    i3 = s3;
    s3 = i3;
    float2x2 _71[2] = { float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)), float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f)) };
    float2x2 h2x2[2] = _71;
    float2x2 f2x2[2] = h2x2;
    f2x2 = h2x2;
    h2x2 = f2x2;
    bool _114 = false;
    if ((f[3] == h[3]) && ((f[2] == h[2]) && ((f[1] == h[1]) && (f[0] == h[0]))))
    {
        _114 = all(bool3(i3[2].x == s3[2].x, i3[2].y == s3[2].y, i3[2].z == s3[2].z)) && (all(bool3(i3[1].x == s3[1].x, i3[1].y == s3[1].y, i3[1].z == s3[1].z)) && all(bool3(i3[0].x == s3[0].x, i3[0].y == s3[0].y, i3[0].z == s3[0].z)));
    }
    else
    {
        _114 = false;
    }
    bool _143 = false;
    if (_114)
    {
        _143 = (all(bool2(f2x2[1][0].x == h2x2[1][0].x, f2x2[1][0].y == h2x2[1][0].y)) && all(bool2(f2x2[1][1].x == h2x2[1][1].x, f2x2[1][1].y == h2x2[1][1].y))) && (all(bool2(f2x2[0][0].x == h2x2[0][0].x, f2x2[0][0].y == h2x2[0][0].y)) && all(bool2(f2x2[0][1].x == h2x2[0][1].x, f2x2[0][1].y == h2x2[0][1].y)));
    }
    else
    {
        _143 = false;
    }
    float4 _144 = 0.0f.xxxx;
    if (_143)
    {
        _144 = _10_colorGreen;
    }
    else
    {
        _144 = _10_colorRed;
    }
    return _144;
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
