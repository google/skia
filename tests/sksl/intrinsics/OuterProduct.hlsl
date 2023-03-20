cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _10_testMatrix3x3 : packoffset(c4);
    float4 _10_testInputs : packoffset(c7);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    float2x2 _30 = float2x2(_10_testMatrix2x2[0] * _10_testMatrix2x2[1].x, _10_testMatrix2x2[0] * _10_testMatrix2x2[1].y);
    float2 _51 = _30[0];
    float2 _54 = _30[1];
    bool _91 = false;
    if (all(bool2(_51.x == float2(3.0f, 6.0f).x, _51.y == float2(3.0f, 6.0f).y)) && all(bool2(_54.x == float2(4.0f, 8.0f).x, _54.y == float2(4.0f, 8.0f).y)))
    {
        float3x3 _60 = float3x3(_10_testMatrix3x3[0] * _10_testMatrix3x3[1].x, _10_testMatrix3x3[0] * _10_testMatrix3x3[1].y, _10_testMatrix3x3[0] * _10_testMatrix3x3[1].z);
        float3 _80 = _60[0];
        float3 _83 = _60[1];
        float3 _87 = _60[2];
        _91 = (all(bool3(_80.x == float3(4.0f, 8.0f, 12.0f).x, _80.y == float3(4.0f, 8.0f, 12.0f).y, _80.z == float3(4.0f, 8.0f, 12.0f).z)) && all(bool3(_83.x == float3(5.0f, 10.0f, 15.0f).x, _83.y == float3(5.0f, 10.0f, 15.0f).y, _83.z == float3(5.0f, 10.0f, 15.0f).z))) && all(bool3(_87.x == float3(6.0f, 12.0f, 18.0f).x, _87.y == float3(6.0f, 12.0f, 18.0f).y, _87.z == float3(6.0f, 12.0f, 18.0f).z));
    }
    else
    {
        _91 = false;
    }
    bool _116 = false;
    if (_91)
    {
        float3x2 _94 = float3x2(_10_testMatrix2x2[0] * _10_testMatrix3x3[1].x, _10_testMatrix2x2[0] * _10_testMatrix3x3[1].y, _10_testMatrix2x2[0] * _10_testMatrix3x3[1].z);
        float2 _105 = _94[0];
        float2 _108 = _94[1];
        float2 _112 = _94[2];
        _116 = (all(bool2(_105.x == float2(4.0f, 8.0f).x, _105.y == float2(4.0f, 8.0f).y)) && all(bool2(_108.x == float2(5.0f, 10.0f).x, _108.y == float2(5.0f, 10.0f).y))) && all(bool2(_112.x == float2(6.0f, 12.0f).x, _112.y == float2(6.0f, 12.0f).y));
    }
    else
    {
        _116 = false;
    }
    bool _154 = false;
    if (_116)
    {
        float4x4 _119 = float4x4(_10_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).x, _10_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).y, _10_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).z, _10_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).w);
        float4 _139 = _119[0];
        float4 _142 = _119[1];
        float4 _146 = _119[2];
        float4 _150 = _119[3];
        _154 = ((all(bool4(_139.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _139.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _139.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _139.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)) && all(bool4(_142.x == 0.0f.xxxx.x, _142.y == 0.0f.xxxx.y, _142.z == 0.0f.xxxx.z, _142.w == 0.0f.xxxx.w))) && all(bool4(_146.x == 0.0f.xxxx.x, _146.y == 0.0f.xxxx.y, _146.z == 0.0f.xxxx.z, _146.w == 0.0f.xxxx.w))) && all(bool4(_150.x == float4(-2.5f, 0.0f, 1.5f, 4.5f).x, _150.y == float4(-2.5f, 0.0f, 1.5f, 4.5f).y, _150.z == float4(-2.5f, 0.0f, 1.5f, 4.5f).z, _150.w == float4(-2.5f, 0.0f, 1.5f, 4.5f).w));
    }
    else
    {
        _154 = false;
    }
    bool _170 = false;
    if (_154)
    {
        float2x4 _157 = float2x4(_10_testInputs * float2(1.0f, 2.0f).x, _10_testInputs * float2(1.0f, 2.0f).y);
        float4 _163 = _157[0];
        float4 _166 = _157[1];
        _170 = all(bool4(_163.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _163.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _163.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _163.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)) && all(bool4(_166.x == float4(-2.5f, 0.0f, 1.5f, 4.5f).x, _166.y == float4(-2.5f, 0.0f, 1.5f, 4.5f).y, _166.z == float4(-2.5f, 0.0f, 1.5f, 4.5f).z, _166.w == float4(-2.5f, 0.0f, 1.5f, 4.5f).w));
    }
    else
    {
        _170 = false;
    }
    bool _196 = false;
    if (_170)
    {
        float4x2 _173 = float4x2(float2(1.0f, 2.0f) * _10_testInputs.x, float2(1.0f, 2.0f) * _10_testInputs.y, float2(1.0f, 2.0f) * _10_testInputs.z, float2(1.0f, 2.0f) * _10_testInputs.w);
        float2 _181 = _173[0];
        float2 _184 = _173[1];
        float2 _188 = _173[2];
        float2 _192 = _173[3];
        _196 = ((all(bool2(_181.x == float2(-1.25f, -2.5f).x, _181.y == float2(-1.25f, -2.5f).y)) && all(bool2(_184.x == 0.0f.xx.x, _184.y == 0.0f.xx.y))) && all(bool2(_188.x == float2(0.75f, 1.5f).x, _188.y == float2(0.75f, 1.5f).y))) && all(bool2(_192.x == float2(2.25f, 4.5f).x, _192.y == float2(2.25f, 4.5f).y));
    }
    else
    {
        _196 = false;
    }
    float4 _197 = 0.0f.xxxx;
    if (_196)
    {
        _197 = _10_colorGreen;
    }
    else
    {
        _197 = _10_colorRed;
    }
    return _197;
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
