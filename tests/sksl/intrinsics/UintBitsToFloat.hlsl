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
    uint4 expectedB = uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u);
    bool _66 = false;
    if (_42.x == asfloat(1065353216u))
    {
        float2 _59 = _42.xy;
        float2 _60 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).xy);
        _66 = all(bool2(_59.x == _60.x, _59.y == _60.y));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float3 _69 = _42.xyz;
        float3 _71 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).xyz);
        _77 = all(bool3(_69.x == _71.x, _69.y == _71.y, _69.z == _71.z));
    }
    else
    {
        _77 = false;
    }
    bool _84 = false;
    if (_77)
    {
        float4 _80 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u));
        _84 = all(bool4(_42.x == _80.x, _42.y == _80.y, _42.z == _80.z, _42.w == _80.w));
    }
    else
    {
        _84 = false;
    }
    float4 _85 = 0.0f.xxxx;
    if (_84)
    {
        _85 = _11_colorGreen;
    }
    else
    {
        _85 = _11_colorRed;
    }
    return _85;
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
