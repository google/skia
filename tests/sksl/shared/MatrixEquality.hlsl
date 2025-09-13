cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    row_major float2x2 _11_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _11_testMatrix3x3 : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _28)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    bool _56 = false;
    if (true)
    {
        _56 = all(bool2(_11_testMatrix2x2[0].x == float2(1.0f, 2.0f).x, _11_testMatrix2x2[0].y == float2(1.0f, 2.0f).y)) && all(bool2(_11_testMatrix2x2[1].x == float2(3.0f, 4.0f).x, _11_testMatrix2x2[1].y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _56 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _56;
    bool _84 = false;
    if (_56)
    {
        _84 = (all(bool3(_11_testMatrix3x3[0].x == float3(1.0f, 2.0f, 3.0f).x, _11_testMatrix3x3[0].y == float3(1.0f, 2.0f, 3.0f).y, _11_testMatrix3x3[0].z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_11_testMatrix3x3[1].x == float3(4.0f, 5.0f, 6.0f).x, _11_testMatrix3x3[1].y == float3(4.0f, 5.0f, 6.0f).y, _11_testMatrix3x3[1].z == float3(4.0f, 5.0f, 6.0f).z))) && all(bool3(_11_testMatrix3x3[2].x == float3(7.0f, 8.0f, 9.0f).x, _11_testMatrix3x3[2].y == float3(7.0f, 8.0f, 9.0f).y, _11_testMatrix3x3[2].z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _84 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _84;
    bool _100 = false;
    if (_84)
    {
        _100 = any(bool2(_11_testMatrix2x2[0].x != float2(100.0f, 0.0f).x, _11_testMatrix2x2[0].y != float2(100.0f, 0.0f).y)) || any(bool2(_11_testMatrix2x2[1].x != float2(0.0f, 100.0f).x, _11_testMatrix2x2[1].y != float2(0.0f, 100.0f).y));
    }
    else
    {
        _100 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _100;
    bool _120 = false;
    if (_100)
    {
        _120 = (any(bool3(_11_testMatrix3x3[0].x != float3(9.0f, 8.0f, 7.0f).x, _11_testMatrix3x3[0].y != float3(9.0f, 8.0f, 7.0f).y, _11_testMatrix3x3[0].z != float3(9.0f, 8.0f, 7.0f).z)) || any(bool3(_11_testMatrix3x3[1].x != float3(6.0f, 5.0f, 4.0f).x, _11_testMatrix3x3[1].y != float3(6.0f, 5.0f, 4.0f).y, _11_testMatrix3x3[1].z != float3(6.0f, 5.0f, 4.0f).z))) || any(bool3(_11_testMatrix3x3[2].x != float3(3.0f, 2.0f, 1.0f).x, _11_testMatrix3x3[2].y != float3(3.0f, 2.0f, 1.0f).y, _11_testMatrix3x3[2].z != float3(3.0f, 2.0f, 1.0f).z));
    }
    else
    {
        _120 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _120;
    float _RESERVED_IDENTIFIER_FIXUP_1_zero = _11_colorGreen.x;
    float _RESERVED_IDENTIFIER_FIXUP_2_one = _11_colorGreen.y;
    float _133 = 2.0f * _11_colorGreen.y;
    float _RESERVED_IDENTIFIER_FIXUP_3_two = _133;
    float _135 = 9.0f * _11_colorGreen.y;
    float _RESERVED_IDENTIFIER_FIXUP_4_nine = _135;
    bool _149 = false;
    if (_120)
    {
        float2 _138 = float2(_11_colorGreen.y, _11_colorGreen.x);
        float2 _139 = float2(_11_colorGreen.x, _11_colorGreen.y);
        _149 = all(bool2(_138.x == float2(1.0f, 0.0f).x, _138.y == float2(1.0f, 0.0f).y)) && all(bool2(_139.x == float2(0.0f, 1.0f).x, _139.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _149 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _149;
    bool _160 = false;
    if (_149)
    {
        float2 _152 = _11_colorGreen.y.xx;
        float2 _153 = float2(_11_colorGreen.y, _11_colorGreen.x);
        _160 = any(bool2(_153.x != float2(1.0f, 0.0f).x, _153.y != float2(1.0f, 0.0f).y)) || any(bool2(_152.x != float2(0.0f, 1.0f).x, _152.y != float2(0.0f, 1.0f).y));
    }
    else
    {
        _160 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _160;
    bool _171 = false;
    if (_160)
    {
        float2 _163 = float2(_11_colorGreen.y, 0.0f);
        float2 _164 = float2(0.0f, _11_colorGreen.y);
        _171 = all(bool2(_163.x == float2(1.0f, 0.0f).x, _163.y == float2(1.0f, 0.0f).y)) && all(bool2(_164.x == float2(0.0f, 1.0f).x, _164.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _171 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _171;
    bool _183 = false;
    if (_171)
    {
        float2 _174 = float2(_11_colorGreen.y, 0.0f);
        float2 _175 = float2(0.0f, _11_colorGreen.y);
        _183 = any(bool2(_174.x != 0.0f.xx.x, _174.y != 0.0f.xx.y)) || any(bool2(_175.x != 0.0f.xx.x, _175.y != 0.0f.xx.y));
    }
    else
    {
        _183 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _183;
    bool _199 = false;
    if (_183)
    {
        float _186 = -_11_colorGreen.y;
        float2 _187 = float2(_186, 0.0f);
        float2 _188 = float2(0.0f, _186);
        _199 = all(bool2(_187.x == float2(-1.0f, 0.0f).x, _187.y == float2(-1.0f, 0.0f).y)) && all(bool2(_188.x == float2(0.0f, -1.0f).x, _188.y == float2(0.0f, -1.0f).y));
    }
    else
    {
        _199 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _199;
    bool _214 = false;
    if (_199)
    {
        float2 _202 = float2(_11_colorGreen.x, 0.0f);
        float2 _203 = float2(0.0f, _11_colorGreen.x);
        _214 = all(bool2(_202.x == float2(-0.0f, 0.0f).x, _202.y == float2(-0.0f, 0.0f).y)) && all(bool2(_203.x == float2(0.0f, -0.0f).x, _203.y == float2(0.0f, -0.0f).y));
    }
    else
    {
        _214 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _214;
    bool _229 = false;
    if (_214)
    {
        float _217 = -_11_colorGreen.y;
        float2 _218 = float2(_217, 0.0f);
        float2 _219 = float2(0.0f, _217);
        float2 _221 = -_218;
        float2 _222 = -_219;
        _229 = all(bool2(_221.x == float2(1.0f, 0.0f).x, _221.y == float2(1.0f, 0.0f).y)) && all(bool2(_222.x == float2(0.0f, 1.0f).x, _222.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _229 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _229;
    bool _243 = false;
    if (_229)
    {
        float2 _232 = float2(_11_colorGreen.x, 0.0f);
        float2 _233 = float2(0.0f, _11_colorGreen.x);
        float2 _235 = -_232;
        float2 _236 = -_233;
        _243 = all(bool2(_235.x == float2(-0.0f, 0.0f).x, _235.y == float2(-0.0f, 0.0f).y)) && all(bool2(_236.x == float2(0.0f, -0.0f).x, _236.y == float2(0.0f, -0.0f).y));
    }
    else
    {
        _243 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _243;
    bool _254 = false;
    if (_243)
    {
        float2 _246 = float2(_11_colorGreen.y, 0.0f);
        float2 _247 = float2(0.0f, _11_colorGreen.y);
        _254 = all(bool2(_246.x == float2(1.0f, 0.0f).x, _246.y == float2(1.0f, 0.0f).y)) && all(bool2(_247.x == float2(0.0f, 1.0f).x, _247.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _254 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _254;
    bool _265 = false;
    if (_254)
    {
        float2 _257 = float2(_133, 0.0f);
        float2 _258 = float2(0.0f, _133);
        _265 = any(bool2(_257.x != float2(1.0f, 0.0f).x, _257.y != float2(1.0f, 0.0f).y)) || any(bool2(_258.x != float2(0.0f, 1.0f).x, _258.y != float2(0.0f, 1.0f).y));
    }
    else
    {
        _265 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _265;
    bool _276 = false;
    if (_265)
    {
        float2 _268 = float2(_11_colorGreen.y, 0.0f);
        float2 _269 = float2(0.0f, _11_colorGreen.y);
        _276 = all(bool2(_268.x == float2(1.0f, 0.0f).x, _268.y == float2(1.0f, 0.0f).y)) && all(bool2(_269.x == float2(0.0f, 1.0f).x, _269.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _276 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _276;
    bool _287 = false;
    if (_276)
    {
        float2 _279 = float2(_11_colorGreen.y, 0.0f);
        float2 _280 = float2(0.0f, _11_colorGreen.y);
        _287 = any(bool2(_279.x != 0.0f.xx.x, _279.y != 0.0f.xx.y)) || any(bool2(_280.x != 0.0f.xx.x, _280.y != 0.0f.xx.y));
    }
    else
    {
        _287 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _287;
    bool _306 = false;
    if (_287)
    {
        float3 _290 = float3(_11_colorGreen.y, _11_colorGreen.xx);
        float3 _291 = float3(_11_colorGreen.x, _11_colorGreen.y, _11_colorGreen.x);
        float3 _292 = float3(_11_colorGreen.xx, _11_colorGreen.y);
        _306 = (all(bool3(_290.x == float3(1.0f, 0.0f, 0.0f).x, _290.y == float3(1.0f, 0.0f, 0.0f).y, _290.z == float3(1.0f, 0.0f, 0.0f).z)) && all(bool3(_291.x == float3(0.0f, 1.0f, 0.0f).x, _291.y == float3(0.0f, 1.0f, 0.0f).y, _291.z == float3(0.0f, 1.0f, 0.0f).z))) && all(bool3(_292.x == float3(0.0f, 0.0f, 1.0f).x, _292.y == float3(0.0f, 0.0f, 1.0f).y, _292.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _306 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _306;
    bool _327 = false;
    if (_306)
    {
        float3 _309 = float3(_135, _11_colorGreen.xx);
        float3 _310 = float3(_11_colorGreen.x, _135, _11_colorGreen.x);
        float3 _311 = float3(_11_colorGreen.xx, _11_colorGreen.y);
        _327 = (all(bool3(_309.x == float3(9.0f, 0.0f, 0.0f).x, _309.y == float3(9.0f, 0.0f, 0.0f).y, _309.z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(_310.x == float3(0.0f, 9.0f, 0.0f).x, _310.y == float3(0.0f, 9.0f, 0.0f).y, _310.z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(_311.x == float3(0.0f, 0.0f, 1.0f).x, _311.y == float3(0.0f, 0.0f, 1.0f).y, _311.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _327 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _327;
    bool _342 = false;
    if (_327)
    {
        float3 _330 = float3(_11_colorGreen.y, 0.0f, 0.0f);
        float3 _331 = float3(0.0f, _11_colorGreen.y, 0.0f);
        float3 _332 = float3(0.0f, 0.0f, _11_colorGreen.y);
        _342 = (all(bool3(_330.x == float3(1.0f, 0.0f, 0.0f).x, _330.y == float3(1.0f, 0.0f, 0.0f).y, _330.z == float3(1.0f, 0.0f, 0.0f).z)) && all(bool3(_331.x == float3(0.0f, 1.0f, 0.0f).x, _331.y == float3(0.0f, 1.0f, 0.0f).y, _331.z == float3(0.0f, 1.0f, 0.0f).z))) && all(bool3(_332.x == float3(0.0f, 0.0f, 1.0f).x, _332.y == float3(0.0f, 0.0f, 1.0f).y, _332.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _342 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _342;
    bool _357 = false;
    if (_342)
    {
        float3 _345 = float3(_135, 0.0f, 0.0f);
        float3 _346 = float3(0.0f, _135, 0.0f);
        float3 _347 = float3(0.0f, 0.0f, _11_colorGreen.y);
        _357 = (all(bool3(_345.x == float3(9.0f, 0.0f, 0.0f).x, _345.y == float3(9.0f, 0.0f, 0.0f).y, _345.z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(_346.x == float3(0.0f, 9.0f, 0.0f).x, _346.y == float3(0.0f, 9.0f, 0.0f).y, _346.z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(_347.x == float3(0.0f, 0.0f, 1.0f).x, _347.y == float3(0.0f, 0.0f, 1.0f).y, _347.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _357 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _357;
    bool _372 = false;
    if (_357)
    {
        float3 _360 = float3(_11_colorGreen.y, 0.0f, 0.0f);
        float3 _361 = float3(0.0f, _11_colorGreen.y, 0.0f);
        float2 _364 = _360.xy;
        float2 _365 = _361.xy;
        _372 = all(bool2(_364.x == float2(1.0f, 0.0f).x, _364.y == float2(1.0f, 0.0f).y)) && all(bool2(_365.x == float2(0.0f, 1.0f).x, _365.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _372 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _372;
    bool _387 = false;
    if (_372)
    {
        float3 _375 = float3(_11_colorGreen.y, 0.0f, 0.0f);
        float3 _376 = float3(0.0f, _11_colorGreen.y, 0.0f);
        float2 _379 = _375.xy;
        float2 _380 = _376.xy;
        _387 = all(bool2(_379.x == float2(1.0f, 0.0f).x, _379.y == float2(1.0f, 0.0f).y)) && all(bool2(_380.x == float2(0.0f, 1.0f).x, _380.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _387 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _387;
    bool _398 = false;
    if (_387)
    {
        float2 _390 = float2(_11_colorGreen.y, _11_colorGreen.x);
        float2 _391 = float2(_11_colorGreen.x, _11_colorGreen.y);
        _398 = all(bool2(_390.x == float2(1.0f, 0.0f).x, _390.y == float2(1.0f, 0.0f).y)) && all(bool2(_391.x == float2(0.0f, 1.0f).x, _391.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _398 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _398;
    bool _409 = false;
    if (_398)
    {
        float2 _401 = float2(_11_colorGreen.y, _11_colorGreen.x);
        float2 _402 = float2(_11_colorGreen.x, _11_colorGreen.y);
        _409 = all(bool2(_401.x == float2(1.0f, 0.0f).x, _401.y == float2(1.0f, 0.0f).y)) && all(bool2(_402.x == float2(0.0f, 1.0f).x, _402.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _409 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _409;
    bool _420 = false;
    if (_409)
    {
        float2 _412 = float2(_11_colorGreen.y, _11_colorGreen.x);
        float2 _413 = float2(_11_colorGreen.x, _11_colorGreen.y);
        _420 = all(bool2(_412.x == float2(1.0f, 0.0f).x, _412.y == float2(1.0f, 0.0f).y)) && all(bool2(_413.x == float2(0.0f, 1.0f).x, _413.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _420 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _420;
    bool _436 = false;
    if (_420)
    {
        float4 _431 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y) * _11_colorGreen.y.xxxx;
        _436 = all(bool4(_431.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _431.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _431.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _431.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _436 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _436;
    bool _457 = false;
    if (_436)
    {
        float4 _447 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y) * _11_colorGreen.y.xxxx;
        float4 _454 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y);
        _457 = all(bool4(_447.x == _454.x, _447.y == _454.y, _447.z == _454.z, _447.w == _454.w));
    }
    else
    {
        _457 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _457;
    bool _472 = false;
    if (_457)
    {
        float4 _468 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y) * _11_colorGreen.x.xxxx;
        _472 = all(bool4(_468.x == 0.0f.xxxx.x, _468.y == 0.0f.xxxx.y, _468.z == 0.0f.xxxx.z, _468.w == 0.0f.xxxx.w));
    }
    else
    {
        _472 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _472;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_5_m = float3x3(float3(_11_colorGreen.y, _133, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, _135));
    bool _485 = false;
    if (_472)
    {
        _485 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[0].x == float3(1.0f, 2.0f, 3.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[0].y == float3(1.0f, 2.0f, 3.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[0].z == float3(1.0f, 2.0f, 3.0f).z));
    }
    else
    {
        _485 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _485;
    bool _493 = false;
    if (_485)
    {
        _493 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[1].x == float3(4.0f, 5.0f, 6.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[1].y == float3(4.0f, 5.0f, 6.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[1].z == float3(4.0f, 5.0f, 6.0f).z));
    }
    else
    {
        _493 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _493;
    bool _500 = false;
    if (_493)
    {
        _500 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[2].x == float3(7.0f, 8.0f, 9.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[2].y == float3(7.0f, 8.0f, 9.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[2].z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _500 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _500;
    bool _507 = false;
    if (_500)
    {
        _507 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].x == 1.0f;
    }
    else
    {
        _507 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _507;
    bool _514 = false;
    if (_507)
    {
        _514 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].y == 2.0f;
    }
    else
    {
        _514 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _514;
    bool _521 = false;
    if (_514)
    {
        _521 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].z == 3.0f;
    }
    else
    {
        _521 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _521;
    bool _528 = false;
    if (_521)
    {
        _528 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].x == 4.0f;
    }
    else
    {
        _528 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _528;
    bool _535 = false;
    if (_528)
    {
        _535 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].y == 5.0f;
    }
    else
    {
        _535 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _535;
    bool _542 = false;
    if (_535)
    {
        _542 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].z == 6.0f;
    }
    else
    {
        _542 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _542;
    bool _549 = false;
    if (_542)
    {
        _549 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].x == 7.0f;
    }
    else
    {
        _549 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _549;
    bool _556 = false;
    if (_549)
    {
        _556 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].y == 8.0f;
    }
    else
    {
        _556 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _556;
    bool _563 = false;
    if (_556)
    {
        _563 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].z == 9.0f;
    }
    else
    {
        _563 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _563;
    float4 _564 = 0.0f.xxxx;
    if (_563)
    {
        _564 = _11_colorGreen;
    }
    else
    {
        _564 = _11_colorRed;
    }
    return _564;
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
