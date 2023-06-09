cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _10_testMatrix3x3 : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    bool _55 = false;
    if (true)
    {
        _55 = all(bool2(_10_testMatrix2x2[0].x == float2(1.0f, 2.0f).x, _10_testMatrix2x2[0].y == float2(1.0f, 2.0f).y)) && all(bool2(_10_testMatrix2x2[1].x == float2(3.0f, 4.0f).x, _10_testMatrix2x2[1].y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _55 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _55;
    bool _83 = false;
    if (_55)
    {
        _83 = (all(bool3(_10_testMatrix3x3[0].x == float3(1.0f, 2.0f, 3.0f).x, _10_testMatrix3x3[0].y == float3(1.0f, 2.0f, 3.0f).y, _10_testMatrix3x3[0].z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_10_testMatrix3x3[1].x == float3(4.0f, 5.0f, 6.0f).x, _10_testMatrix3x3[1].y == float3(4.0f, 5.0f, 6.0f).y, _10_testMatrix3x3[1].z == float3(4.0f, 5.0f, 6.0f).z))) && all(bool3(_10_testMatrix3x3[2].x == float3(7.0f, 8.0f, 9.0f).x, _10_testMatrix3x3[2].y == float3(7.0f, 8.0f, 9.0f).y, _10_testMatrix3x3[2].z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _83 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _83;
    bool _99 = false;
    if (_83)
    {
        _99 = any(bool2(_10_testMatrix2x2[0].x != float2(100.0f, 0.0f).x, _10_testMatrix2x2[0].y != float2(100.0f, 0.0f).y)) || any(bool2(_10_testMatrix2x2[1].x != float2(0.0f, 100.0f).x, _10_testMatrix2x2[1].y != float2(0.0f, 100.0f).y));
    }
    else
    {
        _99 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _99;
    bool _119 = false;
    if (_99)
    {
        _119 = (any(bool3(_10_testMatrix3x3[0].x != float3(9.0f, 8.0f, 7.0f).x, _10_testMatrix3x3[0].y != float3(9.0f, 8.0f, 7.0f).y, _10_testMatrix3x3[0].z != float3(9.0f, 8.0f, 7.0f).z)) || any(bool3(_10_testMatrix3x3[1].x != float3(6.0f, 5.0f, 4.0f).x, _10_testMatrix3x3[1].y != float3(6.0f, 5.0f, 4.0f).y, _10_testMatrix3x3[1].z != float3(6.0f, 5.0f, 4.0f).z))) || any(bool3(_10_testMatrix3x3[2].x != float3(3.0f, 2.0f, 1.0f).x, _10_testMatrix3x3[2].y != float3(3.0f, 2.0f, 1.0f).y, _10_testMatrix3x3[2].z != float3(3.0f, 2.0f, 1.0f).z));
    }
    else
    {
        _119 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _119;
    float _RESERVED_IDENTIFIER_FIXUP_1_zero = _10_colorGreen.x;
    float _RESERVED_IDENTIFIER_FIXUP_2_one = _10_colorGreen.y;
    float _132 = 2.0f * _10_colorGreen.y;
    float _RESERVED_IDENTIFIER_FIXUP_3_two = _132;
    float _134 = 9.0f * _10_colorGreen.y;
    float _RESERVED_IDENTIFIER_FIXUP_4_nine = _134;
    bool _148 = false;
    if (_119)
    {
        float2 _137 = float2(_10_colorGreen.y, _10_colorGreen.x);
        float2 _138 = float2(_10_colorGreen.x, _10_colorGreen.y);
        _148 = all(bool2(_137.x == float2(1.0f, 0.0f).x, _137.y == float2(1.0f, 0.0f).y)) && all(bool2(_138.x == float2(0.0f, 1.0f).x, _138.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _148 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _148;
    bool _159 = false;
    if (_148)
    {
        float2 _151 = _10_colorGreen.y.xx;
        float2 _152 = float2(_10_colorGreen.y, _10_colorGreen.x);
        _159 = any(bool2(_152.x != float2(1.0f, 0.0f).x, _152.y != float2(1.0f, 0.0f).y)) || any(bool2(_151.x != float2(0.0f, 1.0f).x, _151.y != float2(0.0f, 1.0f).y));
    }
    else
    {
        _159 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _159;
    bool _170 = false;
    if (_159)
    {
        float2 _162 = float2(_10_colorGreen.y, 0.0f);
        float2 _163 = float2(0.0f, _10_colorGreen.y);
        _170 = all(bool2(_162.x == float2(1.0f, 0.0f).x, _162.y == float2(1.0f, 0.0f).y)) && all(bool2(_163.x == float2(0.0f, 1.0f).x, _163.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _170 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _170;
    bool _182 = false;
    if (_170)
    {
        float2 _173 = float2(_10_colorGreen.y, 0.0f);
        float2 _174 = float2(0.0f, _10_colorGreen.y);
        _182 = any(bool2(_173.x != 0.0f.xx.x, _173.y != 0.0f.xx.y)) || any(bool2(_174.x != 0.0f.xx.x, _174.y != 0.0f.xx.y));
    }
    else
    {
        _182 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _182;
    bool _198 = false;
    if (_182)
    {
        float _185 = -_10_colorGreen.y;
        float2 _186 = float2(_185, 0.0f);
        float2 _187 = float2(0.0f, _185);
        _198 = all(bool2(_186.x == float2(-1.0f, 0.0f).x, _186.y == float2(-1.0f, 0.0f).y)) && all(bool2(_187.x == float2(0.0f, -1.0f).x, _187.y == float2(0.0f, -1.0f).y));
    }
    else
    {
        _198 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _198;
    bool _213 = false;
    if (_198)
    {
        float2 _201 = float2(_10_colorGreen.x, 0.0f);
        float2 _202 = float2(0.0f, _10_colorGreen.x);
        _213 = all(bool2(_201.x == float2(-0.0f, 0.0f).x, _201.y == float2(-0.0f, 0.0f).y)) && all(bool2(_202.x == float2(0.0f, -0.0f).x, _202.y == float2(0.0f, -0.0f).y));
    }
    else
    {
        _213 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _213;
    bool _228 = false;
    if (_213)
    {
        float _216 = -_10_colorGreen.y;
        float2 _217 = float2(_216, 0.0f);
        float2 _218 = float2(0.0f, _216);
        float2 _220 = -_217;
        float2 _221 = -_218;
        _228 = all(bool2(_220.x == float2(1.0f, 0.0f).x, _220.y == float2(1.0f, 0.0f).y)) && all(bool2(_221.x == float2(0.0f, 1.0f).x, _221.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _228 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _228;
    bool _242 = false;
    if (_228)
    {
        float2 _231 = float2(_10_colorGreen.x, 0.0f);
        float2 _232 = float2(0.0f, _10_colorGreen.x);
        float2 _234 = -_231;
        float2 _235 = -_232;
        _242 = all(bool2(_234.x == float2(-0.0f, 0.0f).x, _234.y == float2(-0.0f, 0.0f).y)) && all(bool2(_235.x == float2(0.0f, -0.0f).x, _235.y == float2(0.0f, -0.0f).y));
    }
    else
    {
        _242 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _242;
    bool _253 = false;
    if (_242)
    {
        float2 _245 = float2(_10_colorGreen.y, 0.0f);
        float2 _246 = float2(0.0f, _10_colorGreen.y);
        _253 = all(bool2(_245.x == float2(1.0f, 0.0f).x, _245.y == float2(1.0f, 0.0f).y)) && all(bool2(_246.x == float2(0.0f, 1.0f).x, _246.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _253 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _253;
    bool _264 = false;
    if (_253)
    {
        float2 _256 = float2(_132, 0.0f);
        float2 _257 = float2(0.0f, _132);
        _264 = any(bool2(_256.x != float2(1.0f, 0.0f).x, _256.y != float2(1.0f, 0.0f).y)) || any(bool2(_257.x != float2(0.0f, 1.0f).x, _257.y != float2(0.0f, 1.0f).y));
    }
    else
    {
        _264 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _264;
    bool _275 = false;
    if (_264)
    {
        float2 _267 = float2(_10_colorGreen.y, 0.0f);
        float2 _268 = float2(0.0f, _10_colorGreen.y);
        _275 = all(bool2(_267.x == float2(1.0f, 0.0f).x, _267.y == float2(1.0f, 0.0f).y)) && all(bool2(_268.x == float2(0.0f, 1.0f).x, _268.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _275 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _275;
    bool _286 = false;
    if (_275)
    {
        float2 _278 = float2(_10_colorGreen.y, 0.0f);
        float2 _279 = float2(0.0f, _10_colorGreen.y);
        _286 = any(bool2(_278.x != 0.0f.xx.x, _278.y != 0.0f.xx.y)) || any(bool2(_279.x != 0.0f.xx.x, _279.y != 0.0f.xx.y));
    }
    else
    {
        _286 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _286;
    bool _305 = false;
    if (_286)
    {
        float3 _289 = float3(_10_colorGreen.y, _10_colorGreen.xx);
        float3 _290 = float3(_10_colorGreen.x, _10_colorGreen.y, _10_colorGreen.x);
        float3 _291 = float3(_10_colorGreen.xx, _10_colorGreen.y);
        _305 = (all(bool3(_289.x == float3(1.0f, 0.0f, 0.0f).x, _289.y == float3(1.0f, 0.0f, 0.0f).y, _289.z == float3(1.0f, 0.0f, 0.0f).z)) && all(bool3(_290.x == float3(0.0f, 1.0f, 0.0f).x, _290.y == float3(0.0f, 1.0f, 0.0f).y, _290.z == float3(0.0f, 1.0f, 0.0f).z))) && all(bool3(_291.x == float3(0.0f, 0.0f, 1.0f).x, _291.y == float3(0.0f, 0.0f, 1.0f).y, _291.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _305 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _305;
    bool _326 = false;
    if (_305)
    {
        float3 _308 = float3(_134, _10_colorGreen.xx);
        float3 _309 = float3(_10_colorGreen.x, _134, _10_colorGreen.x);
        float3 _310 = float3(_10_colorGreen.xx, _10_colorGreen.y);
        _326 = (all(bool3(_308.x == float3(9.0f, 0.0f, 0.0f).x, _308.y == float3(9.0f, 0.0f, 0.0f).y, _308.z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(_309.x == float3(0.0f, 9.0f, 0.0f).x, _309.y == float3(0.0f, 9.0f, 0.0f).y, _309.z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(_310.x == float3(0.0f, 0.0f, 1.0f).x, _310.y == float3(0.0f, 0.0f, 1.0f).y, _310.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _326 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _326;
    bool _341 = false;
    if (_326)
    {
        float3 _329 = float3(_10_colorGreen.y, 0.0f, 0.0f);
        float3 _330 = float3(0.0f, _10_colorGreen.y, 0.0f);
        float3 _331 = float3(0.0f, 0.0f, _10_colorGreen.y);
        _341 = (all(bool3(_329.x == float3(1.0f, 0.0f, 0.0f).x, _329.y == float3(1.0f, 0.0f, 0.0f).y, _329.z == float3(1.0f, 0.0f, 0.0f).z)) && all(bool3(_330.x == float3(0.0f, 1.0f, 0.0f).x, _330.y == float3(0.0f, 1.0f, 0.0f).y, _330.z == float3(0.0f, 1.0f, 0.0f).z))) && all(bool3(_331.x == float3(0.0f, 0.0f, 1.0f).x, _331.y == float3(0.0f, 0.0f, 1.0f).y, _331.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _341 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _341;
    bool _356 = false;
    if (_341)
    {
        float3 _344 = float3(_134, 0.0f, 0.0f);
        float3 _345 = float3(0.0f, _134, 0.0f);
        float3 _346 = float3(0.0f, 0.0f, _10_colorGreen.y);
        _356 = (all(bool3(_344.x == float3(9.0f, 0.0f, 0.0f).x, _344.y == float3(9.0f, 0.0f, 0.0f).y, _344.z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(_345.x == float3(0.0f, 9.0f, 0.0f).x, _345.y == float3(0.0f, 9.0f, 0.0f).y, _345.z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(_346.x == float3(0.0f, 0.0f, 1.0f).x, _346.y == float3(0.0f, 0.0f, 1.0f).y, _346.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _356 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _356;
    bool _371 = false;
    if (_356)
    {
        float3 _359 = float3(_10_colorGreen.y, 0.0f, 0.0f);
        float3 _360 = float3(0.0f, _10_colorGreen.y, 0.0f);
        float2 _363 = _359.xy;
        float2 _364 = _360.xy;
        _371 = all(bool2(_363.x == float2(1.0f, 0.0f).x, _363.y == float2(1.0f, 0.0f).y)) && all(bool2(_364.x == float2(0.0f, 1.0f).x, _364.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _371 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _371;
    bool _386 = false;
    if (_371)
    {
        float3 _374 = float3(_10_colorGreen.y, 0.0f, 0.0f);
        float3 _375 = float3(0.0f, _10_colorGreen.y, 0.0f);
        float2 _378 = _374.xy;
        float2 _379 = _375.xy;
        _386 = all(bool2(_378.x == float2(1.0f, 0.0f).x, _378.y == float2(1.0f, 0.0f).y)) && all(bool2(_379.x == float2(0.0f, 1.0f).x, _379.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _386 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _386;
    bool _397 = false;
    if (_386)
    {
        float2 _389 = float2(_10_colorGreen.y, _10_colorGreen.x);
        float2 _390 = float2(_10_colorGreen.x, _10_colorGreen.y);
        _397 = all(bool2(_389.x == float2(1.0f, 0.0f).x, _389.y == float2(1.0f, 0.0f).y)) && all(bool2(_390.x == float2(0.0f, 1.0f).x, _390.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _397 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _397;
    bool _408 = false;
    if (_397)
    {
        float2 _400 = float2(_10_colorGreen.y, _10_colorGreen.x);
        float2 _401 = float2(_10_colorGreen.x, _10_colorGreen.y);
        _408 = all(bool2(_400.x == float2(1.0f, 0.0f).x, _400.y == float2(1.0f, 0.0f).y)) && all(bool2(_401.x == float2(0.0f, 1.0f).x, _401.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _408 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _408;
    bool _419 = false;
    if (_408)
    {
        float2 _411 = float2(_10_colorGreen.y, _10_colorGreen.x);
        float2 _412 = float2(_10_colorGreen.x, _10_colorGreen.y);
        _419 = all(bool2(_411.x == float2(1.0f, 0.0f).x, _411.y == float2(1.0f, 0.0f).y)) && all(bool2(_412.x == float2(0.0f, 1.0f).x, _412.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _419 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _419;
    bool _435 = false;
    if (_419)
    {
        float4 _430 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * _10_colorGreen.y.xxxx;
        _435 = all(bool4(_430.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _430.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _430.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _430.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _435 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _435;
    bool _456 = false;
    if (_435)
    {
        float4 _446 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * _10_colorGreen.y.xxxx;
        float4 _453 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
        _456 = all(bool4(_446.x == _453.x, _446.y == _453.y, _446.z == _453.z, _446.w == _453.w));
    }
    else
    {
        _456 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _456;
    bool _471 = false;
    if (_456)
    {
        float4 _467 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * _10_colorGreen.x.xxxx;
        _471 = all(bool4(_467.x == 0.0f.xxxx.x, _467.y == 0.0f.xxxx.y, _467.z == 0.0f.xxxx.z, _467.w == 0.0f.xxxx.w));
    }
    else
    {
        _471 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _471;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_5_m = float3x3(float3(_10_colorGreen.y, _132, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, _134));
    bool _484 = false;
    if (_471)
    {
        _484 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[0].x == float3(1.0f, 2.0f, 3.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[0].y == float3(1.0f, 2.0f, 3.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[0].z == float3(1.0f, 2.0f, 3.0f).z));
    }
    else
    {
        _484 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _484;
    bool _492 = false;
    if (_484)
    {
        _492 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[1].x == float3(4.0f, 5.0f, 6.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[1].y == float3(4.0f, 5.0f, 6.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[1].z == float3(4.0f, 5.0f, 6.0f).z));
    }
    else
    {
        _492 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _492;
    bool _499 = false;
    if (_492)
    {
        _499 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[2].x == float3(7.0f, 8.0f, 9.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[2].y == float3(7.0f, 8.0f, 9.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[2].z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _499 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _499;
    bool _506 = false;
    if (_499)
    {
        _506 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].x == 1.0f;
    }
    else
    {
        _506 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _506;
    bool _513 = false;
    if (_506)
    {
        _513 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].y == 2.0f;
    }
    else
    {
        _513 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _513;
    bool _520 = false;
    if (_513)
    {
        _520 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].z == 3.0f;
    }
    else
    {
        _520 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _520;
    bool _527 = false;
    if (_520)
    {
        _527 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].x == 4.0f;
    }
    else
    {
        _527 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _527;
    bool _534 = false;
    if (_527)
    {
        _534 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].y == 5.0f;
    }
    else
    {
        _534 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _534;
    bool _541 = false;
    if (_534)
    {
        _541 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].z == 6.0f;
    }
    else
    {
        _541 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _541;
    bool _548 = false;
    if (_541)
    {
        _548 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].x == 7.0f;
    }
    else
    {
        _548 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _548;
    bool _555 = false;
    if (_548)
    {
        _555 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].y == 8.0f;
    }
    else
    {
        _555 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _555;
    bool _562 = false;
    if (_555)
    {
        _562 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].z == 9.0f;
    }
    else
    {
        _562 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _562;
    float4 _563 = 0.0f.xxxx;
    if (_562)
    {
        _563 = _10_colorGreen;
    }
    else
    {
        _563 = _10_colorRed;
    }
    return _563;
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
