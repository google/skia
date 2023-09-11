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
    int4 expectedB = int4(1065353216, 1073741824, -1069547520, -1065353216);
    bool _62 = false;
    if (_39.x == asfloat(1065353216))
    {
        float2 _55 = _39.xy;
        float2 _56 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216).xy);
        _62 = all(bool2(_55.x == _56.x, _55.y == _56.y));
    }
    else
    {
        _62 = false;
    }
    bool _73 = false;
    if (_62)
    {
        float3 _65 = _39.xyz;
        float3 _67 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216).xyz);
        _73 = all(bool3(_65.x == _67.x, _65.y == _67.y, _65.z == _67.z));
    }
    else
    {
        _73 = false;
    }
    bool _80 = false;
    if (_73)
    {
        float4 _76 = asfloat(int4(1065353216, 1073741824, -1069547520, -1065353216));
        _80 = all(bool4(_39.x == _76.x, _39.y == _76.y, _39.z == _76.z, _39.w == _76.w));
    }
    else
    {
        _80 = false;
    }
    float4 _81 = 0.0f.xxxx;
    if (_80)
    {
        _81 = _7_colorGreen;
    }
    else
    {
        _81 = _7_colorRed;
    }
    return _81;
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
