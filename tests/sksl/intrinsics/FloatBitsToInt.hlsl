cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _11_testMatrix2x2 : packoffset(c0);
    float4 _11_colorGreen : packoffset(c2);
    float4 _11_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    float4 _42 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y) * float4(1.0f, 1.0f, -1.0f, -1.0f);
    float4 inputVal = _42;
    bool _59 = false;
    if (asint(_42.x) == 1065353216)
    {
        int2 _51 = asint(_42.xy);
        _59 = all(bool2(_51.x == int2(1065353216, 1073741824).x, _51.y == int2(1065353216, 1073741824).y));
    }
    else
    {
        _59 = false;
    }
    bool _71 = false;
    if (_59)
    {
        int3 _62 = asint(_42.xyz);
        _71 = all(bool3(_62.x == int3(1065353216, 1073741824, -1069547520).x, _62.y == int3(1065353216, 1073741824, -1069547520).y, _62.z == int3(1065353216, 1073741824, -1069547520).z));
    }
    else
    {
        _71 = false;
    }
    bool _81 = false;
    if (_71)
    {
        int4 _74 = asint(_42);
        _81 = all(bool4(_74.x == int4(1065353216, 1073741824, -1069547520, -1065353216).x, _74.y == int4(1065353216, 1073741824, -1069547520, -1065353216).y, _74.z == int4(1065353216, 1073741824, -1069547520, -1065353216).z, _74.w == int4(1065353216, 1073741824, -1069547520, -1065353216).w));
    }
    else
    {
        _81 = false;
    }
    float4 _82 = 0.0f.xxxx;
    if (_81)
    {
        _82 = _11_colorGreen;
    }
    else
    {
        _82 = _11_colorRed;
    }
    return _82;
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
