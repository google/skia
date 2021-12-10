cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_testInput : packoffset(c0);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c1);
    float4 _10_colorGreen : packoffset(c3);
    float4 _10_colorRed : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _42 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * float4(1.0f, 1.0f, -1.0f, -1.0f);
    float4 inputVal = _42;
    bool _59 = false;
    if (asuint(_42.x) == 1065353216u)
    {
        uint2 _51 = asuint(_42.xy);
        _59 = all(bool2(_51.x == uint2(1065353216u, 1073741824u).x, _51.y == uint2(1065353216u, 1073741824u).y));
    }
    else
    {
        _59 = false;
    }
    bool _71 = false;
    if (_59)
    {
        uint3 _62 = asuint(_42.xyz);
        _71 = all(bool3(_62.x == uint3(1065353216u, 1073741824u, 3225419776u).x, _62.y == uint3(1065353216u, 1073741824u, 3225419776u).y, _62.z == uint3(1065353216u, 1073741824u, 3225419776u).z));
    }
    else
    {
        _71 = false;
    }
    bool _81 = false;
    if (_71)
    {
        uint4 _74 = asuint(_42);
        _81 = all(bool4(_74.x == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).x, _74.y == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).y, _74.z == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).z, _74.w == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).w));
    }
    else
    {
        _81 = false;
    }
    float4 _82 = 0.0f.xxxx;
    if (_81)
    {
        _82 = _10_colorGreen;
    }
    else
    {
        _82 = _10_colorRed;
    }
    return _82;
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
