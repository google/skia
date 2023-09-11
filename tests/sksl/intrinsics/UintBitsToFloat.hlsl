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
    uint4 expectedB = uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u);
    bool _63 = false;
    if (_39.x == asfloat(1065353216u))
    {
        float2 _56 = _39.xy;
        float2 _57 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).xy);
        _63 = all(bool2(_56.x == _57.x, _56.y == _57.y));
    }
    else
    {
        _63 = false;
    }
    bool _74 = false;
    if (_63)
    {
        float3 _66 = _39.xyz;
        float3 _68 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).xyz);
        _74 = all(bool3(_66.x == _68.x, _66.y == _68.y, _66.z == _68.z));
    }
    else
    {
        _74 = false;
    }
    bool _81 = false;
    if (_74)
    {
        float4 _77 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u));
        _81 = all(bool4(_39.x == _77.x, _39.y == _77.y, _39.z == _77.z, _39.w == _77.w));
    }
    else
    {
        _81 = false;
    }
    float4 _82 = 0.0f.xxxx;
    if (_81)
    {
        _82 = _7_colorGreen;
    }
    else
    {
        _82 = _7_colorRed;
    }
    return _82;
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
