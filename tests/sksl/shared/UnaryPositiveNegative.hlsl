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
    int _44 = int(_16_colorWhite.x);
    int x = _44;
    int _45 = -_44;
    x = _45;
    return _45 == (-1);
}

bool test_fvec_b()
{
    float2 x = _16_colorWhite.xy;
    float2 _53 = -_16_colorWhite.xy;
    x = _53;
    return all(bool2(_53.x == (-1.0f).xx.x, _53.y == (-1.0f).xx.y));
}

bool test_ivec_b()
{
    int2 _67 = int(_16_colorWhite.x).xx;
    int2 x = _67;
    int2 _68 = -_67;
    x = _68;
    return all(bool2(_68.x == int2(-1, -1).x, _68.y == int2(-1, -1).y));
}

bool test_mat2_b()
{
    float2x2 negated = float2x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f));
    float2x2 x = _16_testMatrix2x2;
    float2 _87 = -_16_testMatrix2x2[0];
    float2 _89 = -_16_testMatrix2x2[1];
    x = float2x2(_87, _89);
    return all(bool2(_87.x == float2(-1.0f, -2.0f).x, _87.y == float2(-1.0f, -2.0f).y)) && all(bool2(_89.x == float2(-3.0f, -4.0f).x, _89.y == float2(-3.0f, -4.0f).y));
}

bool test_mat3_b()
{
    float3x3 negated = float3x3(float3(-1.0f, -2.0f, -3.0f), float3(-4.0f, -5.0f, -6.0f), float3(-7.0f, -8.0f, -9.0f));
    float3x3 x = _16_testMatrix3x3;
    float3 _114 = -_16_testMatrix3x3[0];
    float3 _116 = -_16_testMatrix3x3[1];
    float3 _118 = -_16_testMatrix3x3[2];
    x = float3x3(_114, _116, _118);
    return (all(bool3(_114.x == float3(-1.0f, -2.0f, -3.0f).x, _114.y == float3(-1.0f, -2.0f, -3.0f).y, _114.z == float3(-1.0f, -2.0f, -3.0f).z)) && all(bool3(_116.x == float3(-4.0f, -5.0f, -6.0f).x, _116.y == float3(-4.0f, -5.0f, -6.0f).y, _116.z == float3(-4.0f, -5.0f, -6.0f).z))) && all(bool3(_118.x == float3(-7.0f, -8.0f, -9.0f).x, _118.y == float3(-7.0f, -8.0f, -9.0f).y, _118.z == float3(-7.0f, -8.0f, -9.0f).z));
}

bool test_mat4_b()
{
    float4x4 negated = float4x4(float4(-1.0f, -2.0f, -3.0f, -4.0f), float4(-5.0f, -6.0f, -7.0f, -8.0f), float4(-9.0f, -10.0f, -11.0f, -12.0f), float4(-13.0f, -14.0f, -15.0f, -16.0f));
    float4x4 x = _16_testMatrix4x4;
    float4 _150 = -_16_testMatrix4x4[0];
    float4 _152 = -_16_testMatrix4x4[1];
    float4 _154 = -_16_testMatrix4x4[2];
    float4 _156 = -_16_testMatrix4x4[3];
    x = float4x4(_150, _152, _154, _156);
    return ((all(bool4(_150.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).x, _150.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).y, _150.z == float4(-1.0f, -2.0f, -3.0f, -4.0f).z, _150.w == float4(-1.0f, -2.0f, -3.0f, -4.0f).w)) && all(bool4(_152.x == float4(-5.0f, -6.0f, -7.0f, -8.0f).x, _152.y == float4(-5.0f, -6.0f, -7.0f, -8.0f).y, _152.z == float4(-5.0f, -6.0f, -7.0f, -8.0f).z, _152.w == float4(-5.0f, -6.0f, -7.0f, -8.0f).w))) && all(bool4(_154.x == float4(-9.0f, -10.0f, -11.0f, -12.0f).x, _154.y == float4(-9.0f, -10.0f, -11.0f, -12.0f).y, _154.z == float4(-9.0f, -10.0f, -11.0f, -12.0f).z, _154.w == float4(-9.0f, -10.0f, -11.0f, -12.0f).w))) && all(bool4(_156.x == float4(-13.0f, -14.0f, -15.0f, -16.0f).x, _156.y == float4(-13.0f, -14.0f, -15.0f, -16.0f).y, _156.z == float4(-13.0f, -14.0f, -15.0f, -16.0f).z, _156.w == float4(-13.0f, -14.0f, -15.0f, -16.0f).w));
}

bool test_hmat2_b()
{
    float2x2 negated = float2x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f));
    float2x2 x = _16_testMatrix2x2;
    float2 _176 = -_16_testMatrix2x2[0];
    float2 _178 = -_16_testMatrix2x2[1];
    x = float2x2(_176, _178);
    return all(bool2(_176.x == float2(-1.0f, -2.0f).x, _176.y == float2(-1.0f, -2.0f).y)) && all(bool2(_178.x == float2(-3.0f, -4.0f).x, _178.y == float2(-3.0f, -4.0f).y));
}

bool test_hmat3_b()
{
    float3x3 negated = float3x3(float3(-1.0f, -2.0f, -3.0f), float3(-4.0f, -5.0f, -6.0f), float3(-7.0f, -8.0f, -9.0f));
    float3x3 x = _16_testMatrix3x3;
    float3 _191 = -_16_testMatrix3x3[0];
    float3 _193 = -_16_testMatrix3x3[1];
    float3 _195 = -_16_testMatrix3x3[2];
    x = float3x3(_191, _193, _195);
    return (all(bool3(_191.x == float3(-1.0f, -2.0f, -3.0f).x, _191.y == float3(-1.0f, -2.0f, -3.0f).y, _191.z == float3(-1.0f, -2.0f, -3.0f).z)) && all(bool3(_193.x == float3(-4.0f, -5.0f, -6.0f).x, _193.y == float3(-4.0f, -5.0f, -6.0f).y, _193.z == float3(-4.0f, -5.0f, -6.0f).z))) && all(bool3(_195.x == float3(-7.0f, -8.0f, -9.0f).x, _195.y == float3(-7.0f, -8.0f, -9.0f).y, _195.z == float3(-7.0f, -8.0f, -9.0f).z));
}

bool test_hmat4_b()
{
    float4x4 negated = float4x4(float4(-1.0f, -2.0f, -3.0f, -4.0f), float4(-5.0f, -6.0f, -7.0f, -8.0f), float4(-9.0f, -10.0f, -11.0f, -12.0f), float4(-13.0f, -14.0f, -15.0f, -16.0f));
    float4x4 x = _16_testMatrix4x4;
    float4 _211 = -_16_testMatrix4x4[0];
    float4 _213 = -_16_testMatrix4x4[1];
    float4 _215 = -_16_testMatrix4x4[2];
    float4 _217 = -_16_testMatrix4x4[3];
    x = float4x4(_211, _213, _215, _217);
    return ((all(bool4(_211.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).x, _211.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).y, _211.z == float4(-1.0f, -2.0f, -3.0f, -4.0f).z, _211.w == float4(-1.0f, -2.0f, -3.0f, -4.0f).w)) && all(bool4(_213.x == float4(-5.0f, -6.0f, -7.0f, -8.0f).x, _213.y == float4(-5.0f, -6.0f, -7.0f, -8.0f).y, _213.z == float4(-5.0f, -6.0f, -7.0f, -8.0f).z, _213.w == float4(-5.0f, -6.0f, -7.0f, -8.0f).w))) && all(bool4(_215.x == float4(-9.0f, -10.0f, -11.0f, -12.0f).x, _215.y == float4(-9.0f, -10.0f, -11.0f, -12.0f).y, _215.z == float4(-9.0f, -10.0f, -11.0f, -12.0f).z, _215.w == float4(-9.0f, -10.0f, -11.0f, -12.0f).w))) && all(bool4(_217.x == float4(-13.0f, -14.0f, -15.0f, -16.0f).x, _217.y == float4(-13.0f, -14.0f, -15.0f, -16.0f).y, _217.z == float4(-13.0f, -14.0f, -15.0f, -16.0f).z, _217.w == float4(-13.0f, -14.0f, -15.0f, -16.0f).w));
}

float4 main(float2 _231)
{
    float _RESERVED_IDENTIFIER_FIXUP_0_x = _16_colorWhite.x;
    float _238 = -_16_colorWhite.x;
    _RESERVED_IDENTIFIER_FIXUP_0_x = _238;
    bool _244 = false;
    if (_238 == (-1.0f))
    {
        _244 = test_iscalar_b();
    }
    else
    {
        _244 = false;
    }
    bool _248 = false;
    if (_244)
    {
        _248 = test_fvec_b();
    }
    else
    {
        _248 = false;
    }
    bool _252 = false;
    if (_248)
    {
        _252 = test_ivec_b();
    }
    else
    {
        _252 = false;
    }
    bool _256 = false;
    if (_252)
    {
        _256 = test_mat2_b();
    }
    else
    {
        _256 = false;
    }
    bool _260 = false;
    if (_256)
    {
        _260 = test_mat3_b();
    }
    else
    {
        _260 = false;
    }
    bool _264 = false;
    if (_260)
    {
        _264 = test_mat4_b();
    }
    else
    {
        _264 = false;
    }
    bool _268 = false;
    if (_264)
    {
        _268 = test_hmat2_b();
    }
    else
    {
        _268 = false;
    }
    bool _272 = false;
    if (_268)
    {
        _272 = test_hmat3_b();
    }
    else
    {
        _272 = false;
    }
    bool _276 = false;
    if (_272)
    {
        _276 = test_hmat4_b();
    }
    else
    {
        _276 = false;
    }
    float4 _277 = 0.0f.xxxx;
    if (_276)
    {
        _277 = _16_colorGreen;
    }
    else
    {
        _277 = _16_colorRed;
    }
    return _277;
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
