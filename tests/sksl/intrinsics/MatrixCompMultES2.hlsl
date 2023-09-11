cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _10_testMatrix3x3 : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    float2x2 h22 = float2x2(1000000.0f.xx, 1000000.0f.xx);
    float2x2 hugeM22 = float2x2(1000000015047466219876688855040.0f.xx, 1000000015047466219876688855040.0f.xx);
    h22 = float2x2(1000000015047466219876688855040.0f.xx * 1000000015047466219876688855040.0f.xx, 1000000015047466219876688855040.0f.xx * 1000000015047466219876688855040.0f.xx);
    h22 = float2x2(float2(0.0f, 5.0f), float2(10.0f, 15.0f));
    float2 _60 = _10_testMatrix2x2[0] * float2(1.0f, 0.0f);
    float2 _62 = _10_testMatrix2x2[1] * float2(0.0f, 1.0f);
    float2x2 f22 = float2x2(_60, _62);
    float3 _75 = _10_testMatrix3x3[0] * 2.0f.xxx;
    float3 _77 = _10_testMatrix3x3[1] * 2.0f.xxx;
    float3 _79 = _10_testMatrix3x3[2] * 2.0f.xxx;
    float3x3 h33 = float3x3(_75, _77, _79);
    bool _98 = false;
    if (all(bool2(float2(0.0f, 5.0f).x == float2(0.0f, 5.0f).x, float2(0.0f, 5.0f).y == float2(0.0f, 5.0f).y)) && all(bool2(float2(10.0f, 15.0f).x == float2(10.0f, 15.0f).x, float2(10.0f, 15.0f).y == float2(10.0f, 15.0f).y)))
    {
        _98 = all(bool2(_60.x == float2(1.0f, 0.0f).x, _60.y == float2(1.0f, 0.0f).y)) && all(bool2(_62.x == float2(0.0f, 4.0f).x, _62.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _98 = false;
    }
    bool _120 = false;
    if (_98)
    {
        _120 = (all(bool3(_75.x == float3(2.0f, 4.0f, 6.0f).x, _75.y == float3(2.0f, 4.0f, 6.0f).y, _75.z == float3(2.0f, 4.0f, 6.0f).z)) && all(bool3(_77.x == float3(8.0f, 10.0f, 12.0f).x, _77.y == float3(8.0f, 10.0f, 12.0f).y, _77.z == float3(8.0f, 10.0f, 12.0f).z))) && all(bool3(_79.x == float3(14.0f, 16.0f, 18.0f).x, _79.y == float3(14.0f, 16.0f, 18.0f).y, _79.z == float3(14.0f, 16.0f, 18.0f).z));
    }
    else
    {
        _120 = false;
    }
    float4 _121 = 0.0f.xxxx;
    if (_120)
    {
        _121 = _10_colorGreen;
    }
    else
    {
        _121 = _10_colorRed;
    }
    return _121;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
