cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _16_colorWhite : packoffset(c0);
    float4 _16_colorGreen : packoffset(c1);
    float4 _16_colorRed : packoffset(c2);
    row_major float2x2 _16_testMatrix2x2 : packoffset(c3);
    row_major float3x3 _16_testMatrix3x3 : packoffset(c5);
    row_major float4x4 _16_testMatrix4x4 : packoffset(c8);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_iscalar_b()
{
    int _43 = int(_16_colorWhite.x);
    int x = _43;
    int _44 = -_43;
    x = _44;
    return _44 == (-1);
}

bool test_fvec_b()
{
    float2 x = _16_colorWhite.xy;
    float2 _52 = -_16_colorWhite.xy;
    x = _52;
    return all(bool2(_52.x == (-1.0f).xx.x, _52.y == (-1.0f).xx.y));
}

bool test_ivec_b()
{
    int2 _66 = int(_16_colorWhite.x).xx;
    int2 x = _66;
    int2 _67 = -_66;
    x = _67;
    return all(bool2(_67.x == int2(-1, -1).x, _67.y == int2(-1, -1).y));
}

bool test_mat2_b()
{
    float2x2 negated = float2x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f));
    float2x2 x = _16_testMatrix2x2;
    float2 _86 = -_16_testMatrix2x2[0];
    float2 _88 = -_16_testMatrix2x2[1];
    x = float2x2(_86, _88);
    return all(bool2(_86.x == float2(-1.0f, -2.0f).x, _86.y == float2(-1.0f, -2.0f).y)) && all(bool2(_88.x == float2(-3.0f, -4.0f).x, _88.y == float2(-3.0f, -4.0f).y));
}

bool test_mat3_b()
{
    float3x3 negated = float3x3(float3(-1.0f, -2.0f, -3.0f), float3(-4.0f, -5.0f, -6.0f), float3(-7.0f, -8.0f, -9.0f));
    float3x3 x = _16_testMatrix3x3;
    float3 _113 = -_16_testMatrix3x3[0];
    float3 _115 = -_16_testMatrix3x3[1];
    float3 _117 = -_16_testMatrix3x3[2];
    x = float3x3(_113, _115, _117);
    return (all(bool3(_113.x == float3(-1.0f, -2.0f, -3.0f).x, _113.y == float3(-1.0f, -2.0f, -3.0f).y, _113.z == float3(-1.0f, -2.0f, -3.0f).z)) && all(bool3(_115.x == float3(-4.0f, -5.0f, -6.0f).x, _115.y == float3(-4.0f, -5.0f, -6.0f).y, _115.z == float3(-4.0f, -5.0f, -6.0f).z))) && all(bool3(_117.x == float3(-7.0f, -8.0f, -9.0f).x, _117.y == float3(-7.0f, -8.0f, -9.0f).y, _117.z == float3(-7.0f, -8.0f, -9.0f).z));
}

bool test_mat4_b()
{
    float4x4 negated = float4x4(float4(-1.0f, -2.0f, -3.0f, -4.0f), float4(-5.0f, -6.0f, -7.0f, -8.0f), float4(-9.0f, -10.0f, -11.0f, -12.0f), float4(-13.0f, -14.0f, -15.0f, -16.0f));
    float4x4 x = _16_testMatrix4x4;
    float4 _149 = -_16_testMatrix4x4[0];
    float4 _151 = -_16_testMatrix4x4[1];
    float4 _153 = -_16_testMatrix4x4[2];
    float4 _155 = -_16_testMatrix4x4[3];
    x = float4x4(_149, _151, _153, _155);
    return ((all(bool4(_149.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).x, _149.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).y, _149.z == float4(-1.0f, -2.0f, -3.0f, -4.0f).z, _149.w == float4(-1.0f, -2.0f, -3.0f, -4.0f).w)) && all(bool4(_151.x == float4(-5.0f, -6.0f, -7.0f, -8.0f).x, _151.y == float4(-5.0f, -6.0f, -7.0f, -8.0f).y, _151.z == float4(-5.0f, -6.0f, -7.0f, -8.0f).z, _151.w == float4(-5.0f, -6.0f, -7.0f, -8.0f).w))) && all(bool4(_153.x == float4(-9.0f, -10.0f, -11.0f, -12.0f).x, _153.y == float4(-9.0f, -10.0f, -11.0f, -12.0f).y, _153.z == float4(-9.0f, -10.0f, -11.0f, -12.0f).z, _153.w == float4(-9.0f, -10.0f, -11.0f, -12.0f).w))) && all(bool4(_155.x == float4(-13.0f, -14.0f, -15.0f, -16.0f).x, _155.y == float4(-13.0f, -14.0f, -15.0f, -16.0f).y, _155.z == float4(-13.0f, -14.0f, -15.0f, -16.0f).z, _155.w == float4(-13.0f, -14.0f, -15.0f, -16.0f).w));
}

float4 main(float2 _170)
{
    float _RESERVED_IDENTIFIER_FIXUP_0_x = _16_colorWhite.x;
    float _177 = -_16_colorWhite.x;
    _RESERVED_IDENTIFIER_FIXUP_0_x = _177;
    bool _183 = false;
    if (_177 == (-1.0f))
    {
        _183 = test_iscalar_b();
    }
    else
    {
        _183 = false;
    }
    bool _187 = false;
    if (_183)
    {
        _187 = test_fvec_b();
    }
    else
    {
        _187 = false;
    }
    bool _191 = false;
    if (_187)
    {
        _191 = test_ivec_b();
    }
    else
    {
        _191 = false;
    }
    bool _195 = false;
    if (_191)
    {
        _195 = test_mat2_b();
    }
    else
    {
        _195 = false;
    }
    bool _199 = false;
    if (_195)
    {
        _199 = test_mat3_b();
    }
    else
    {
        _199 = false;
    }
    bool _203 = false;
    if (_199)
    {
        _203 = test_mat4_b();
    }
    else
    {
        _203 = false;
    }
    float4 _204 = 0.0f.xxxx;
    if (_203)
    {
        _204 = _16_colorGreen;
    }
    else
    {
        _204 = _16_colorRed;
    }
    return _204;
}

void frag_main()
{
    float2 _30 = 0.0f.xx;
    sk_FragColor = main(_30);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
