cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _7_testMatrix2x2 : packoffset(c0);
    float4 _7_colorGreen : packoffset(c2);
    float4 _7_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    float4 _39 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y) * float4(1.0f, 1.0f, -1.0f, -1.0f);
    float4 inputVal = _39;
    bool _57 = false;
    if (asuint(_39.x) == 1065353216u)
    {
        uint2 _49 = asuint(_39.xy);
        _57 = all(bool2(_49.x == uint2(1065353216u, 1073741824u).x, _49.y == uint2(1065353216u, 1073741824u).y));
    }
    else
    {
        _57 = false;
    }
    bool _69 = false;
    if (_57)
    {
        uint3 _60 = asuint(_39.xyz);
        _69 = all(bool3(_60.x == uint3(1065353216u, 1073741824u, 3225419776u).x, _60.y == uint3(1065353216u, 1073741824u, 3225419776u).y, _60.z == uint3(1065353216u, 1073741824u, 3225419776u).z));
    }
    else
    {
        _69 = false;
    }
    bool _79 = false;
    if (_69)
    {
        uint4 _72 = asuint(_39);
        _79 = all(bool4(_72.x == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).x, _72.y == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).y, _72.z == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).z, _72.w == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).w));
    }
    else
    {
        _79 = false;
    }
    float4 _80 = 0.0f.xxxx;
    if (_79)
    {
        _80 = _7_colorGreen;
    }
    else
    {
        _80 = _7_colorRed;
    }
    return _80;
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
