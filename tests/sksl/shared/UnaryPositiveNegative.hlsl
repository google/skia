cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _19_colorWhite : packoffset(c0);
    float4 _19_colorGreen : packoffset(c1);
    float4 _19_colorRed : packoffset(c2);
    row_major float2x2 _19_testMatrix2x2 : packoffset(c3);
    row_major float3x3 _19_testMatrix3x3 : packoffset(c5);
    row_major float4x4 _19_testMatrix4x4 : packoffset(c8);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_iscalar_b()
{
    int _46 = int(_19_colorWhite.x);
    int x = _46;
    int _47 = -_46;
    x = _47;
    return _47 == (-1);
}

bool test_fvec_b()
{
    float2 x = _19_colorWhite.xy;
    float2 _55 = -_19_colorWhite.xy;
    x = _55;
    return all(bool2(_55.x == (-1.0f).xx.x, _55.y == (-1.0f).xx.y));
}

bool test_ivec_b()
{
    int2 _69 = int(_19_colorWhite.x).xx;
    int2 x = _69;
    int2 _70 = -_69;
    x = _70;
    return all(bool2(_70.x == int2(-1, -1).x, _70.y == int2(-1, -1).y));
}

bool test_mat2_b()
{
    float2x2 negated = float2x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f));
    float2x2 x = _19_testMatrix2x2;
    float2 _89 = -_19_testMatrix2x2[0];
    float2 _91 = -_19_testMatrix2x2[1];
    x = float2x2(_89, _91);
    return all(bool2(_89.x == float2(-1.0f, -2.0f).x, _89.y == float2(-1.0f, -2.0f).y)) && all(bool2(_91.x == float2(-3.0f, -4.0f).x, _91.y == float2(-3.0f, -4.0f).y));
}

bool test_mat3_b()
{
    float3x3 negated = float3x3(float3(-1.0f, -2.0f, -3.0f), float3(-4.0f, -5.0f, -6.0f), float3(-7.0f, -8.0f, -9.0f));
    float3x3 x = _19_testMatrix3x3;
    float3 _116 = -_19_testMatrix3x3[0];
    float3 _118 = -_19_testMatrix3x3[1];
    float3 _120 = -_19_testMatrix3x3[2];
    x = float3x3(_116, _118, _120);
    return (all(bool3(_116.x == float3(-1.0f, -2.0f, -3.0f).x, _116.y == float3(-1.0f, -2.0f, -3.0f).y, _116.z == float3(-1.0f, -2.0f, -3.0f).z)) && all(bool3(_118.x == float3(-4.0f, -5.0f, -6.0f).x, _118.y == float3(-4.0f, -5.0f, -6.0f).y, _118.z == float3(-4.0f, -5.0f, -6.0f).z))) && all(bool3(_120.x == float3(-7.0f, -8.0f, -9.0f).x, _120.y == float3(-7.0f, -8.0f, -9.0f).y, _120.z == float3(-7.0f, -8.0f, -9.0f).z));
}

bool test_mat4_b()
{
    float4x4 negated = float4x4(float4(-1.0f, -2.0f, -3.0f, -4.0f), float4(-5.0f, -6.0f, -7.0f, -8.0f), float4(-9.0f, -10.0f, -11.0f, -12.0f), float4(-13.0f, -14.0f, -15.0f, -16.0f));
    float4x4 x = _19_testMatrix4x4;
    float4 _152 = -_19_testMatrix4x4[0];
    float4 _154 = -_19_testMatrix4x4[1];
    float4 _156 = -_19_testMatrix4x4[2];
    float4 _158 = -_19_testMatrix4x4[3];
    x = float4x4(_152, _154, _156, _158);
    return ((all(bool4(_152.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).x, _152.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).y, _152.z == float4(-1.0f, -2.0f, -3.0f, -4.0f).z, _152.w == float4(-1.0f, -2.0f, -3.0f, -4.0f).w)) && all(bool4(_154.x == float4(-5.0f, -6.0f, -7.0f, -8.0f).x, _154.y == float4(-5.0f, -6.0f, -7.0f, -8.0f).y, _154.z == float4(-5.0f, -6.0f, -7.0f, -8.0f).z, _154.w == float4(-5.0f, -6.0f, -7.0f, -8.0f).w))) && all(bool4(_156.x == float4(-9.0f, -10.0f, -11.0f, -12.0f).x, _156.y == float4(-9.0f, -10.0f, -11.0f, -12.0f).y, _156.z == float4(-9.0f, -10.0f, -11.0f, -12.0f).z, _156.w == float4(-9.0f, -10.0f, -11.0f, -12.0f).w))) && all(bool4(_158.x == float4(-13.0f, -14.0f, -15.0f, -16.0f).x, _158.y == float4(-13.0f, -14.0f, -15.0f, -16.0f).y, _158.z == float4(-13.0f, -14.0f, -15.0f, -16.0f).z, _158.w == float4(-13.0f, -14.0f, -15.0f, -16.0f).w));
}

bool test_hmat2_b()
{
    float2x2 negated = float2x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f));
    float2x2 x = _19_testMatrix2x2;
    float2 _178 = -_19_testMatrix2x2[0];
    float2 _180 = -_19_testMatrix2x2[1];
    x = float2x2(_178, _180);
    return all(bool2(_178.x == float2(-1.0f, -2.0f).x, _178.y == float2(-1.0f, -2.0f).y)) && all(bool2(_180.x == float2(-3.0f, -4.0f).x, _180.y == float2(-3.0f, -4.0f).y));
}

bool test_hmat3_b()
{
    float3x3 negated = float3x3(float3(-1.0f, -2.0f, -3.0f), float3(-4.0f, -5.0f, -6.0f), float3(-7.0f, -8.0f, -9.0f));
    float3x3 x = _19_testMatrix3x3;
    float3 _193 = -_19_testMatrix3x3[0];
    float3 _195 = -_19_testMatrix3x3[1];
    float3 _197 = -_19_testMatrix3x3[2];
    x = float3x3(_193, _195, _197);
    return (all(bool3(_193.x == float3(-1.0f, -2.0f, -3.0f).x, _193.y == float3(-1.0f, -2.0f, -3.0f).y, _193.z == float3(-1.0f, -2.0f, -3.0f).z)) && all(bool3(_195.x == float3(-4.0f, -5.0f, -6.0f).x, _195.y == float3(-4.0f, -5.0f, -6.0f).y, _195.z == float3(-4.0f, -5.0f, -6.0f).z))) && all(bool3(_197.x == float3(-7.0f, -8.0f, -9.0f).x, _197.y == float3(-7.0f, -8.0f, -9.0f).y, _197.z == float3(-7.0f, -8.0f, -9.0f).z));
}

bool test_hmat4_b()
{
    float4x4 negated = float4x4(float4(-1.0f, -2.0f, -3.0f, -4.0f), float4(-5.0f, -6.0f, -7.0f, -8.0f), float4(-9.0f, -10.0f, -11.0f, -12.0f), float4(-13.0f, -14.0f, -15.0f, -16.0f));
    float4x4 x = _19_testMatrix4x4;
    float4 _213 = -_19_testMatrix4x4[0];
    float4 _215 = -_19_testMatrix4x4[1];
    float4 _217 = -_19_testMatrix4x4[2];
    float4 _219 = -_19_testMatrix4x4[3];
    x = float4x4(_213, _215, _217, _219);
    return ((all(bool4(_213.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).x, _213.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).y, _213.z == float4(-1.0f, -2.0f, -3.0f, -4.0f).z, _213.w == float4(-1.0f, -2.0f, -3.0f, -4.0f).w)) && all(bool4(_215.x == float4(-5.0f, -6.0f, -7.0f, -8.0f).x, _215.y == float4(-5.0f, -6.0f, -7.0f, -8.0f).y, _215.z == float4(-5.0f, -6.0f, -7.0f, -8.0f).z, _215.w == float4(-5.0f, -6.0f, -7.0f, -8.0f).w))) && all(bool4(_217.x == float4(-9.0f, -10.0f, -11.0f, -12.0f).x, _217.y == float4(-9.0f, -10.0f, -11.0f, -12.0f).y, _217.z == float4(-9.0f, -10.0f, -11.0f, -12.0f).z, _217.w == float4(-9.0f, -10.0f, -11.0f, -12.0f).w))) && all(bool4(_219.x == float4(-13.0f, -14.0f, -15.0f, -16.0f).x, _219.y == float4(-13.0f, -14.0f, -15.0f, -16.0f).y, _219.z == float4(-13.0f, -14.0f, -15.0f, -16.0f).z, _219.w == float4(-13.0f, -14.0f, -15.0f, -16.0f).w));
}

float4 main(float2 _233)
{
    float _RESERVED_IDENTIFIER_FIXUP_0_x = _19_colorWhite.x;
    float _240 = -_19_colorWhite.x;
    _RESERVED_IDENTIFIER_FIXUP_0_x = _240;
    bool _246 = false;
    if (_240 == (-1.0f))
    {
        _246 = test_iscalar_b();
    }
    else
    {
        _246 = false;
    }
    bool _250 = false;
    if (_246)
    {
        _250 = test_fvec_b();
    }
    else
    {
        _250 = false;
    }
    bool _254 = false;
    if (_250)
    {
        _254 = test_ivec_b();
    }
    else
    {
        _254 = false;
    }
    bool _258 = false;
    if (_254)
    {
        _258 = test_mat2_b();
    }
    else
    {
        _258 = false;
    }
    bool _262 = false;
    if (_258)
    {
        _262 = test_mat3_b();
    }
    else
    {
        _262 = false;
    }
    bool _266 = false;
    if (_262)
    {
        _266 = test_mat4_b();
    }
    else
    {
        _266 = false;
    }
    bool _270 = false;
    if (_266)
    {
        _270 = test_hmat2_b();
    }
    else
    {
        _270 = false;
    }
    bool _274 = false;
    if (_270)
    {
        _274 = test_hmat3_b();
    }
    else
    {
        _274 = false;
    }
    bool _278 = false;
    if (_274)
    {
        _278 = test_hmat4_b();
    }
    else
    {
        _278 = false;
    }
    float4 _279 = 0.0f.xxxx;
    if (_278)
    {
        _279 = _19_colorGreen;
    }
    else
    {
        _279 = _19_colorRed;
    }
    return _279;
}

void frag_main()
{
    float2 _33 = 0.0f.xx;
    sk_FragColor = main(_33);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
