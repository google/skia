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
    bool _56 = false;
    if (asint(_39.x) == 1065353216)
    {
        int2 _48 = asint(_39.xy);
        _56 = all(bool2(_48.x == int2(1065353216, 1073741824).x, _48.y == int2(1065353216, 1073741824).y));
    }
    else
    {
        _56 = false;
    }
    bool _68 = false;
    if (_56)
    {
        int3 _59 = asint(_39.xyz);
        _68 = all(bool3(_59.x == int3(1065353216, 1073741824, -1069547520).x, _59.y == int3(1065353216, 1073741824, -1069547520).y, _59.z == int3(1065353216, 1073741824, -1069547520).z));
    }
    else
    {
        _68 = false;
    }
    bool _78 = false;
    if (_68)
    {
        int4 _71 = asint(_39);
        _78 = all(bool4(_71.x == int4(1065353216, 1073741824, -1069547520, -1065353216).x, _71.y == int4(1065353216, 1073741824, -1069547520, -1065353216).y, _71.z == int4(1065353216, 1073741824, -1069547520, -1065353216).z, _71.w == int4(1065353216, 1073741824, -1069547520, -1065353216).w));
    }
    else
    {
        _78 = false;
    }
    float4 _79 = 0.0f.xxxx;
    if (_78)
    {
        _79 = _7_colorGreen;
    }
    else
    {
        _79 = _7_colorRed;
    }
    return _79;
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
