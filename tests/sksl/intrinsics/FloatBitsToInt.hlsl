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
    bool _58 = false;
    if (asint(_42.x) == 1065353216)
    {
        int2 _50 = asint(_42.xy);
        _58 = all(bool2(_50.x == int2(1065353216, 1073741824).x, _50.y == int2(1065353216, 1073741824).y));
    }
    else
    {
        _58 = false;
    }
    bool _70 = false;
    if (_58)
    {
        int3 _61 = asint(_42.xyz);
        _70 = all(bool3(_61.x == int3(1065353216, 1073741824, -1069547520).x, _61.y == int3(1065353216, 1073741824, -1069547520).y, _61.z == int3(1065353216, 1073741824, -1069547520).z));
    }
    else
    {
        _70 = false;
    }
    bool _80 = false;
    if (_70)
    {
        int4 _73 = asint(_42);
        _80 = all(bool4(_73.x == int4(1065353216, 1073741824, -1069547520, -1065353216).x, _73.y == int4(1065353216, 1073741824, -1069547520, -1065353216).y, _73.z == int4(1065353216, 1073741824, -1069547520, -1065353216).z, _73.w == int4(1065353216, 1073741824, -1069547520, -1065353216).w));
    }
    else
    {
        _80 = false;
    }
    float4 _81 = 0.0f.xxxx;
    if (_80)
    {
        _81 = _10_colorGreen;
    }
    else
    {
        _81 = _10_colorRed;
    }
    return _81;
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
