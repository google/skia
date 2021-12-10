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
    float4 inputVal = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * float4(1.0f, 1.0f, -1.0f, -1.0f);
    uint4 expectedB = uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u);
    bool _67 = false;
    if (asuint(inputVal.x) == 1065353216u)
    {
        uint2 _59 = asuint(inputVal.xy);
        _67 = all(bool2(_59.x == uint2(1065353216u, 1073741824u).x, _59.y == uint2(1065353216u, 1073741824u).y));
    }
    else
    {
        _67 = false;
    }
    bool _79 = false;
    if (_67)
    {
        uint3 _70 = asuint(inputVal.xyz);
        _79 = all(bool3(_70.x == uint3(1065353216u, 1073741824u, 3225419776u).x, _70.y == uint3(1065353216u, 1073741824u, 3225419776u).y, _70.z == uint3(1065353216u, 1073741824u, 3225419776u).z));
    }
    else
    {
        _79 = false;
    }
    bool _88 = false;
    if (_79)
    {
        uint4 _82 = asuint(inputVal);
        _88 = all(bool4(_82.x == expectedB.x, _82.y == expectedB.y, _82.z == expectedB.z, _82.w == expectedB.w));
    }
    else
    {
        _88 = false;
    }
    float4 _89 = 0.0f.xxxx;
    if (_88)
    {
        _89 = _10_colorGreen;
    }
    else
    {
        _89 = _10_colorRed;
    }
    return _89;
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
