cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _7_testMatrix3x3 : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float2x2 h22 = float2x2(1000000.0f.xx, 1000000.0f.xx);
    float2x2 hugeM22 = float2x2(1000000015047466219876688855040.0f.xx, 1000000015047466219876688855040.0f.xx);
    h22 = float2x2(1000000015047466219876688855040.0f.xx * 1000000015047466219876688855040.0f.xx, 1000000015047466219876688855040.0f.xx * 1000000015047466219876688855040.0f.xx);
    h22 = float2x2(float2(0.0f, 5.0f), float2(10.0f, 15.0f));
    float2 _57 = _7_testMatrix2x2[0] * float2(1.0f, 0.0f);
    float2 _59 = _7_testMatrix2x2[1] * float2(0.0f, 1.0f);
    float2x2 f22 = float2x2(_57, _59);
    float3 _72 = _7_testMatrix3x3[0] * 2.0f.xxx;
    float3 _74 = _7_testMatrix3x3[1] * 2.0f.xxx;
    float3 _76 = _7_testMatrix3x3[2] * 2.0f.xxx;
    float3x3 h33 = float3x3(_72, _74, _76);
    bool _96 = false;
    if (all(bool2(float2(0.0f, 5.0f).x == float2(0.0f, 5.0f).x, float2(0.0f, 5.0f).y == float2(0.0f, 5.0f).y)) && all(bool2(float2(10.0f, 15.0f).x == float2(10.0f, 15.0f).x, float2(10.0f, 15.0f).y == float2(10.0f, 15.0f).y)))
    {
        _96 = all(bool2(_57.x == float2(1.0f, 0.0f).x, _57.y == float2(1.0f, 0.0f).y)) && all(bool2(_59.x == float2(0.0f, 4.0f).x, _59.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _96 = false;
    }
    bool _118 = false;
    if (_96)
    {
        _118 = (all(bool3(_72.x == float3(2.0f, 4.0f, 6.0f).x, _72.y == float3(2.0f, 4.0f, 6.0f).y, _72.z == float3(2.0f, 4.0f, 6.0f).z)) && all(bool3(_74.x == float3(8.0f, 10.0f, 12.0f).x, _74.y == float3(8.0f, 10.0f, 12.0f).y, _74.z == float3(8.0f, 10.0f, 12.0f).z))) && all(bool3(_76.x == float3(14.0f, 16.0f, 18.0f).x, _76.y == float3(14.0f, 16.0f, 18.0f).y, _76.z == float3(14.0f, 16.0f, 18.0f).z));
    }
    else
    {
        _118 = false;
    }
    float4 _119 = 0.0f.xxxx;
    if (_118)
    {
        _119 = _7_colorGreen;
    }
    else
    {
        _119 = _7_colorRed;
    }
    return _119;
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
