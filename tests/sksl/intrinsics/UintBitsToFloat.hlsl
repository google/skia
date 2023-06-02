cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _10_testMatrix2x2 : packoffset(c0);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
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
    bool4 _84 = _83.xxxx;
    return float4(_84.x ? _10_colorGreen.x : _10_colorRed.x, _84.y ? _10_colorGreen.y : _10_colorRed.y, _84.z ? _10_colorGreen.z : _10_colorRed.z, _84.w ? _10_colorGreen.w : _10_colorRed.w);
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
