cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _20_colorWhite : packoffset(c0);
    float4 _20_colorGreen : packoffset(c1);
    float4 _20_colorRed : packoffset(c2);
    row_major float2x2 _20_testMatrix2x2 : packoffset(c3);
    row_major float3x3 _20_testMatrix3x3 : packoffset(c5);
    row_major float4x4 _20_testMatrix4x4 : packoffset(c8);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_iscalar_b()
{
    int _47 = int(_20_colorWhite.x);
    int x = _47;
    int _48 = -_47;
    x = _48;
    return _48 == (-1);
}

bool test_fvec_b()
{
    float2 x = _20_colorWhite.xy;
    float2 _56 = -_20_colorWhite.xy;
    x = _56;
    return all(bool2(_56.x == (-1.0f).xx.x, _56.y == (-1.0f).xx.y));
}

bool test_ivec_b()
{
    int2 _70 = int(_20_colorWhite.x).xx;
    int2 x = _70;
    int2 _71 = -_70;
    x = _71;
    return all(bool2(_71.x == int2(-1, -1).x, _71.y == int2(-1, -1).y));
}

bool test_mat2_b()
{
    float2x2 negated = float2x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f));
    float2x2 x = _20_testMatrix2x2;
    float2 _90 = -_20_testMatrix2x2[0];
    float2 _92 = -_20_testMatrix2x2[1];
    x = float2x2(_90, _92);
    return all(bool2(_90.x == float2(-1.0f, -2.0f).x, _90.y == float2(-1.0f, -2.0f).y)) && all(bool2(_92.x == float2(-3.0f, -4.0f).x, _92.y == float2(-3.0f, -4.0f).y));
}

bool test_mat3_b()
{
    float3x3 negated = float3x3(float3(-1.0f, -2.0f, -3.0f), float3(-4.0f, -5.0f, -6.0f), float3(-7.0f, -8.0f, -9.0f));
    float3x3 x = _20_testMatrix3x3;
    float3 _117 = -_20_testMatrix3x3[0];
    float3 _119 = -_20_testMatrix3x3[1];
    float3 _121 = -_20_testMatrix3x3[2];
    x = float3x3(_117, _119, _121);
    return (all(bool3(_117.x == float3(-1.0f, -2.0f, -3.0f).x, _117.y == float3(-1.0f, -2.0f, -3.0f).y, _117.z == float3(-1.0f, -2.0f, -3.0f).z)) && all(bool3(_119.x == float3(-4.0f, -5.0f, -6.0f).x, _119.y == float3(-4.0f, -5.0f, -6.0f).y, _119.z == float3(-4.0f, -5.0f, -6.0f).z))) && all(bool3(_121.x == float3(-7.0f, -8.0f, -9.0f).x, _121.y == float3(-7.0f, -8.0f, -9.0f).y, _121.z == float3(-7.0f, -8.0f, -9.0f).z));
}

bool test_mat4_b()
{
    float4x4 negated = float4x4(float4(-1.0f, -2.0f, -3.0f, -4.0f), float4(-5.0f, -6.0f, -7.0f, -8.0f), float4(-9.0f, -10.0f, -11.0f, -12.0f), float4(-13.0f, -14.0f, -15.0f, -16.0f));
    float4x4 x = _20_testMatrix4x4;
    float4 _153 = -_20_testMatrix4x4[0];
    float4 _155 = -_20_testMatrix4x4[1];
    float4 _157 = -_20_testMatrix4x4[2];
    float4 _159 = -_20_testMatrix4x4[3];
    x = float4x4(_153, _155, _157, _159);
    return ((all(bool4(_153.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).x, _153.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).y, _153.z == float4(-1.0f, -2.0f, -3.0f, -4.0f).z, _153.w == float4(-1.0f, -2.0f, -3.0f, -4.0f).w)) && all(bool4(_155.x == float4(-5.0f, -6.0f, -7.0f, -8.0f).x, _155.y == float4(-5.0f, -6.0f, -7.0f, -8.0f).y, _155.z == float4(-5.0f, -6.0f, -7.0f, -8.0f).z, _155.w == float4(-5.0f, -6.0f, -7.0f, -8.0f).w))) && all(bool4(_157.x == float4(-9.0f, -10.0f, -11.0f, -12.0f).x, _157.y == float4(-9.0f, -10.0f, -11.0f, -12.0f).y, _157.z == float4(-9.0f, -10.0f, -11.0f, -12.0f).z, _157.w == float4(-9.0f, -10.0f, -11.0f, -12.0f).w))) && all(bool4(_159.x == float4(-13.0f, -14.0f, -15.0f, -16.0f).x, _159.y == float4(-13.0f, -14.0f, -15.0f, -16.0f).y, _159.z == float4(-13.0f, -14.0f, -15.0f, -16.0f).z, _159.w == float4(-13.0f, -14.0f, -15.0f, -16.0f).w));
}

bool test_hmat2_b()
{
    float2x2 negated = float2x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f));
    float2x2 x = _20_testMatrix2x2;
    float2 _179 = -_20_testMatrix2x2[0];
    float2 _181 = -_20_testMatrix2x2[1];
    x = float2x2(_179, _181);
    return all(bool2(_179.x == float2(-1.0f, -2.0f).x, _179.y == float2(-1.0f, -2.0f).y)) && all(bool2(_181.x == float2(-3.0f, -4.0f).x, _181.y == float2(-3.0f, -4.0f).y));
}

bool test_hmat3_b()
{
    float3x3 negated = float3x3(float3(-1.0f, -2.0f, -3.0f), float3(-4.0f, -5.0f, -6.0f), float3(-7.0f, -8.0f, -9.0f));
    float3x3 x = _20_testMatrix3x3;
    float3 _194 = -_20_testMatrix3x3[0];
    float3 _196 = -_20_testMatrix3x3[1];
    float3 _198 = -_20_testMatrix3x3[2];
    x = float3x3(_194, _196, _198);
    return (all(bool3(_194.x == float3(-1.0f, -2.0f, -3.0f).x, _194.y == float3(-1.0f, -2.0f, -3.0f).y, _194.z == float3(-1.0f, -2.0f, -3.0f).z)) && all(bool3(_196.x == float3(-4.0f, -5.0f, -6.0f).x, _196.y == float3(-4.0f, -5.0f, -6.0f).y, _196.z == float3(-4.0f, -5.0f, -6.0f).z))) && all(bool3(_198.x == float3(-7.0f, -8.0f, -9.0f).x, _198.y == float3(-7.0f, -8.0f, -9.0f).y, _198.z == float3(-7.0f, -8.0f, -9.0f).z));
}

bool test_hmat4_b()
{
    float4x4 negated = float4x4(float4(-1.0f, -2.0f, -3.0f, -4.0f), float4(-5.0f, -6.0f, -7.0f, -8.0f), float4(-9.0f, -10.0f, -11.0f, -12.0f), float4(-13.0f, -14.0f, -15.0f, -16.0f));
    float4x4 x = _20_testMatrix4x4;
    float4 _214 = -_20_testMatrix4x4[0];
    float4 _216 = -_20_testMatrix4x4[1];
    float4 _218 = -_20_testMatrix4x4[2];
    float4 _220 = -_20_testMatrix4x4[3];
    x = float4x4(_214, _216, _218, _220);
    return ((all(bool4(_214.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).x, _214.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).y, _214.z == float4(-1.0f, -2.0f, -3.0f, -4.0f).z, _214.w == float4(-1.0f, -2.0f, -3.0f, -4.0f).w)) && all(bool4(_216.x == float4(-5.0f, -6.0f, -7.0f, -8.0f).x, _216.y == float4(-5.0f, -6.0f, -7.0f, -8.0f).y, _216.z == float4(-5.0f, -6.0f, -7.0f, -8.0f).z, _216.w == float4(-5.0f, -6.0f, -7.0f, -8.0f).w))) && all(bool4(_218.x == float4(-9.0f, -10.0f, -11.0f, -12.0f).x, _218.y == float4(-9.0f, -10.0f, -11.0f, -12.0f).y, _218.z == float4(-9.0f, -10.0f, -11.0f, -12.0f).z, _218.w == float4(-9.0f, -10.0f, -11.0f, -12.0f).w))) && all(bool4(_220.x == float4(-13.0f, -14.0f, -15.0f, -16.0f).x, _220.y == float4(-13.0f, -14.0f, -15.0f, -16.0f).y, _220.z == float4(-13.0f, -14.0f, -15.0f, -16.0f).z, _220.w == float4(-13.0f, -14.0f, -15.0f, -16.0f).w));
}

float4 main(float2 _234)
{
    float _RESERVED_IDENTIFIER_FIXUP_0_x = _20_colorWhite.x;
    float _241 = -_20_colorWhite.x;
    _RESERVED_IDENTIFIER_FIXUP_0_x = _241;
    bool _247 = false;
    if (_241 == (-1.0f))
    {
        _247 = test_iscalar_b();
    }
    else
    {
        _247 = false;
    }
    bool _251 = false;
    if (_247)
    {
        _251 = test_fvec_b();
    }
    else
    {
        _251 = false;
    }
    bool _255 = false;
    if (_251)
    {
        _255 = test_ivec_b();
    }
    else
    {
        _255 = false;
    }
    bool _259 = false;
    if (_255)
    {
        _259 = test_mat2_b();
    }
    else
    {
        _259 = false;
    }
    bool _263 = false;
    if (_259)
    {
        _263 = test_mat3_b();
    }
    else
    {
        _263 = false;
    }
    bool _267 = false;
    if (_263)
    {
        _267 = test_mat4_b();
    }
    else
    {
        _267 = false;
    }
    bool _271 = false;
    if (_267)
    {
        _271 = test_hmat2_b();
    }
    else
    {
        _271 = false;
    }
    bool _275 = false;
    if (_271)
    {
        _275 = test_hmat3_b();
    }
    else
    {
        _275 = false;
    }
    bool _279 = false;
    if (_275)
    {
        _279 = test_hmat4_b();
    }
    else
    {
        _279 = false;
    }
    float4 _280 = 0.0f.xxxx;
    if (_279)
    {
        _280 = _20_colorGreen;
    }
    else
    {
        _280 = _20_colorRed;
    }
    return _280;
}

void frag_main()
{
    float2 _34 = 0.0f.xx;
    sk_FragColor = main(_34);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
