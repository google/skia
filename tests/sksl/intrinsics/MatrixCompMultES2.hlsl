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
    float2x2 h22 = float2x2(float2(0.0f, 5.0f), float2(10.0f, 15.0f));
    float2x2 _45 = float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f));
    float2x2 f22 = float2x2(_10_testMatrix2x2[0] * _45[0], _10_testMatrix2x2[1] * _45[1]);
    float3x3 _66 = float3x3(2.0f.xxx, 2.0f.xxx, 2.0f.xxx);
    float3x3 h33 = float3x3(_10_testMatrix3x3[0] * _66[0], _10_testMatrix3x3[1] * _66[1], _10_testMatrix3x3[2] * _66[2]);
    float2x2 _81 = float2x2(float2(0.0f, 5.0f), float2(10.0f, 15.0f));
    float2 _84 = _81[0];
    float2 _88 = _81[1];
    bool _108 = false;
    if (all(bool2(h22[0].x == _84.x, h22[0].y == _84.y)) && all(bool2(h22[1].x == _88.x, h22[1].y == _88.y)))
    {
        float2x2 _98 = float2x2(float2(1.0f, 0.0f), float2(0.0f, 4.0f));
        float2 _100 = _98[0];
        float2 _104 = _98[1];
        _108 = all(bool2(f22[0].x == _100.x, f22[0].y == _100.y)) && all(bool2(f22[1].x == _104.x, f22[1].y == _104.y));
    }
    else
    {
        _108 = false;
    }
    bool _137 = false;
    if (_108)
    {
        float3x3 _121 = float3x3(float3(2.0f, 4.0f, 6.0f), float3(8.0f, 10.0f, 12.0f), float3(14.0f, 16.0f, 18.0f));
        float3 _124 = _121[0];
        float3 _128 = _121[1];
        float3 _133 = _121[2];
        _137 = (all(bool3(h33[0].x == _124.x, h33[0].y == _124.y, h33[0].z == _124.z)) && all(bool3(h33[1].x == _128.x, h33[1].y == _128.y, h33[1].z == _128.z))) && all(bool3(h33[2].x == _133.x, h33[2].y == _133.y, h33[2].z == _133.z));
    }
    else
    {
        _137 = false;
    }
    float4 _138 = 0.0f.xxxx;
    if (_137)
    {
        _138 = _10_colorGreen;
    }
    else
    {
        _138 = _10_colorRed;
    }
    return _138;
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
