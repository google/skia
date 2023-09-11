cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _7_testMatrix3x3 : packoffset(c4);
    float4 _7_testInputs : packoffset(c7);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float2x2 _28 = float2x2(_7_testMatrix2x2[0] * _7_testMatrix2x2[1].x, _7_testMatrix2x2[0] * _7_testMatrix2x2[1].y);
    float2 _49 = _28[0];
    float2 _52 = _28[1];
    bool _89 = false;
    if (all(bool2(_49.x == float2(3.0f, 6.0f).x, _49.y == float2(3.0f, 6.0f).y)) && all(bool2(_52.x == float2(4.0f, 8.0f).x, _52.y == float2(4.0f, 8.0f).y)))
    {
        float3x3 _58 = float3x3(_7_testMatrix3x3[0] * _7_testMatrix3x3[1].x, _7_testMatrix3x3[0] * _7_testMatrix3x3[1].y, _7_testMatrix3x3[0] * _7_testMatrix3x3[1].z);
        float3 _78 = _58[0];
        float3 _81 = _58[1];
        float3 _85 = _58[2];
        _89 = (all(bool3(_78.x == float3(4.0f, 8.0f, 12.0f).x, _78.y == float3(4.0f, 8.0f, 12.0f).y, _78.z == float3(4.0f, 8.0f, 12.0f).z)) && all(bool3(_81.x == float3(5.0f, 10.0f, 15.0f).x, _81.y == float3(5.0f, 10.0f, 15.0f).y, _81.z == float3(5.0f, 10.0f, 15.0f).z))) && all(bool3(_85.x == float3(6.0f, 12.0f, 18.0f).x, _85.y == float3(6.0f, 12.0f, 18.0f).y, _85.z == float3(6.0f, 12.0f, 18.0f).z));
    }
    else
    {
        _89 = false;
    }
    bool _114 = false;
    if (_89)
    {
        float3x2 _92 = float3x2(_7_testMatrix2x2[0] * _7_testMatrix3x3[1].x, _7_testMatrix2x2[0] * _7_testMatrix3x3[1].y, _7_testMatrix2x2[0] * _7_testMatrix3x3[1].z);
        float2 _103 = _92[0];
        float2 _106 = _92[1];
        float2 _110 = _92[2];
        _114 = (all(bool2(_103.x == float2(4.0f, 8.0f).x, _103.y == float2(4.0f, 8.0f).y)) && all(bool2(_106.x == float2(5.0f, 10.0f).x, _106.y == float2(5.0f, 10.0f).y))) && all(bool2(_110.x == float2(6.0f, 12.0f).x, _110.y == float2(6.0f, 12.0f).y));
    }
    else
    {
        _114 = false;
    }
    bool _152 = false;
    if (_114)
    {
        float4x4 _117 = float4x4(_7_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).x, _7_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).y, _7_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).z, _7_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).w);
        float4 _137 = _117[0];
        float4 _140 = _117[1];
        float4 _144 = _117[2];
        float4 _148 = _117[3];
        _152 = ((all(bool4(_137.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _137.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _137.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _137.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)) && all(bool4(_140.x == 0.0f.xxxx.x, _140.y == 0.0f.xxxx.y, _140.z == 0.0f.xxxx.z, _140.w == 0.0f.xxxx.w))) && all(bool4(_144.x == 0.0f.xxxx.x, _144.y == 0.0f.xxxx.y, _144.z == 0.0f.xxxx.z, _144.w == 0.0f.xxxx.w))) && all(bool4(_148.x == float4(-2.5f, 0.0f, 1.5f, 4.5f).x, _148.y == float4(-2.5f, 0.0f, 1.5f, 4.5f).y, _148.z == float4(-2.5f, 0.0f, 1.5f, 4.5f).z, _148.w == float4(-2.5f, 0.0f, 1.5f, 4.5f).w));
    }
    else
    {
        _152 = false;
    }
    bool _168 = false;
    if (_152)
    {
        float2x4 _155 = float2x4(_7_testInputs * float2(1.0f, 2.0f).x, _7_testInputs * float2(1.0f, 2.0f).y);
        float4 _161 = _155[0];
        float4 _164 = _155[1];
        _168 = all(bool4(_161.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _161.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _161.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _161.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)) && all(bool4(_164.x == float4(-2.5f, 0.0f, 1.5f, 4.5f).x, _164.y == float4(-2.5f, 0.0f, 1.5f, 4.5f).y, _164.z == float4(-2.5f, 0.0f, 1.5f, 4.5f).z, _164.w == float4(-2.5f, 0.0f, 1.5f, 4.5f).w));
    }
    else
    {
        _168 = false;
    }
    bool _194 = false;
    if (_168)
    {
        float4x2 _171 = float4x2(float2(1.0f, 2.0f) * _7_testInputs.x, float2(1.0f, 2.0f) * _7_testInputs.y, float2(1.0f, 2.0f) * _7_testInputs.z, float2(1.0f, 2.0f) * _7_testInputs.w);
        float2 _179 = _171[0];
        float2 _182 = _171[1];
        float2 _186 = _171[2];
        float2 _190 = _171[3];
        _194 = ((all(bool2(_179.x == float2(-1.25f, -2.5f).x, _179.y == float2(-1.25f, -2.5f).y)) && all(bool2(_182.x == 0.0f.xx.x, _182.y == 0.0f.xx.y))) && all(bool2(_186.x == float2(0.75f, 1.5f).x, _186.y == float2(0.75f, 1.5f).y))) && all(bool2(_190.x == float2(2.25f, 4.5f).x, _190.y == float2(2.25f, 4.5f).y));
    }
    else
    {
        _194 = false;
    }
    float4 _195 = 0.0f.xxxx;
    if (_194)
    {
        _195 = _7_colorGreen;
    }
    else
    {
        _195 = _7_colorRed;
    }
    return _195;
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
