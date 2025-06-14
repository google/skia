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
    int4 expectedB = int4(1065353216, 1073741824, -1069547520, -1065353216);
    bool _65 = false;
    if (_42.x == asfloat(1065353216))
    {
        float2 _58 = _42.xy;
        float2 _59 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216).xy);
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
        float3 _70 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216).xyz);
        _76 = all(bool3(_68.x == _70.x, _68.y == _70.y, _68.z == _70.z));
    }
    else
    {
        _76 = false;
    }
    bool _83 = false;
    if (_76)
    {
        float4 _79 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216));
        _83 = all(bool4(_42.x == _79.x, _42.y == _79.y, _42.z == _79.z, _42.w == _79.w));
    }
    else
    {
        _83 = false;
    }
    float4 _84 = 0.0f.xxxx;
    if (_83)
    {
        _84 = _11_colorGreen;
    }
    else
    {
        _84 = _11_colorRed;
    }
    return _84;
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
