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
    float2 c12 = float2(1.0f, 2.0f);
    float2x2 _34 = float2x2(_10_testMatrix2x2[0] * _10_testMatrix2x2[1].x, _10_testMatrix2x2[0] * _10_testMatrix2x2[1].y);
    float2x2 _53 = float2x2(float2(3.0f, 6.0f), float2(4.0f, 8.0f));
    float2 _55 = _34[0];
    float2 _56 = _53[0];
    float2 _59 = _34[1];
    float2 _60 = _53[1];
    bool _100 = false;
    if (all(bool2(_55.x == _56.x, _55.y == _56.y)) && all(bool2(_59.x == _60.x, _59.y == _60.y)))
    {
        float3x3 _66 = float3x3(_10_testMatrix3x3[0] * _10_testMatrix3x3[1].x, _10_testMatrix3x3[0] * _10_testMatrix3x3[1].y, _10_testMatrix3x3[0] * _10_testMatrix3x3[1].z);
        float3x3 _84 = float3x3(float3(4.0f, 8.0f, 12.0f), float3(5.0f, 10.0f, 15.0f), float3(6.0f, 12.0f, 18.0f));
        float3 _86 = _66[0];
        float3 _87 = _84[0];
        float3 _90 = _66[1];
        float3 _91 = _84[1];
        float3 _95 = _66[2];
        float3 _96 = _84[2];
        _100 = (all(bool3(_86.x == _87.x, _86.y == _87.y, _86.z == _87.z)) && all(bool3(_90.x == _91.x, _90.y == _91.y, _90.z == _91.z))) && all(bool3(_95.x == _96.x, _95.y == _96.y, _95.z == _96.z));
    }
    else
    {
        _100 = false;
    }
    bool _129 = false;
    if (_100)
    {
        float3x2 _103 = float3x2(_10_testMatrix2x2[0] * _10_testMatrix3x3[1].x, _10_testMatrix2x2[0] * _10_testMatrix3x3[1].y, _10_testMatrix2x2[0] * _10_testMatrix3x3[1].z);
        float3x2 _114 = float3x2(float2(4.0f, 8.0f), float2(5.0f, 10.0f), float2(6.0f, 12.0f));
        float2 _115 = _103[0];
        float2 _116 = _114[0];
        float2 _119 = _103[1];
        float2 _120 = _114[1];
        float2 _124 = _103[2];
        float2 _125 = _114[2];
        _129 = (all(bool2(_115.x == _116.x, _115.y == _116.y)) && all(bool2(_119.x == _120.x, _119.y == _120.y))) && all(bool2(_124.x == _125.x, _124.y == _125.y));
    }
    else
    {
        _129 = false;
    }
    bool _170 = false;
    if (_129)
    {
        float4x4 _132 = float4x4(_10_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).x, _10_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).y, _10_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).z, _10_testInputs * float4(1.0f, 0.0f, 0.0f, 2.0f).w);
        float4x4 _149 = float4x4(float4(-1.25f, 0.0f, 0.75f, 2.25f), 0.0f.xxxx, 0.0f.xxxx, float4(-2.5f, 0.0f, 1.5f, 4.5f));
        float4 _151 = _132[0];
        float4 _152 = _149[0];
        float4 _155 = _132[1];
        float4 _156 = _149[1];
        float4 _160 = _132[2];
        float4 _161 = _149[2];
        float4 _165 = _132[3];
        float4 _166 = _149[3];
        _170 = ((all(bool4(_151.x == _152.x, _151.y == _152.y, _151.z == _152.z, _151.w == _152.w)) && all(bool4(_155.x == _156.x, _155.y == _156.y, _155.z == _156.z, _155.w == _156.w))) && all(bool4(_160.x == _161.x, _160.y == _161.y, _160.z == _161.z, _160.w == _161.w))) && all(bool4(_165.x == _166.x, _165.y == _166.y, _165.z == _166.z, _165.w == _166.w));
    }
    else
    {
        _170 = false;
    }
    bool _190 = false;
    if (_170)
    {
        float2x4 _173 = float2x4(_10_testInputs * c12.x, _10_testInputs * c12.y);
        float2x4 _180 = float2x4(float4(-1.25f, 0.0f, 0.75f, 2.25f), float4(-2.5f, 0.0f, 1.5f, 4.5f));
        float4 _181 = _173[0];
        float4 _182 = _180[0];
        float4 _185 = _173[1];
        float4 _186 = _180[1];
        _190 = all(bool4(_181.x == _182.x, _181.y == _182.y, _181.z == _182.z, _181.w == _182.w)) && all(bool4(_185.x == _186.x, _185.y == _186.y, _185.z == _186.z, _185.w == _186.w));
    }
    else
    {
        _190 = false;
    }
    bool _222 = false;
    if (_190)
    {
        float4x2 _193 = float4x2(c12 * _10_testInputs.x, c12 * _10_testInputs.y, c12 * _10_testInputs.z, c12 * _10_testInputs.w);
        float4x2 _202 = float4x2(float2(-1.25f, -2.5f), 0.0f.xx, float2(0.75f, 1.5f), float2(2.25f, 4.5f));
        float2 _203 = _193[0];
        float2 _204 = _202[0];
        float2 _207 = _193[1];
        float2 _208 = _202[1];
        float2 _212 = _193[2];
        float2 _213 = _202[2];
        float2 _217 = _193[3];
        float2 _218 = _202[3];
        _222 = ((all(bool2(_203.x == _204.x, _203.y == _204.y)) && all(bool2(_207.x == _208.x, _207.y == _208.y))) && all(bool2(_212.x == _213.x, _212.y == _213.y))) && all(bool2(_217.x == _218.x, _217.y == _218.y));
    }
    else
    {
        _222 = false;
    }
    float4 _223 = 0.0f.xxxx;
    if (_222)
    {
        _223 = _10_colorGreen;
    }
    else
    {
        _223 = _10_colorRed;
    }
    return _223;
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
