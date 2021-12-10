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
    uint4 expectedB = uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u);
    bool _65 = false;
    if (_42.x == asfloat(1065353216u))
    {
        float2 _58 = _42.xy;
        float2 _59 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).xy);
        _65 = all(bool2(_58.x == _59.x, _58.y == _59.y));
    }
    else
    {
        _65 = false;
    }
    bool _76 = false;
    if (_65)
    {
        float3 _68 = _42.xyz;
        float3 _70 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u).xyz);
        _76 = all(bool3(_68.x == _70.x, _68.y == _70.y, _68.z == _70.z));
    }
    else
    {
        _76 = false;
    }
    bool _83 = false;
    if (_76)
    {
        float4 _79 = asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u));
        _83 = all(bool4(_42.x == _79.x, _42.y == _79.y, _42.z == _79.z, _42.w == _79.w));
    }
    else
    {
        _83 = false;
    }
    float4 _84 = 0.0f.xxxx;
    if (_83)
    {
        _84 = _10_colorGreen;
    }
    else
    {
        _84 = _10_colorRed;
    }
    return _84;
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
