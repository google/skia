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
    bool _60 = false;
    if (asuint(_42.x) == 1065353216u)
    {
        uint2 _52 = asuint(_42.xy);
        _60 = all(bool2(_52.x == uint2(1065353216u, 1073741824u).x, _52.y == uint2(1065353216u, 1073741824u).y));
    }
    else
    {
        _60 = false;
    }
    bool _72 = false;
    if (_60)
    {
        uint3 _63 = asuint(_42.xyz);
        _72 = all(bool3(_63.x == uint3(1065353216u, 1073741824u, 3225419776u).x, _63.y == uint3(1065353216u, 1073741824u, 3225419776u).y, _63.z == uint3(1065353216u, 1073741824u, 3225419776u).z));
    }
    else
    {
        _72 = false;
    }
    bool _82 = false;
    if (_72)
    {
        uint4 _75 = asuint(_42);
        _82 = all(bool4(_75.x == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).x, _75.y == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).y, _75.z == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).z, _75.w == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).w));
    }
    else
    {
        _82 = false;
    }
    float4 _83 = 0.0f.xxxx;
    if (_82)
    {
        _83 = _11_colorGreen;
    }
    else
    {
        _83 = _11_colorRed;
    }
    return _83;
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
