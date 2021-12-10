cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_half_b()
{
    bool ok = true;
    float2x2 m1 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _58 = false;
    if (ok)
    {
        float2x2 _47 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
        float2 _50 = _47[0];
        float2 _54 = _47[1];
        _58 = all(bool2(m1[0].x == _50.x, m1[0].y == _50.y)) && all(bool2(m1[1].x == _54.x, m1[1].y == _54.y));
    }
    else
    {
        _58 = false;
    }
    ok = _58;
    float2x2 m3 = m1;
    bool _77 = false;
    if (ok)
    {
        float2x2 _67 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
        float2 _69 = _67[0];
        float2 _73 = _67[1];
        _77 = all(bool2(m3[0].x == _69.x, m3[0].y == _69.y)) && all(bool2(m3[1].x == _73.x, m3[1].y == _73.y));
    }
    else
    {
        _77 = false;
    }
    ok = _77;
    float2x2 m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
    bool _99 = false;
    if (ok)
    {
        float2x2 _89 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
        float2 _91 = _89[0];
        float2 _95 = _89[1];
        _99 = all(bool2(m4[0].x == _91.x, m4[0].y == _91.y)) && all(bool2(m4[1].x == _95.x, m4[1].y == _95.y));
    }
    else
    {
        _99 = false;
    }
    ok = _99;
    m3 = mul(m4, m3);
    bool _122 = false;
    if (ok)
    {
        float2x2 _112 = float2x2(float2(6.0f, 12.0f), float2(18.0f, 24.0f));
        float2 _114 = _112[0];
        float2 _118 = _112[1];
        _122 = all(bool2(m3[0].x == _114.x, m3[0].y == _114.y)) && all(bool2(m3[1].x == _118.x, m3[1].y == _118.y));
    }
    else
    {
        _122 = false;
    }
    ok = _122;
    float2x2 m5 = float2x2(float2(m1[1].y, 0.0f), float2(0.0f, m1[1].y));
    bool _148 = false;
    if (ok)
    {
        float2x2 _138 = float2x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f));
        float2 _140 = _138[0];
        float2 _144 = _138[1];
        _148 = all(bool2(m5[0].x == _140.x, m5[0].y == _140.y)) && all(bool2(m5[1].x == _144.x, m5[1].y == _144.y));
    }
    else
    {
        _148 = false;
    }
    ok = _148;
    m1 = float2x2(m1[0] + m5[0], m1[1] + m5[1]);
    bool _176 = false;
    if (ok)
    {
        float2x2 _166 = float2x2(float2(5.0f, 2.0f), float2(3.0f, 8.0f));
        float2 _168 = _166[0];
        float2 _172 = _166[1];
        _176 = all(bool2(m1[0].x == _168.x, m1[0].y == _168.y)) && all(bool2(m1[1].x == _172.x, m1[1].y == _172.y));
    }
    else
    {
        _176 = false;
    }
    ok = _176;
    float2x2 m7 = float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f));
    bool _198 = false;
    if (ok)
    {
        float2x2 _188 = float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f));
        float2 _190 = _188[0];
        float2 _194 = _188[1];
        _198 = all(bool2(m7[0].x == _190.x, m7[0].y == _190.y)) && all(bool2(m7[1].x == _194.x, m7[1].y == _194.y));
    }
    else
    {
        _198 = false;
    }
    ok = _198;
    float3x3 m9 = float3x3(float3(9.0f, 0.0f, 0.0f), float3(0.0f, 9.0f, 0.0f), float3(0.0f, 0.0f, 9.0f));
    bool _231 = false;
    if (ok)
    {
        float3x3 _215 = float3x3(float3(9.0f, 0.0f, 0.0f), float3(0.0f, 9.0f, 0.0f), float3(0.0f, 0.0f, 9.0f));
        float3 _218 = _215[0];
        float3 _222 = _215[1];
        float3 _227 = _215[2];
        _231 = (all(bool3(m9[0].x == _218.x, m9[0].y == _218.y, m9[0].z == _218.z)) && all(bool3(m9[1].x == _222.x, m9[1].y == _222.y, m9[1].z == _222.z))) && all(bool3(m9[2].x == _227.x, m9[2].y == _227.y, m9[2].z == _227.z));
    }
    else
    {
        _231 = false;
    }
    ok = _231;
    float4x4 m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
    bool _270 = false;
    if (ok)
    {
        float4x4 _249 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
        float4 _252 = _249[0];
        float4 _256 = _249[1];
        float4 _261 = _249[2];
        float4 _266 = _249[3];
        _270 = ((all(bool4(m10[0].x == _252.x, m10[0].y == _252.y, m10[0].z == _252.z, m10[0].w == _252.w)) && all(bool4(m10[1].x == _256.x, m10[1].y == _256.y, m10[1].z == _256.z, m10[1].w == _256.w))) && all(bool4(m10[2].x == _261.x, m10[2].y == _261.y, m10[2].z == _261.z, m10[2].w == _261.w))) && all(bool4(m10[3].x == _266.x, m10[3].y == _266.y, m10[3].z == _266.z, m10[3].w == _266.w));
    }
    else
    {
        _270 = false;
    }
    ok = _270;
    float4x4 m11 = float4x4(20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx);
    m11 = float4x4(m11[0] - m10[0], m11[1] - m10[1], m11[2] - m10[2], m11[3] - m10[3]);
    bool _321 = false;
    if (ok)
    {
        float4x4 _301 = float4x4(float4(9.0f, 20.0f, 20.0f, 20.0f), float4(20.0f, 9.0f, 20.0f, 20.0f), float4(20.0f, 20.0f, 9.0f, 20.0f), float4(20.0f, 20.0f, 20.0f, 9.0f));
        float4 _303 = _301[0];
        float4 _307 = _301[1];
        float4 _312 = _301[2];
        float4 _317 = _301[3];
        _321 = ((all(bool4(m11[0].x == _303.x, m11[0].y == _303.y, m11[0].z == _303.z, m11[0].w == _303.w)) && all(bool4(m11[1].x == _307.x, m11[1].y == _307.y, m11[1].z == _307.z, m11[1].w == _307.w))) && all(bool4(m11[2].x == _312.x, m11[2].y == _312.y, m11[2].z == _312.z, m11[2].w == _312.w))) && all(bool4(m11[3].x == _317.x, m11[3].y == _317.y, m11[3].z == _317.z, m11[3].w == _317.w));
    }
    else
    {
        _321 = false;
    }
    ok = _321;
    return ok;
}

bool test_comma_b()
{
    float2x2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    float2x2 y = float2x2(float2(2.0f, 4.0f), float2(6.0f, 8.0f)) * 0.5f;
    return all(bool2(x[0].x == y[0].x, x[0].y == y[0].y)) && all(bool2(x[1].x == y[1].x, x[1].y == y[1].y));
}

float4 main(float2 _346)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_1_m1 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _369 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _359 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
        float2 _361 = _359[0];
        float2 _365 = _359[1];
        _369 = all(bool2(_RESERVED_IDENTIFIER_FIXUP_1_m1[0].x == _361.x, _RESERVED_IDENTIFIER_FIXUP_1_m1[0].y == _361.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_1_m1[1].x == _365.x, _RESERVED_IDENTIFIER_FIXUP_1_m1[1].y == _365.y));
    }
    else
    {
        _369 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _369;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m3 = _RESERVED_IDENTIFIER_FIXUP_1_m1;
    bool _388 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _378 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
        float2 _380 = _378[0];
        float2 _384 = _378[1];
        _388 = all(bool2(_RESERVED_IDENTIFIER_FIXUP_2_m3[0].x == _380.x, _RESERVED_IDENTIFIER_FIXUP_2_m3[0].y == _380.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_2_m3[1].x == _384.x, _RESERVED_IDENTIFIER_FIXUP_2_m3[1].y == _384.y));
    }
    else
    {
        _388 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _388;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_3_m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
    bool _409 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _399 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
        float2 _401 = _399[0];
        float2 _405 = _399[1];
        _409 = all(bool2(_RESERVED_IDENTIFIER_FIXUP_3_m4[0].x == _401.x, _RESERVED_IDENTIFIER_FIXUP_3_m4[0].y == _401.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_3_m4[1].x == _405.x, _RESERVED_IDENTIFIER_FIXUP_3_m4[1].y == _405.y));
    }
    else
    {
        _409 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _409;
    _RESERVED_IDENTIFIER_FIXUP_2_m3 = mul(_RESERVED_IDENTIFIER_FIXUP_3_m4, _RESERVED_IDENTIFIER_FIXUP_2_m3);
    bool _429 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _419 = float2x2(float2(6.0f, 12.0f), float2(18.0f, 24.0f));
        float2 _421 = _419[0];
        float2 _425 = _419[1];
        _429 = all(bool2(_RESERVED_IDENTIFIER_FIXUP_2_m3[0].x == _421.x, _RESERVED_IDENTIFIER_FIXUP_2_m3[0].y == _421.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_2_m3[1].x == _425.x, _RESERVED_IDENTIFIER_FIXUP_2_m3[1].y == _425.y));
    }
    else
    {
        _429 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _429;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_4_m5 = float2x2(float2(_RESERVED_IDENTIFIER_FIXUP_1_m1[1].y, 0.0f), float2(0.0f, _RESERVED_IDENTIFIER_FIXUP_1_m1[1].y));
    bool _453 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _443 = float2x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f));
        float2 _445 = _443[0];
        float2 _449 = _443[1];
        _453 = all(bool2(_RESERVED_IDENTIFIER_FIXUP_4_m5[0].x == _445.x, _RESERVED_IDENTIFIER_FIXUP_4_m5[0].y == _445.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_4_m5[1].x == _449.x, _RESERVED_IDENTIFIER_FIXUP_4_m5[1].y == _449.y));
    }
    else
    {
        _453 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _453;
    _RESERVED_IDENTIFIER_FIXUP_1_m1 = float2x2(_RESERVED_IDENTIFIER_FIXUP_1_m1[0] + _RESERVED_IDENTIFIER_FIXUP_4_m5[0], _RESERVED_IDENTIFIER_FIXUP_1_m1[1] + _RESERVED_IDENTIFIER_FIXUP_4_m5[1]);
    bool _479 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _469 = float2x2(float2(5.0f, 2.0f), float2(3.0f, 8.0f));
        float2 _471 = _469[0];
        float2 _475 = _469[1];
        _479 = all(bool2(_RESERVED_IDENTIFIER_FIXUP_1_m1[0].x == _471.x, _RESERVED_IDENTIFIER_FIXUP_1_m1[0].y == _471.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_1_m1[1].x == _475.x, _RESERVED_IDENTIFIER_FIXUP_1_m1[1].y == _475.y));
    }
    else
    {
        _479 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _479;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_5_m7 = float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f));
    bool _500 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _490 = float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f));
        float2 _492 = _490[0];
        float2 _496 = _490[1];
        _500 = all(bool2(_RESERVED_IDENTIFIER_FIXUP_5_m7[0].x == _492.x, _RESERVED_IDENTIFIER_FIXUP_5_m7[0].y == _492.y)) && all(bool2(_RESERVED_IDENTIFIER_FIXUP_5_m7[1].x == _496.x, _RESERVED_IDENTIFIER_FIXUP_5_m7[1].y == _496.y));
    }
    else
    {
        _500 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _500;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_6_m9 = float3x3(float3(9.0f, 0.0f, 0.0f), float3(0.0f, 9.0f, 0.0f), float3(0.0f, 0.0f, 9.0f));
    bool _528 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _513 = float3x3(float3(9.0f, 0.0f, 0.0f), float3(0.0f, 9.0f, 0.0f), float3(0.0f, 0.0f, 9.0f));
        float3 _515 = _513[0];
        float3 _519 = _513[1];
        float3 _524 = _513[2];
        _528 = (all(bool3(_RESERVED_IDENTIFIER_FIXUP_6_m9[0].x == _515.x, _RESERVED_IDENTIFIER_FIXUP_6_m9[0].y == _515.y, _RESERVED_IDENTIFIER_FIXUP_6_m9[0].z == _515.z)) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_6_m9[1].x == _519.x, _RESERVED_IDENTIFIER_FIXUP_6_m9[1].y == _519.y, _RESERVED_IDENTIFIER_FIXUP_6_m9[1].z == _519.z))) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_6_m9[2].x == _524.x, _RESERVED_IDENTIFIER_FIXUP_6_m9[2].y == _524.y, _RESERVED_IDENTIFIER_FIXUP_6_m9[2].z == _524.z));
    }
    else
    {
        _528 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _528;
    float4x4 _RESERVED_IDENTIFIER_FIXUP_7_m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
    bool _563 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float4x4 _543 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
        float4 _545 = _543[0];
        float4 _549 = _543[1];
        float4 _554 = _543[2];
        float4 _559 = _543[3];
        _563 = ((all(bool4(_RESERVED_IDENTIFIER_FIXUP_7_m10[0].x == _545.x, _RESERVED_IDENTIFIER_FIXUP_7_m10[0].y == _545.y, _RESERVED_IDENTIFIER_FIXUP_7_m10[0].z == _545.z, _RESERVED_IDENTIFIER_FIXUP_7_m10[0].w == _545.w)) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_7_m10[1].x == _549.x, _RESERVED_IDENTIFIER_FIXUP_7_m10[1].y == _549.y, _RESERVED_IDENTIFIER_FIXUP_7_m10[1].z == _549.z, _RESERVED_IDENTIFIER_FIXUP_7_m10[1].w == _549.w))) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_7_m10[2].x == _554.x, _RESERVED_IDENTIFIER_FIXUP_7_m10[2].y == _554.y, _RESERVED_IDENTIFIER_FIXUP_7_m10[2].z == _554.z, _RESERVED_IDENTIFIER_FIXUP_7_m10[2].w == _554.w))) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_7_m10[3].x == _559.x, _RESERVED_IDENTIFIER_FIXUP_7_m10[3].y == _559.y, _RESERVED_IDENTIFIER_FIXUP_7_m10[3].z == _559.z, _RESERVED_IDENTIFIER_FIXUP_7_m10[3].w == _559.w));
    }
    else
    {
        _563 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _563;
    float4x4 _RESERVED_IDENTIFIER_FIXUP_8_m11 = float4x4(20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx);
    _RESERVED_IDENTIFIER_FIXUP_8_m11 = float4x4(_RESERVED_IDENTIFIER_FIXUP_8_m11[0] - _RESERVED_IDENTIFIER_FIXUP_7_m10[0], _RESERVED_IDENTIFIER_FIXUP_8_m11[1] - _RESERVED_IDENTIFIER_FIXUP_7_m10[1], _RESERVED_IDENTIFIER_FIXUP_8_m11[2] - _RESERVED_IDENTIFIER_FIXUP_7_m10[2], _RESERVED_IDENTIFIER_FIXUP_8_m11[3] - _RESERVED_IDENTIFIER_FIXUP_7_m10[3]);
    bool _613 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float4x4 _593 = float4x4(float4(9.0f, 20.0f, 20.0f, 20.0f), float4(20.0f, 9.0f, 20.0f, 20.0f), float4(20.0f, 20.0f, 9.0f, 20.0f), float4(20.0f, 20.0f, 20.0f, 9.0f));
        float4 _595 = _593[0];
        float4 _599 = _593[1];
        float4 _604 = _593[2];
        float4 _609 = _593[3];
        _613 = ((all(bool4(_RESERVED_IDENTIFIER_FIXUP_8_m11[0].x == _595.x, _RESERVED_IDENTIFIER_FIXUP_8_m11[0].y == _595.y, _RESERVED_IDENTIFIER_FIXUP_8_m11[0].z == _595.z, _RESERVED_IDENTIFIER_FIXUP_8_m11[0].w == _595.w)) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_8_m11[1].x == _599.x, _RESERVED_IDENTIFIER_FIXUP_8_m11[1].y == _599.y, _RESERVED_IDENTIFIER_FIXUP_8_m11[1].z == _599.z, _RESERVED_IDENTIFIER_FIXUP_8_m11[1].w == _599.w))) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_8_m11[2].x == _604.x, _RESERVED_IDENTIFIER_FIXUP_8_m11[2].y == _604.y, _RESERVED_IDENTIFIER_FIXUP_8_m11[2].z == _604.z, _RESERVED_IDENTIFIER_FIXUP_8_m11[2].w == _604.w))) && all(bool4(_RESERVED_IDENTIFIER_FIXUP_8_m11[3].x == _609.x, _RESERVED_IDENTIFIER_FIXUP_8_m11[3].y == _609.y, _RESERVED_IDENTIFIER_FIXUP_8_m11[3].z == _609.z, _RESERVED_IDENTIFIER_FIXUP_8_m11[3].w == _609.w));
    }
    else
    {
        _613 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _613;
    bool _618 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _618 = test_half_b();
    }
    else
    {
        _618 = false;
    }
    bool _622 = false;
    if (_618)
    {
        _622 = test_comma_b();
    }
    else
    {
        _622 = false;
    }
    float4 _623 = 0.0f.xxxx;
    if (_622)
    {
        _623 = _12_colorGreen;
    }
    else
    {
        _623 = _12_colorRed;
    }
    return _623;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
