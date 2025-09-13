cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    row_major float2x2 _11_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _11_testMatrix3x3 : packoffset(c4);
    float4 _11_testInputs : packoffset(c7);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _28)
{
    float2x2 _32 = float2x2(_11_testMatrix2x2[0] * _11_testMatrix2x2[1].x, _11_testMatrix2x2[0] * _11_testMatrix2x2[1].y);
    float2 _52 = _32[0];
    float2 _55 = _32[1];
    bool _92 = false;
    if (all(bool2(_52.x == float2(3.0f, 6.0f).x, _52.y == float2(3.0f, 6.0f).y)) && all(bool2(_55.x == float2(4.0f, 8.0f).x, _55.y == float2(4.0f, 8.0f).y)))
    {
        float3x3 _61 = float3x3(_11_testMatrix3x3[0] * _11_testMatrix3x3[1].x, _11_testMatrix3x3[0] * _11_testMatrix3x3[1].y, _11_testMatrix3x3[0] * _11_testMatrix3x3[1].z);
        float3 _81 = _61[0];
        float3 _84 = _61[1];
        float3 _88 = _61[2];
        _92 = (all(bool3(_81.x == float3(4.0f, 8.0f, 12.0f).x, _81.y == float3(4.0f, 8.0f, 12.0f).y, _81.z == float3(4.0f, 8.0f, 12.0f).z)) && all(bool3(_84.x == float3(5.0f, 10.0f, 15.0f).x, _84.y == float3(5.0f, 10.0f, 15.0f).y, _84.z == float3(5.0f, 10.0f, 15.0f).z))) && all(bool3(_88.x == float3(6.0f, 12.0f, 18.0f).x, _88.y == float3(6.0f, 12.0f, 18.0f).y, _88.z == float3(6.0f, 12.0f, 18.0f).z));
    }
    else
    {
        _92 = false;
    }
    bool _117 = false;
    if (_92)
    {
        float3x2 _95 = float3x2(_11_testMatrix2x2[0] * _11_testMatrix3x3[1].x, _11_testMatrix2x2[0] * _11_testMatrix3x3[1].y, _11_testMatrix2x2[0] * _11_testMatrix3x3[1].z);
        float2 _106 = _95[0];
        float2 _109 = _95[1];
        float2 _113 = _95[2];
        _117 = (all(bool2(_106.x == float2(4.0f, 8.0f).x, _106.y == float2(4.0f, 8.0f).y)) && all(bool2(_109.x == float2(5.0f, 10.0f).x, _109.y == float2(5.0f, 10.0f).y))) && all(bool2(_113.x == float2(6.0f, 12.0f).x, _113.y == float2(6.0f, 12.0f).y));
    }
    else
    {
        _117 = false;
    }
    bool _155 = false;
    if (_117)
    {
        float4x4 _120 = float4x4(_11_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).x, _11_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).y, _11_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).z, _11_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).w);
        float4 _140 = _120[0];
        float4 _143 = _120[1];
        float4 _147 = _120[2];
        float4 _151 = _120[3];
        _155 = ((all(bool4(_140.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _140.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _140.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _140.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)) && all(bool4(_143.x == 0.0f.xxxx.x, _143.y == 0.0f.xxxx.y, _143.z == 0.0f.xxxx.z, _143.w == 0.0f.xxxx.w))) && all(bool4(_147.x == 0.0f.xxxx.x, _147.y == 0.0f.xxxx.y, _147.z == 0.0f.xxxx.z, _147.w == 0.0f.xxxx.w))) && all(bool4(_151.x == float4(-2.5f, 0.0f, 1.5f, 4.5f).x, _151.y == float4(-2.5f, 0.0f, 1.5f, 4.5f).y, _151.z == float4(-2.5f, 0.0f, 1.5f, 4.5f).z, _151.w == float4(-2.5f, 0.0f, 1.5f, 4.5f).w));
    }
    else
    {
        _155 = false;
    }
    bool _171 = false;
    if (_155)
    {
        float2x4 _158 = float2x4(_11_testInputs * float2(1.0f, 2.0f).x, _11_testInputs * float2(1.0f, 2.0f).y);
        float4 _164 = _158[0];
        float4 _167 = _158[1];
        _171 = all(bool4(_164.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _164.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _164.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _164.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)) && all(bool4(_167.x == float4(-2.5f, 0.0f, 1.5f, 4.5f).x, _167.y == float4(-2.5f, 0.0f, 1.5f, 4.5f).y, _167.z == float4(-2.5f, 0.0f, 1.5f, 4.5f).z, _167.w == float4(-2.5f, 0.0f, 1.5f, 4.5f).w));
    }
    else
    {
        _171 = false;
    }
    bool _197 = false;
    if (_171)
    {
        float4x2 _174 = float4x2(float2(1.0f, 2.0f) * _11_testInputs.x, float2(1.0f, 2.0f) * _11_testInputs.y, float2(1.0f, 2.0f) * _11_testInputs.z, float2(1.0f, 2.0f) * _11_testInputs.w);
        float2 _182 = _174[0];
        float2 _185 = _174[1];
        float2 _189 = _174[2];
        float2 _193 = _174[3];
        _197 = ((all(bool2(_182.x == float2(-1.25f, -2.5f).x, _182.y == float2(-1.25f, -2.5f).y)) && all(bool2(_185.x == 0.0f.xx.x, _185.y == 0.0f.xx.y))) && all(bool2(_189.x == float2(0.75f, 1.5f).x, _189.y == float2(0.75f, 1.5f).y))) && all(bool2(_193.x == float2(2.25f, 4.5f).x, _193.y == float2(2.25f, 4.5f).y));
    }
    else
    {
        _197 = false;
    }
    float4 _198 = 0.0f.xxxx;
    if (_197)
    {
        _198 = _11_colorGreen;
    }
    else
    {
        _198 = _11_colorRed;
    }
    return _198;
}

void frag_main()
{
    float2 _24 = 0.0f.xx;
    sk_FragColor = main(_24);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
