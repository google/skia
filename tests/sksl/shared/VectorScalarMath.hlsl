cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorRed : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float _11_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_int_b()
{
    bool ok = true;
    int4 inputRed = int4(int(_11_colorRed.x), int(_11_colorRed.y), int(_11_colorRed.z), int(_11_colorRed.w));
    int4 inputGreen = int4(int(_11_colorGreen.x), int(_11_colorGreen.y), int(_11_colorGreen.z), int(_11_colorGreen.w));
    int4 x = inputRed + int4(2, 2, 2, 2);
    bool _74 = false;
    if (ok)
    {
        _74 = all(bool4(x.x == int4(3, 2, 2, 3).x, x.y == int4(3, 2, 2, 3).y, x.z == int4(3, 2, 2, 3).z, x.w == int4(3, 2, 2, 3).w));
    }
    else
    {
        _74 = false;
    }
    ok = _74;
    x = inputGreen.ywxz - int4(2, 2, 2, 2);
    bool _88 = false;
    if (ok)
    {
        _88 = all(bool4(x.x == int4(-1, -1, -2, -2).x, x.y == int4(-1, -1, -2, -2).y, x.z == int4(-1, -1, -2, -2).z, x.w == int4(-1, -1, -2, -2).w));
    }
    else
    {
        _88 = false;
    }
    ok = _88;
    x = inputRed + inputGreen.y.xxxx;
    bool _101 = false;
    if (ok)
    {
        _101 = all(bool4(x.x == int4(2, 1, 1, 2).x, x.y == int4(2, 1, 1, 2).y, x.z == int4(2, 1, 1, 2).z, x.w == int4(2, 1, 1, 2).w));
    }
    else
    {
        _101 = false;
    }
    ok = _101;
    int3 _107 = inputGreen.wyw * int3(9, 9, 9);
    x = int4(_107.x, _107.y, _107.z, x.w);
    bool _117 = false;
    if (ok)
    {
        _117 = all(bool4(x.x == int4(9, 9, 9, 2).x, x.y == int4(9, 9, 9, 2).y, x.z == int4(9, 9, 9, 2).z, x.w == int4(9, 9, 9, 2).w));
    }
    else
    {
        _117 = false;
    }
    ok = _117;
    int2 _122 = x.zw / int2(3, 3);
    x = int4(_122.x, _122.y, x.z, x.w);
    bool _132 = false;
    if (ok)
    {
        _132 = all(bool4(x.x == int4(3, 0, 9, 2).x, x.y == int4(3, 0, 9, 2).y, x.z == int4(3, 0, 9, 2).z, x.w == int4(3, 0, 9, 2).w));
    }
    else
    {
        _132 = false;
    }
    ok = _132;
    x = (inputRed * int4(5, 5, 5, 5)).yxwz;
    bool _145 = false;
    if (ok)
    {
        _145 = all(bool4(x.x == int4(0, 5, 5, 0).x, x.y == int4(0, 5, 5, 0).y, x.z == int4(0, 5, 5, 0).z, x.w == int4(0, 5, 5, 0).w));
    }
    else
    {
        _145 = false;
    }
    ok = _145;
    x = int4(2, 2, 2, 2) + inputRed;
    bool _155 = false;
    if (ok)
    {
        _155 = all(bool4(x.x == int4(3, 2, 2, 3).x, x.y == int4(3, 2, 2, 3).y, x.z == int4(3, 2, 2, 3).z, x.w == int4(3, 2, 2, 3).w));
    }
    else
    {
        _155 = false;
    }
    ok = _155;
    x = int4(10, 10, 10, 10) - inputGreen.ywxz;
    bool _168 = false;
    if (ok)
    {
        _168 = all(bool4(x.x == int4(9, 9, 10, 10).x, x.y == int4(9, 9, 10, 10).y, x.z == int4(9, 9, 10, 10).z, x.w == int4(9, 9, 10, 10).w));
    }
    else
    {
        _168 = false;
    }
    ok = _168;
    x = inputRed.x.xxxx + inputGreen;
    bool _181 = false;
    if (ok)
    {
        _181 = all(bool4(x.x == int4(1, 2, 1, 2).x, x.y == int4(1, 2, 1, 2).y, x.z == int4(1, 2, 1, 2).z, x.w == int4(1, 2, 1, 2).w));
    }
    else
    {
        _181 = false;
    }
    ok = _181;
    int3 _185 = int3(9, 9, 9) * inputGreen.wyw;
    x = int4(_185.x, _185.y, _185.z, x.w);
    bool _194 = false;
    if (ok)
    {
        _194 = all(bool4(x.x == int4(9, 9, 9, 2).x, x.y == int4(9, 9, 9, 2).y, x.z == int4(9, 9, 9, 2).z, x.w == int4(9, 9, 9, 2).w));
    }
    else
    {
        _194 = false;
    }
    ok = _194;
    int2 _199 = int2(36, 36) / x.zw;
    x = int4(_199.x, _199.y, x.z, x.w);
    bool _211 = false;
    if (ok)
    {
        _211 = all(bool4(x.x == int4(4, 18, 9, 2).x, x.y == int4(4, 18, 9, 2).y, x.z == int4(4, 18, 9, 2).z, x.w == int4(4, 18, 9, 2).w));
    }
    else
    {
        _211 = false;
    }
    ok = _211;
    x = (int4(36, 36, 36, 36) / x).yxwz;
    bool _223 = false;
    if (ok)
    {
        _223 = all(bool4(x.x == int4(2, 9, 18, 4).x, x.y == int4(2, 9, 18, 4).y, x.z == int4(2, 9, 18, 4).z, x.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _223 = false;
    }
    ok = _223;
    x += int4(2, 2, 2, 2);
    x *= int4(2, 2, 2, 2);
    x -= int4(4, 4, 4, 4);
    x /= int4(2, 2, 2, 2);
    bool _242 = false;
    if (ok)
    {
        _242 = all(bool4(x.x == int4(2, 9, 18, 4).x, x.y == int4(2, 9, 18, 4).y, x.z == int4(2, 9, 18, 4).z, x.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _242 = false;
    }
    ok = _242;
    x += int4(2, 2, 2, 2);
    x *= int4(2, 2, 2, 2);
    x -= int4(4, 4, 4, 4);
    x /= int4(2, 2, 2, 2);
    bool _261 = false;
    if (ok)
    {
        _261 = all(bool4(x.x == int4(2, 9, 18, 4).x, x.y == int4(2, 9, 18, 4).y, x.z == int4(2, 9, 18, 4).z, x.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _261 = false;
    }
    ok = _261;
    return ok;
}

float4 main(float2 _264)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float4 _RESERVED_IDENTIFIER_FIXUP_1_inputRed = _11_colorRed;
    float4 _RESERVED_IDENTIFIER_FIXUP_2_inputGreen = _11_colorGreen;
    float4 _RESERVED_IDENTIFIER_FIXUP_3_x = _RESERVED_IDENTIFIER_FIXUP_1_inputRed + 2.0f.xxxx;
    bool _287 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _287 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(3.0f, 2.0f, 2.0f, 3.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(3.0f, 2.0f, 2.0f, 3.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(3.0f, 2.0f, 2.0f, 3.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(3.0f, 2.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _287 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _287;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _RESERVED_IDENTIFIER_FIXUP_2_inputGreen.ywxz - 2.0f.xxxx;
    bool _301 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _301 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(-1.0f, -1.0f, -2.0f, -2.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(-1.0f, -1.0f, -2.0f, -2.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(-1.0f, -1.0f, -2.0f, -2.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(-1.0f, -1.0f, -2.0f, -2.0f).w));
    }
    else
    {
        _301 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _301;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _RESERVED_IDENTIFIER_FIXUP_1_inputRed + _RESERVED_IDENTIFIER_FIXUP_2_inputGreen.y.xxxx;
    bool _315 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _315 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(2.0f, 1.0f, 1.0f, 2.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(2.0f, 1.0f, 1.0f, 2.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(2.0f, 1.0f, 1.0f, 2.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(2.0f, 1.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _315 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _315;
    float3 _320 = _RESERVED_IDENTIFIER_FIXUP_2_inputGreen.wyw * 9.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = float4(_320.x, _320.y, _320.z, _RESERVED_IDENTIFIER_FIXUP_3_x.w);
    bool _330 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _330 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(9.0f, 9.0f, 9.0f, 2.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(9.0f, 9.0f, 9.0f, 2.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(9.0f, 9.0f, 9.0f, 2.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(9.0f, 9.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _330 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _330;
    float2 _333 = _RESERVED_IDENTIFIER_FIXUP_3_x.zw * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = float4(_333.x, _333.y, _RESERVED_IDENTIFIER_FIXUP_3_x.z, _RESERVED_IDENTIFIER_FIXUP_3_x.w);
    bool _345 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _345 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(18.0f, 4.0f, 9.0f, 2.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(18.0f, 4.0f, 9.0f, 2.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(18.0f, 4.0f, 9.0f, 2.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(18.0f, 4.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _345 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _345;
    _RESERVED_IDENTIFIER_FIXUP_3_x = (_RESERVED_IDENTIFIER_FIXUP_1_inputRed * 5.0f).yxwz;
    bool _357 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _357 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(0.0f, 5.0f, 5.0f, 0.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(0.0f, 5.0f, 5.0f, 0.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(0.0f, 5.0f, 5.0f, 0.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(0.0f, 5.0f, 5.0f, 0.0f).w));
    }
    else
    {
        _357 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _357;
    _RESERVED_IDENTIFIER_FIXUP_3_x = 2.0f.xxxx + _RESERVED_IDENTIFIER_FIXUP_1_inputRed;
    bool _367 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _367 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(3.0f, 2.0f, 2.0f, 3.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(3.0f, 2.0f, 2.0f, 3.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(3.0f, 2.0f, 2.0f, 3.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(3.0f, 2.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _367 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _367;
    _RESERVED_IDENTIFIER_FIXUP_3_x = 10.0f.xxxx - _RESERVED_IDENTIFIER_FIXUP_2_inputGreen.ywxz;
    bool _380 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _380 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(9.0f, 9.0f, 10.0f, 10.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(9.0f, 9.0f, 10.0f, 10.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(9.0f, 9.0f, 10.0f, 10.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(9.0f, 9.0f, 10.0f, 10.0f).w));
    }
    else
    {
        _380 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _380;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _RESERVED_IDENTIFIER_FIXUP_1_inputRed.x.xxxx + _RESERVED_IDENTIFIER_FIXUP_2_inputGreen;
    bool _393 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _393 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(1.0f, 2.0f, 1.0f, 2.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(1.0f, 2.0f, 1.0f, 2.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(1.0f, 2.0f, 1.0f, 2.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(1.0f, 2.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _393 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _393;
    float3 _396 = _RESERVED_IDENTIFIER_FIXUP_2_inputGreen.wyw * 9.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = float4(_396.x, _396.y, _396.z, _RESERVED_IDENTIFIER_FIXUP_3_x.w);
    bool _405 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _405 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(9.0f, 9.0f, 9.0f, 2.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(9.0f, 9.0f, 9.0f, 2.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(9.0f, 9.0f, 9.0f, 2.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(9.0f, 9.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _405 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _405;
    float2 _410 = 36.0f.xx / _RESERVED_IDENTIFIER_FIXUP_3_x.zw;
    _RESERVED_IDENTIFIER_FIXUP_3_x = float4(_410.x, _410.y, _RESERVED_IDENTIFIER_FIXUP_3_x.z, _RESERVED_IDENTIFIER_FIXUP_3_x.w);
    bool _420 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _420 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(4.0f, 18.0f, 9.0f, 2.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(4.0f, 18.0f, 9.0f, 2.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(4.0f, 18.0f, 9.0f, 2.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(4.0f, 18.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _420 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _420;
    _RESERVED_IDENTIFIER_FIXUP_3_x = (36.0f.xxxx / _RESERVED_IDENTIFIER_FIXUP_3_x).yxwz;
    bool _432 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _432 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(2.0f, 9.0f, 18.0f, 4.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(2.0f, 9.0f, 18.0f, 4.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(2.0f, 9.0f, 18.0f, 4.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(2.0f, 9.0f, 18.0f, 4.0f).w));
    }
    else
    {
        _432 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _432;
    _RESERVED_IDENTIFIER_FIXUP_3_x += 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x *= 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x -= 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x *= (1.0f / 2.0f);
    bool _450 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _450 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(2.0f, 9.0f, 18.0f, 4.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(2.0f, 9.0f, 18.0f, 4.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(2.0f, 9.0f, 18.0f, 4.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(2.0f, 9.0f, 18.0f, 4.0f).w));
    }
    else
    {
        _450 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _450;
    _RESERVED_IDENTIFIER_FIXUP_3_x += 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x *= 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x -= 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x *= 0.5f;
    bool _468 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _468 = all(bool4(_RESERVED_IDENTIFIER_FIXUP_3_x.x == float4(2.0f, 9.0f, 18.0f, 4.0f).x, _RESERVED_IDENTIFIER_FIXUP_3_x.y == float4(2.0f, 9.0f, 18.0f, 4.0f).y, _RESERVED_IDENTIFIER_FIXUP_3_x.z == float4(2.0f, 9.0f, 18.0f, 4.0f).z, _RESERVED_IDENTIFIER_FIXUP_3_x.w == float4(2.0f, 9.0f, 18.0f, 4.0f).w));
    }
    else
    {
        _468 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _468;
    bool _473 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _473 = test_int_b();
    }
    else
    {
        _473 = false;
    }
    float4 _474 = 0.0f.xxxx;
    if (_473)
    {
        _474 = _11_colorGreen;
    }
    else
    {
        _474 = _11_colorRed;
    }
    return _474;
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
