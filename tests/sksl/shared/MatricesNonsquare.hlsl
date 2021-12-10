cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_half_b()
{
    bool ok = true;
    float2x3 m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
    bool _55 = false;
    if (ok)
    {
        float2x3 _44 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
        float3 _47 = _44[0];
        float3 _51 = _44[1];
        _55 = all(bool3(m23[0].x == _47.x, m23[0].y == _47.y, m23[0].z == _47.z)) && all(bool3(m23[1].x == _51.x, m23[1].y == _51.y, m23[1].z == _51.z));
    }
    else
    {
        _55 = false;
    }
    ok = _55;
    float2x4 m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _80 = false;
    if (ok)
    {
        float2x4 _69 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
        float4 _72 = _69[0];
        float4 _76 = _69[1];
        _80 = all(bool4(m24[0].x == _72.x, m24[0].y == _72.y, m24[0].z == _72.z, m24[0].w == _72.w)) && all(bool4(m24[1].x == _76.x, m24[1].y == _76.y, m24[1].z == _76.z, m24[1].w == _76.w));
    }
    else
    {
        _80 = false;
    }
    ok = _80;
    float3x2 m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _112 = false;
    if (ok)
    {
        float3x2 _96 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
        float2 _99 = _96[0];
        float2 _103 = _96[1];
        float2 _108 = _96[2];
        _112 = (all(bool2(m32[0].x == _99.x, m32[0].y == _99.y)) && all(bool2(m32[1].x == _103.x, m32[1].y == _103.y))) && all(bool2(m32[2].x == _108.x, m32[2].y == _108.y));
    }
    else
    {
        _112 = false;
    }
    ok = _112;
    float3x4 m34 = float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f));
    bool _143 = false;
    if (ok)
    {
        float3x4 _128 = float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f));
        float4 _130 = _128[0];
        float4 _134 = _128[1];
        float4 _139 = _128[2];
        _143 = (all(bool4(m34[0].x == _130.x, m34[0].y == _130.y, m34[0].z == _130.z, m34[0].w == _130.w)) && all(bool4(m34[1].x == _134.x, m34[1].y == _134.y, m34[1].z == _134.z, m34[1].w == _134.w))) && all(bool4(m34[2].x == _139.x, m34[2].y == _139.y, m34[2].z == _139.z, m34[2].w == _139.w));
    }
    else
    {
        _143 = false;
    }
    ok = _143;
    float4x2 m42 = float4x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f), 0.0f.xx, 0.0f.xx);
    bool _181 = false;
    if (ok)
    {
        float4x2 _161 = float4x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f), 0.0f.xx, 0.0f.xx);
        float2 _163 = _161[0];
        float2 _167 = _161[1];
        float2 _172 = _161[2];
        float2 _177 = _161[3];
        _181 = ((all(bool2(m42[0].x == _163.x, m42[0].y == _163.y)) && all(bool2(m42[1].x == _167.x, m42[1].y == _167.y))) && all(bool2(m42[2].x == _172.x, m42[2].y == _172.y))) && all(bool2(m42[3].x == _177.x, m42[3].y == _177.y));
    }
    else
    {
        _181 = false;
    }
    ok = _181;
    float4x3 m43 = float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx);
    bool _219 = false;
    if (ok)
    {
        float4x3 _199 = float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx);
        float3 _201 = _199[0];
        float3 _205 = _199[1];
        float3 _210 = _199[2];
        float3 _215 = _199[3];
        _219 = ((all(bool3(m43[0].x == _201.x, m43[0].y == _201.y, m43[0].z == _201.z)) && all(bool3(m43[1].x == _205.x, m43[1].y == _205.y, m43[1].z == _205.z))) && all(bool3(m43[2].x == _210.x, m43[2].y == _210.y, m43[2].z == _210.z))) && all(bool3(m43[3].x == _215.x, m43[3].y == _215.y, m43[3].z == _215.z));
    }
    else
    {
        _219 = false;
    }
    ok = _219;
    float2x2 m22 = mul(m23, m32);
    bool _243 = false;
    if (ok)
    {
        float2x2 _231 = float2x2(float2(8.0f, 0.0f), float2(0.0f, 8.0f));
        float2 _235 = _231[0];
        float2 _239 = _231[1];
        _243 = all(bool2(m22[0].x == _235.x, m22[0].y == _235.y)) && all(bool2(m22[1].x == _239.x, m22[1].y == _239.y));
    }
    else
    {
        _243 = false;
    }
    ok = _243;
    float3x3 m33 = mul(m34, m43);
    bool _273 = false;
    if (ok)
    {
        float3x3 _255 = float3x3(float3(35.0f, 0.0f, 0.0f), float3(0.0f, 35.0f, 0.0f), float3(0.0f, 0.0f, 35.0f));
        float3 _260 = _255[0];
        float3 _264 = _255[1];
        float3 _269 = _255[2];
        _273 = (all(bool3(m33[0].x == _260.x, m33[0].y == _260.y, m33[0].z == _260.z)) && all(bool3(m33[1].x == _264.x, m33[1].y == _264.y, m33[1].z == _264.z))) && all(bool3(m33[2].x == _269.x, m33[2].y == _269.y, m33[2].z == _269.z));
    }
    else
    {
        _273 = false;
    }
    ok = _273;
    float3 _276 = 1.0f.xxx;
    float2x3 _277 = float2x3(_276, _276);
    m23 = float2x3(m23[0] + _277[0], m23[1] + _277[1]);
    bool _301 = false;
    if (ok)
    {
        float2x3 _291 = float2x3(float3(3.0f, 1.0f, 1.0f), float3(1.0f, 3.0f, 1.0f));
        float3 _293 = _291[0];
        float3 _297 = _291[1];
        _301 = all(bool3(m23[0].x == _293.x, m23[0].y == _293.y, m23[0].z == _293.z)) && all(bool3(m23[1].x == _297.x, m23[1].y == _297.y, m23[1].z == _297.z));
    }
    else
    {
        _301 = false;
    }
    ok = _301;
    float2 _303 = 2.0f.xx;
    float3x2 _304 = float3x2(_303, _303, _303);
    m32 = float3x2(m32[0] - _304[0], m32[1] - _304[1], m32[2] - _304[2]);
    bool _338 = false;
    if (ok)
    {
        float3x2 _323 = float3x2(float2(2.0f, -2.0f), float2(-2.0f, 2.0f), (-2.0f).xx);
        float2 _325 = _323[0];
        float2 _329 = _323[1];
        float2 _334 = _323[2];
        _338 = (all(bool2(m32[0].x == _325.x, m32[0].y == _325.y)) && all(bool2(m32[1].x == _329.x, m32[1].y == _329.y))) && all(bool2(m32[2].x == _334.x, m32[2].y == _334.y));
    }
    else
    {
        _338 = false;
    }
    ok = _338;
    float4 _340 = 4.0f.xxxx;
    float2x4 _341 = float2x4(_340, _340);
    m24 = float2x4(m24[0] / _341[0], m24[1] / _341[1]);
    bool _366 = false;
    if (ok)
    {
        float2x4 _356 = float2x4(float4(0.75f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.75f, 0.0f, 0.0f));
        float4 _358 = _356[0];
        float4 _362 = _356[1];
        _366 = all(bool4(m24[0].x == _358.x, m24[0].y == _358.y, m24[0].z == _358.z, m24[0].w == _358.w)) && all(bool4(m24[1].x == _362.x, m24[1].y == _362.y, m24[1].z == _362.z, m24[1].w == _362.w));
    }
    else
    {
        _366 = false;
    }
    ok = _366;
    return ok;
}

float4 main(float2 _369)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
    bool _392 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x3 _382 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
        float3 _384 = _382[0];
        float3 _388 = _382[1];
        _392 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_1_m23[0].x == _384.x, _RESERVED_IDENTIFIER_FIXUP_1_m23[0].y == _384.y, _RESERVED_IDENTIFIER_FIXUP_1_m23[0].z == _384.z)) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_1_m23[1].x == _388.x, _RESERVED_IDENTIFIER_FIXUP_1_m23[1].y == _388.y, _RESERVED_IDENTIFIER_FIXUP_1_m23[1].z == _388.z));
    }
    else
    {
        _392 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _392;
    float2x4 _RESERVED_IDENTIFIER_FIXUP_2_m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _413 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x4 _403 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
        float4 _405 = _403[0];
        float4 _409 = _403[1];
        _413 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_2_m24[0].x == _405.x, _RESERVED_IDENTIFIER_FIXUP_2_m24[0].y == _405.y, _RESERVED_IDENTIFIER_FIXUP_2_m24[0].z == _405.z, _RESERVED_IDENTIFIER_FIXUP_2_m24[0].w == _405.w)) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_2_m24[1].x == _409.x, _RESERVED_IDENTIFIER_FIXUP_2_m24[1].y == _409.y, _RESERVED_IDENTIFIER_FIXUP_2_m24[1].z == _409.z, _RESERVED_IDENTIFIER_FIXUP_2_m24[1].w == _409.w));
    }
    else
    {
        _413 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _413;
    float3x2 _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _441 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x2 _426 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
        float2 _428 = _426[0];
        float2 _432 = _426[1];
        float2 _437 = _426[2];
        _441 = (all(bool2(_RESERVED_IDENTIFIER_FIXUP_3_m32[0].x == _428.x, _RESERVED_IDENTIFIER_FIXUP_3_m32[0].y == _428.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_3_m32[1].x == _432.x, _RESERVED_IDENTIFIER_FIXUP_3_m32[1].y == _432.y))) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_3_m32[2].x == _437.x, _RESERVED_IDENTIFIER_FIXUP_3_m32[2].y == _437.y));
    }
    else
    {
        _441 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _441;
    float3x4 _RESERVED_IDENTIFIER_FIXUP_4_m34 = float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f));
    bool _469 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x4 _454 = float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f));
        float4 _456 = _454[0];
        float4 _460 = _454[1];
        float4 _465 = _454[2];
        _469 = (all(bool4(_RESERVED_IDENTIFIER_FIXUP_4_m34[0].x == _456.x, _RESERVED_IDENTIFIER_FIXUP_4_m34[0].y == _456.y, _RESERVED_IDENTIFIER_FIXUP_4_m34[0].z == _456.z, _RESERVED_IDENTIFIER_FIXUP_4_m34[0].w == _456.w)) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_4_m34[1].x == _460.x, _RESERVED_IDENTIFIER_FIXUP_4_m34[1].y == _460.y, _RESERVED_IDENTIFIER_FIXUP_4_m34[1].z == _460.z, _RESERVED_IDENTIFIER_FIXUP_4_m34[1].w == _460.w))) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_4_m34[2].x == _465.x, _RESERVED_IDENTIFIER_FIXUP_4_m34[2].y == _465.y, _RESERVED_IDENTIFIER_FIXUP_4_m34[2].z == _465.z, _RESERVED_IDENTIFIER_FIXUP_4_m34[2].w == _465.w));
    }
    else
    {
        _469 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _469;
    float4x2 _RESERVED_IDENTIFIER_FIXUP_5_m42 = float4x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f), 0.0f.xx, 0.0f.xx);
    bool _504 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float4x2 _484 = float4x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f), 0.0f.xx, 0.0f.xx);
        float2 _486 = _484[0];
        float2 _490 = _484[1];
        float2 _495 = _484[2];
        float2 _500 = _484[3];
        _504 = ((all(bool2(_RESERVED_IDENTIFIER_FIXUP_5_m42[0].x == _486.x, _RESERVED_IDENTIFIER_FIXUP_5_m42[0].y == _486.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_5_m42[1].x == _490.x, _RESERVED_IDENTIFIER_FIXUP_5_m42[1].y == _490.y))) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_5_m42[2].x == _495.x, _RESERVED_IDENTIFIER_FIXUP_5_m42[2].y == _495.y))) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_5_m42[3].x == _500.x, _RESERVED_IDENTIFIER_FIXUP_5_m42[3].y == _500.y));
    }
    else
    {
        _504 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _504;
    float4x3 _RESERVED_IDENTIFIER_FIXUP_6_m43 = float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx);
    bool _539 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float4x3 _519 = float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx);
        float3 _521 = _519[0];
        float3 _525 = _519[1];
        float3 _530 = _519[2];
        float3 _535 = _519[3];
        _539 = ((all(bool3(_RESERVED_IDENTIFIER_FIXUP_6_m43[0].x == _521.x, _RESERVED_IDENTIFIER_FIXUP_6_m43[0].y == _521.y, _RESERVED_IDENTIFIER_FIXUP_6_m43[0].z == _521.z)) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_6_m43[1].x == _525.x, _RESERVED_IDENTIFIER_FIXUP_6_m43[1].y == _525.y, _RESERVED_IDENTIFIER_FIXUP_6_m43[1].z == _525.z))) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_6_m43[2].x == _530.x, _RESERVED_IDENTIFIER_FIXUP_6_m43[2].y == _530.y, _RESERVED_IDENTIFIER_FIXUP_6_m43[2].z == _530.z))) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_6_m43[3].x == _535.x, _RESERVED_IDENTIFIER_FIXUP_6_m43[3].y == _535.y, _RESERVED_IDENTIFIER_FIXUP_6_m43[3].z == _535.z));
    }
    else
    {
        _539 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _539;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_7_m22 = mul(_RESERVED_IDENTIFIER_FIXUP_1_m23, _RESERVED_IDENTIFIER_FIXUP_3_m32);
    bool _560 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _548 = float2x2(float2(8.0f, 0.0f), float2(0.0f, 8.0f));
        float2 _552 = _548[0];
        float2 _556 = _548[1];
        _560 = all(bool2(_RESERVED_IDENTIFIER_FIXUP_7_m22[0].x == _552.x, _RESERVED_IDENTIFIER_FIXUP_7_m22[0].y == _552.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_7_m22[1].x == _556.x, _RESERVED_IDENTIFIER_FIXUP_7_m22[1].y == _556.y));
    }
    else
    {
        _560 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _560;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_8_m33 = mul(_RESERVED_IDENTIFIER_FIXUP_4_m34, _RESERVED_IDENTIFIER_FIXUP_6_m43);
    bool _587 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _569 = float3x3(float3(35.0f, 0.0f, 0.0f), float3(0.0f, 35.0f, 0.0f), float3(0.0f, 0.0f, 35.0f));
        float3 _574 = _569[0];
        float3 _578 = _569[1];
        float3 _583 = _569[2];
        _587 = (all(bool3(_RESERVED_IDENTIFIER_FIXUP_8_m33[0].x == _574.x, _RESERVED_IDENTIFIER_FIXUP_8_m33[0].y == _574.y, _RESERVED_IDENTIFIER_FIXUP_8_m33[0].z == _574.z)) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_8_m33[1].x == _578.x, _RESERVED_IDENTIFIER_FIXUP_8_m33[1].y == _578.y, _RESERVED_IDENTIFIER_FIXUP_8_m33[1].z == _578.z))) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_8_m33[2].x == _583.x, _RESERVED_IDENTIFIER_FIXUP_8_m33[2].y == _583.y, _RESERVED_IDENTIFIER_FIXUP_8_m33[2].z == _583.z));
    }
    else
    {
        _587 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _587;
    float3 _589 = 1.0f.xxx;
    float2x3 _590 = float2x3(_589, _589);
    _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(_RESERVED_IDENTIFIER_FIXUP_1_m23[0] + _590[0], _RESERVED_IDENTIFIER_FIXUP_1_m23[1] + _590[1]);
    bool _614 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x3 _604 = float2x3(float3(3.0f, 1.0f, 1.0f), float3(1.0f, 3.0f, 1.0f));
        float3 _606 = _604[0];
        float3 _610 = _604[1];
        _614 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_1_m23[0].x == _606.x, _RESERVED_IDENTIFIER_FIXUP_1_m23[0].y == _606.y, _RESERVED_IDENTIFIER_FIXUP_1_m23[0].z == _606.z)) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_1_m23[1].x == _610.x, _RESERVED_IDENTIFIER_FIXUP_1_m23[1].y == _610.y, _RESERVED_IDENTIFIER_FIXUP_1_m23[1].z == _610.z));
    }
    else
    {
        _614 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _614;
    float2 _616 = 2.0f.xx;
    float3x2 _617 = float3x2(_616, _616, _616);
    _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(_RESERVED_IDENTIFIER_FIXUP_3_m32[0] - _617[0], _RESERVED_IDENTIFIER_FIXUP_3_m32[1] - _617[1], _RESERVED_IDENTIFIER_FIXUP_3_m32[2] - _617[2]);
    bool _650 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x2 _635 = float3x2(float2(2.0f, -2.0f), float2(-2.0f, 2.0f), (-2.0f).xx);
        float2 _637 = _635[0];
        float2 _641 = _635[1];
        float2 _646 = _635[2];
        _650 = (all(bool2(_RESERVED_IDENTIFIER_FIXUP_3_m32[0].x == _637.x, _RESERVED_IDENTIFIER_FIXUP_3_m32[0].y == _637.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_3_m32[1].x == _641.x, _RESERVED_IDENTIFIER_FIXUP_3_m32[1].y == _641.y))) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_3_m32[2].x == _646.x, _RESERVED_IDENTIFIER_FIXUP_3_m32[2].y == _646.y));
    }
    else
    {
        _650 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _650;
    float4 _652 = 4.0f.xxxx;
    float2x4 _653 = float2x4(_652, _652);
    _RESERVED_IDENTIFIER_FIXUP_2_m24 = float2x4(_RESERVED_IDENTIFIER_FIXUP_2_m24[0] / _653[0], _RESERVED_IDENTIFIER_FIXUP_2_m24[1] / _653[1]);
    bool _677 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x4 _667 = float2x4(float4(0.75f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.75f, 0.0f, 0.0f));
        float4 _669 = _667[0];
        float4 _673 = _667[1];
        _677 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_2_m24[0].x == _669.x, _RESERVED_IDENTIFIER_FIXUP_2_m24[0].y == _669.y, _RESERVED_IDENTIFIER_FIXUP_2_m24[0].z == _669.z, _RESERVED_IDENTIFIER_FIXUP_2_m24[0].w == _669.w)) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_2_m24[1].x == _673.x, _RESERVED_IDENTIFIER_FIXUP_2_m24[1].y == _673.y, _RESERVED_IDENTIFIER_FIXUP_2_m24[1].z == _673.z, _RESERVED_IDENTIFIER_FIXUP_2_m24[1].w == _673.w));
    }
    else
    {
        _677 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _677;
    bool _682 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _682 = test_half_b();
    }
    else
    {
        _682 = false;
    }
    float4 _683 = 0.0f.xxxx;
    if (_682)
    {
        _683 = _11_colorGreen;
    }
    else
    {
        _683 = _11_colorRed;
    }
    return _683;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
