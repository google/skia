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
    int4 expectedB = int4(1065353216, 1073741824, -1069547520, -1065353216);
    bool _64 = false;
    if (_42.x == asfloat(1065353216))
    {
        float2 _57 = _42.xy;
        float2 _58 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216).xy);
        _64 = all(bool2(_57.x == _58.x, _57.y == _58.y));
    }
    else
    {
        _64 = false;
    }
    bool _75 = false;
    if (_64)
    {
        float3 _67 = _42.xyz;
        float3 _69 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216).xyz);
        _75 = all(bool3(_67.x == _69.x, _67.y == _69.y, _67.z == _69.z));
    }
    else
    {
        _75 = false;
    }
    bool _82 = false;
    if (_75)
    {
        float4 _78 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216));
        _82 = all(bool4(_42.x == _78.x, _42.y == _78.y, _42.z == _78.z, _42.w == _78.w));
    }
    else
    {
        _82 = false;
    }
    float4 _83 = 0.0f.xxxx;
    if (_82)
    {
        _83 = _10_colorGreen;
    }
    else
    {
        _83 = _10_colorRed;
    }
    return _83;
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
