cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float2x4 _33 = float2x4(9.0f.xxxx, 9.0f.xxxx);
    float2x4 _42 = float2x4(_10_colorRed, _10_colorGreen);
    float2x4 h24 = float2x4(_33[0] * _42[0], _33[1] * _42[1]);
    float4x2 _66 = float4x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f), float2(7.0f, 8.0f));
    float4x2 _83 = float4x2(float2(_10_colorRed.xy), float2(_10_colorRed.zw), float2(_10_colorGreen.xy), float2(_10_colorGreen.zw));
    float4x2 h42 = float4x2(_66[0] * _83[0], _66[1] * _83[1], _66[2] * _83[2], _66[3] * _83[3]);
    float4x3 f43 = float4x3(float3(12.0f, 22.0f, 30.0f), float3(36.0f, 40.0f, 42.0f), float3(42.0f, 40.0f, 36.0f), float3(30.0f, 22.0f, 12.0f));
    float2x4 _116 = float2x4(float4(9.0f, 0.0f, 0.0f, 9.0f), float4(0.0f, 9.0f, 0.0f, 9.0f));
    float4 _119 = _116[0];
    float4 _123 = _116[1];
    bool _155 = false;
    if (all(bool4(h24[0].x == _119.x, h24[0].y == _119.y, h24[0].z == _119.z, h24[0].w == _119.w)) && all(bool4(h24[1].x == _123.x, h24[1].y == _123.y, h24[1].z == _123.z, h24[1].w == _123.w)))
    {
        float4x2 _134 = float4x2(float2(1.0f, 0.0f), float2(0.0f, 4.0f), float2(0.0f, 6.0f), float2(0.0f, 8.0f));
        float2 _137 = _134[0];
        float2 _141 = _134[1];
        float2 _146 = _134[2];
        float2 _151 = _134[3];
        _155 = ((all(bool2(h42[0].x == _137.x, h42[0].y == _137.y)) && all(bool2(h42[1].x == _141.x, h42[1].y == _141.y))) && all(bool2(h42[2].x == _146.x, h42[2].y == _146.y))) && all(bool2(h42[3].x == _151.x, h42[3].y == _151.y));
    }
    else
    {
        _155 = false;
    }
    bool _184 = false;
    if (_155)
    {
        float4x3 _163 = float4x3(float3(12.0f, 22.0f, 30.0f), float3(36.0f, 40.0f, 42.0f), float3(42.0f, 40.0f, 36.0f), float3(30.0f, 22.0f, 12.0f));
        float3 _166 = _163[0];
        float3 _170 = _163[1];
        float3 _175 = _163[2];
        float3 _180 = _163[3];
        _184 = ((all(bool3(f43[0].x == _166.x, f43[0].y == _166.y, f43[0].z == _166.z)) && all(bool3(f43[1].x == _170.x, f43[1].y == _170.y, f43[1].z == _170.z))) && all(bool3(f43[2].x == _175.x, f43[2].y == _175.y, f43[2].z == _175.z))) && all(bool3(f43[3].x == _180.x, f43[3].y == _180.y, f43[3].z == _180.z));
    }
    else
    {
        _184 = false;
    }
    float4 _185 = 0.0f.xxxx;
    if (_184)
    {
        _185 = _10_colorGreen;
    }
    else
    {
        _185 = _10_colorRed;
    }
    return _185;
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
