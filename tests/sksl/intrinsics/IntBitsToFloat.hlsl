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
    int4 expectedB = int4(1065353216, 1073741824, -1069547520, -1065353216);
    bool _69 = false;
    if (inputVal.x == asfloat(expectedB.x))
    {
        float2 _62 = asfloat(expectedB.xy);
        _69 = all(bool2(inputVal.xy.x == _62.x, inputVal.xy.y == _62.y));
    }
    else
    {
        _69 = false;
    }
    bool _82 = false;
    if (_69)
    {
        float3 _75 = asfloat(expectedB.xyz);
        _82 = all(bool3(inputVal.xyz.x == _75.x, inputVal.xyz.y == _75.y, inputVal.xyz.z == _75.z));
    }
    else
    {
        _82 = false;
    }
    bool _91 = false;
    if (_82)
    {
        float4 _86 = asfloat(expectedB);
        _91 = all(bool4(inputVal.x == _86.x, inputVal.y == _86.y, inputVal.z == _86.z, inputVal.w == _86.w));
    }
    else
    {
        _91 = false;
    }
    float4 _92 = 0.0f.xxxx;
    if (_91)
    {
        _92 = _10_colorGreen;
    }
    else
    {
        _92 = _10_colorRed;
    }
    return _92;
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
