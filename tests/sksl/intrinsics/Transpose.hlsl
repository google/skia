cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _7_testMatrix2x2 : packoffset(c0);
    row_major float3x3 _7_testMatrix3x3 : packoffset(c2);
    float4 _7_colorGreen : packoffset(c5);
    float4 _7_colorRed : packoffset(c6);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float2x3 testMatrix2x3 = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
    float2x2 _40 = transpose(_7_testMatrix2x2);
    float2 _50 = _40[0];
    float2 _53 = _40[1];
    bool _76 = false;
    if (all(bool2(_50.x == float2(1.0f, 3.0f).x, _50.y == float2(1.0f, 3.0f).y)) && all(bool2(_53.x == float2(2.0f, 4.0f).x, _53.y == float2(2.0f, 4.0f).y)))
    {
        float3x2 _59 = transpose(float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f)));
        float2 _65 = _59[0];
        float2 _68 = _59[1];
        float2 _72 = _59[2];
        _76 = (all(bool2(_65.x == float2(1.0f, 4.0f).x, _65.y == float2(1.0f, 4.0f).y)) && all(bool2(_68.x == float2(2.0f, 5.0f).x, _68.y == float2(2.0f, 5.0f).y))) && all(bool2(_72.x == float2(3.0f, 6.0f).x, _72.y == float2(3.0f, 6.0f).y));
    }
    else
    {
        _76 = false;
    }
    bool _103 = false;
    if (_76)
    {
        float3x3 _79 = transpose(_7_testMatrix3x3);
        float3 _92 = _79[0];
        float3 _95 = _79[1];
        float3 _99 = _79[2];
        _103 = (all(bool3(_92.x == float3(1.0f, 4.0f, 7.0f).x, _92.y == float3(1.0f, 4.0f, 7.0f).y, _92.z == float3(1.0f, 4.0f, 7.0f).z)) && all(bool3(_95.x == float3(2.0f, 5.0f, 8.0f).x, _95.y == float3(2.0f, 5.0f, 8.0f).y, _95.z == float3(2.0f, 5.0f, 8.0f).z))) && all(bool3(_99.x == float3(3.0f, 6.0f, 9.0f).x, _99.y == float3(3.0f, 6.0f, 9.0f).y, _99.z == float3(3.0f, 6.0f, 9.0f).z));
    }
    else
    {
        _103 = false;
    }
    float4 _104 = 0.0f.xxxx;
    if (_103)
    {
        _104 = _7_colorGreen;
    }
    else
    {
        _104 = _7_colorRed;
    }
    return _104;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
