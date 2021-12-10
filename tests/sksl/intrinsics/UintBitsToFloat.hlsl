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
    bool _70 = false;
    if (inputVal.x == asfloat(expectedB.x))
    {
        float2 _63 = asfloat(expectedB.xy);
        _70 = all(bool2(inputVal.xy.x == _63.x, inputVal.xy.y == _63.y));
    }
    else
    {
        _70 = false;
    }
    bool _83 = false;
    if (_70)
    {
        float3 _76 = asfloat(expectedB.xyz);
        _83 = all(bool3(inputVal.xyz.x == _76.x, inputVal.xyz.y == _76.y, inputVal.xyz.z == _76.z));
    }
    else
    {
        _83 = false;
    }
    bool _92 = false;
    if (_83)
    {
        float4 _87 = asfloat(expectedB);
        _92 = all(bool4(inputVal.x == _87.x, inputVal.y == _87.y, inputVal.z == _87.z, inputVal.w == _87.w));
    }
    else
    {
        _92 = false;
    }
    float4 _93 = 0.0f.xxxx;
    if (_92)
    {
        _93 = _10_colorGreen;
    }
    else
    {
        _93 = _10_colorRed;
    }
    return _93;
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
