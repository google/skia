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
    bool _66 = false;
    if (asint(inputVal.x) == 1065353216)
    {
        int2 _58 = asint(inputVal.xy);
        _66 = all(bool2(_58.x == int2(1065353216, 1073741824).x, _58.y == int2(1065353216, 1073741824).y));
    }
    else
    {
        _66 = false;
    }
    bool _78 = false;
    if (_66)
    {
        int3 _69 = asint(inputVal.xyz);
        _78 = all(bool3(_69.x == int3(1065353216, 1073741824, -1069547520).x, _69.y == int3(1065353216, 1073741824, -1069547520).y, _69.z == int3(1065353216, 1073741824, -1069547520).z));
    }
    else
    {
        _78 = false;
    }
    bool _87 = false;
    if (_78)
    {
        int4 _81 = asint(inputVal);
        _87 = all(bool4(_81.x == expectedB.x, _81.y == expectedB.y, _81.z == expectedB.z, _81.w == expectedB.w));
    }
    else
    {
        _87 = false;
    }
    float4 _88 = 0.0f.xxxx;
    if (_87)
    {
        _88 = _10_colorGreen;
    }
    else
    {
        _88 = _10_colorRed;
    }
    return _88;
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
