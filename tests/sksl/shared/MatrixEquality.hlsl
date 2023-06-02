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
    bool _160 = false;
    if (_148)
    {
        float2 _152 = _10_colorGreen.y.xx;
        float2 _153 = float2(_10_colorGreen.y, _10_colorGreen.x);
        _160 = !(all(bool2(_153.x == float2(1.0f, 0.0f).x, _153.y == float2(1.0f, 0.0f).y)) && all(bool2(_152.x == float2(0.0f, 1.0f).x, _152.y == float2(0.0f, 1.0f).y)));
    }
    else
    {
        _160 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _160;
    bool _171 = false;
    if (_160)
    {
        float2 _163 = float2(_10_colorGreen.y, 0.0f);
        float2 _164 = float2(0.0f, _10_colorGreen.y);
        _171 = all(bool2(_163.x == float2(1.0f, 0.0f).x, _163.y == float2(1.0f, 0.0f).y)) && all(bool2(_164.x == float2(0.0f, 1.0f).x, _164.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _171 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _171;
    bool _184 = false;
    if (_171)
    {
        float2 _175 = float2(_10_colorGreen.y, 0.0f);
        float2 _176 = float2(0.0f, _10_colorGreen.y);
        _184 = !(all(bool2(_175.x == 0.0f.xx.x, _175.y == 0.0f.xx.y)) && all(bool2(_176.x == 0.0f.xx.x, _176.y == 0.0f.xx.y)));
    }
    else
    {
        _184 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _184;
    bool _200 = false;
    if (_184)
    {
        float _187 = -_10_colorGreen.y;
        float2 _188 = float2(_187, 0.0f);
        float2 _189 = float2(0.0f, _187);
        _200 = all(bool2(_188.x == float2(-1.0f, 0.0f).x, _188.y == float2(-1.0f, 0.0f).y)) && all(bool2(_189.x == float2(0.0f, -1.0f).x, _189.y == float2(0.0f, -1.0f).y));
    }
    else
    {
        _200 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _200;
    bool _215 = false;
    if (_200)
    {
        float2 _203 = float2(_10_colorGreen.x, 0.0f);
        float2 _204 = float2(0.0f, _10_colorGreen.x);
        _215 = all(bool2(_203.x == float2(-0.0f, 0.0f).x, _203.y == float2(-0.0f, 0.0f).y)) && all(bool2(_204.x == float2(0.0f, -0.0f).x, _204.y == float2(0.0f, -0.0f).y));
    }
    else
    {
        _215 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _215;
    bool _230 = false;
    if (_215)
    {
        float _218 = -_10_colorGreen.y;
        float2 _219 = float2(_218, 0.0f);
        float2 _220 = float2(0.0f, _218);
        float2 _222 = -_219;
        float2 _223 = -_220;
        _230 = all(bool2(_222.x == float2(1.0f, 0.0f).x, _222.y == float2(1.0f, 0.0f).y)) && all(bool2(_223.x == float2(0.0f, 1.0f).x, _223.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _230 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _230;
    bool _244 = false;
    if (_230)
    {
        float2 _233 = float2(_10_colorGreen.x, 0.0f);
        float2 _234 = float2(0.0f, _10_colorGreen.x);
        float2 _236 = -_233;
        float2 _237 = -_234;
        _244 = all(bool2(_236.x == float2(-0.0f, 0.0f).x, _236.y == float2(-0.0f, 0.0f).y)) && all(bool2(_237.x == float2(0.0f, -0.0f).x, _237.y == float2(0.0f, -0.0f).y));
    }
    else
    {
        _244 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _244;
    bool _255 = false;
    if (_244)
    {
        float2 _247 = float2(_10_colorGreen.y, 0.0f);
        float2 _248 = float2(0.0f, _10_colorGreen.y);
        _255 = all(bool2(_247.x == float2(1.0f, 0.0f).x, _247.y == float2(1.0f, 0.0f).y)) && all(bool2(_248.x == float2(0.0f, 1.0f).x, _248.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _255 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _255;
    bool _267 = false;
    if (_255)
    {
        float2 _259 = float2(_132, 0.0f);
        float2 _260 = float2(0.0f, _132);
        _267 = !(all(bool2(_259.x == float2(1.0f, 0.0f).x, _259.y == float2(1.0f, 0.0f).y)) && all(bool2(_260.x == float2(0.0f, 1.0f).x, _260.y == float2(0.0f, 1.0f).y)));
    }
    else
    {
        _267 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _267;
    bool _279 = false;
    if (_267)
    {
        float2 _271 = float2(_10_colorGreen.y, 0.0f);
        float2 _272 = float2(0.0f, _10_colorGreen.y);
        _279 = !(any(bool2(_271.x != float2(1.0f, 0.0f).x, _271.y != float2(1.0f, 0.0f).y)) || any(bool2(_272.x != float2(0.0f, 1.0f).x, _272.y != float2(0.0f, 1.0f).y)));
    }
    else
    {
        _279 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _279;
    bool _290 = false;
    if (_279)
    {
        float2 _282 = float2(_10_colorGreen.y, 0.0f);
        float2 _283 = float2(0.0f, _10_colorGreen.y);
        _290 = any(bool2(_282.x != 0.0f.xx.x, _282.y != 0.0f.xx.y)) || any(bool2(_283.x != 0.0f.xx.x, _283.y != 0.0f.xx.y));
    }
    else
    {
        _290 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _290;
    bool _309 = false;
    if (_290)
    {
        float3 _293 = float3(_10_colorGreen.y, _10_colorGreen.xx);
        float3 _294 = float3(_10_colorGreen.x, _10_colorGreen.y, _10_colorGreen.x);
        float3 _295 = float3(_10_colorGreen.xx, _10_colorGreen.y);
        _309 = (all(bool3(_293.x == float3(1.0f, 0.0f, 0.0f).x, _293.y == float3(1.0f, 0.0f, 0.0f).y, _293.z == float3(1.0f, 0.0f, 0.0f).z)) && all(bool3(_294.x == float3(0.0f, 1.0f, 0.0f).x, _294.y == float3(0.0f, 1.0f, 0.0f).y, _294.z == float3(0.0f, 1.0f, 0.0f).z))) && all(bool3(_295.x == float3(0.0f, 0.0f, 1.0f).x, _295.y == float3(0.0f, 0.0f, 1.0f).y, _295.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _309 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _309;
    bool _330 = false;
    if (_309)
    {
        float3 _312 = float3(_134, _10_colorGreen.xx);
        float3 _313 = float3(_10_colorGreen.x, _134, _10_colorGreen.x);
        float3 _314 = float3(_10_colorGreen.xx, _10_colorGreen.y);
        _330 = (all(bool3(_312.x == float3(9.0f, 0.0f, 0.0f).x, _312.y == float3(9.0f, 0.0f, 0.0f).y, _312.z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(_313.x == float3(0.0f, 9.0f, 0.0f).x, _313.y == float3(0.0f, 9.0f, 0.0f).y, _313.z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(_314.x == float3(0.0f, 0.0f, 1.0f).x, _314.y == float3(0.0f, 0.0f, 1.0f).y, _314.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _330 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _330;
    bool _345 = false;
    if (_330)
    {
        float3 _333 = float3(_10_colorGreen.y, 0.0f, 0.0f);
        float3 _334 = float3(0.0f, _10_colorGreen.y, 0.0f);
        float3 _335 = float3(0.0f, 0.0f, _10_colorGreen.y);
        _345 = (all(bool3(_333.x == float3(1.0f, 0.0f, 0.0f).x, _333.y == float3(1.0f, 0.0f, 0.0f).y, _333.z == float3(1.0f, 0.0f, 0.0f).z)) && all(bool3(_334.x == float3(0.0f, 1.0f, 0.0f).x, _334.y == float3(0.0f, 1.0f, 0.0f).y, _334.z == float3(0.0f, 1.0f, 0.0f).z))) && all(bool3(_335.x == float3(0.0f, 0.0f, 1.0f).x, _335.y == float3(0.0f, 0.0f, 1.0f).y, _335.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _345 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _345;
    bool _360 = false;
    if (_345)
    {
        float3 _348 = float3(_134, 0.0f, 0.0f);
        float3 _349 = float3(0.0f, _134, 0.0f);
        float3 _350 = float3(0.0f, 0.0f, _10_colorGreen.y);
        _360 = (all(bool3(_348.x == float3(9.0f, 0.0f, 0.0f).x, _348.y == float3(9.0f, 0.0f, 0.0f).y, _348.z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(_349.x == float3(0.0f, 9.0f, 0.0f).x, _349.y == float3(0.0f, 9.0f, 0.0f).y, _349.z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(_350.x == float3(0.0f, 0.0f, 1.0f).x, _350.y == float3(0.0f, 0.0f, 1.0f).y, _350.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _360 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _360;
    bool _375 = false;
    if (_360)
    {
        float3 _363 = float3(_10_colorGreen.y, 0.0f, 0.0f);
        float3 _364 = float3(0.0f, _10_colorGreen.y, 0.0f);
        float2 _367 = _363.xy;
        float2 _368 = _364.xy;
        _375 = all(bool2(_367.x == float2(1.0f, 0.0f).x, _367.y == float2(1.0f, 0.0f).y)) && all(bool2(_368.x == float2(0.0f, 1.0f).x, _368.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _375 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _375;
    bool _390 = false;
    if (_375)
    {
        float3 _378 = float3(_10_colorGreen.y, 0.0f, 0.0f);
        float3 _379 = float3(0.0f, _10_colorGreen.y, 0.0f);
        float2 _382 = _378.xy;
        float2 _383 = _379.xy;
        _390 = all(bool2(_382.x == float2(1.0f, 0.0f).x, _382.y == float2(1.0f, 0.0f).y)) && all(bool2(_383.x == float2(0.0f, 1.0f).x, _383.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _390 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _390;
    bool _401 = false;
    if (_390)
    {
        float2 _393 = float2(_10_colorGreen.y, _10_colorGreen.x);
        float2 _394 = float2(_10_colorGreen.x, _10_colorGreen.y);
        _401 = all(bool2(_393.x == float2(1.0f, 0.0f).x, _393.y == float2(1.0f, 0.0f).y)) && all(bool2(_394.x == float2(0.0f, 1.0f).x, _394.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _401 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _401;
    bool _412 = false;
    if (_401)
    {
        float2 _404 = float2(_10_colorGreen.y, _10_colorGreen.x);
        float2 _405 = float2(_10_colorGreen.x, _10_colorGreen.y);
        _412 = all(bool2(_404.x == float2(1.0f, 0.0f).x, _404.y == float2(1.0f, 0.0f).y)) && all(bool2(_405.x == float2(0.0f, 1.0f).x, _405.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _412 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _412;
    bool _423 = false;
    if (_412)
    {
        float2 _415 = float2(_10_colorGreen.y, _10_colorGreen.x);
        float2 _416 = float2(_10_colorGreen.x, _10_colorGreen.y);
        _423 = all(bool2(_415.x == float2(1.0f, 0.0f).x, _415.y == float2(1.0f, 0.0f).y)) && all(bool2(_416.x == float2(0.0f, 1.0f).x, _416.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _423 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _423;
    bool _439 = false;
    if (_423)
    {
        float4 _434 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * _10_colorGreen.y.xxxx;
        _439 = all(bool4(_434.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _434.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _434.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _434.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _439 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _439;
    bool _460 = false;
    if (_439)
    {
        float4 _450 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * _10_colorGreen.y.xxxx;
        float4 _457 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
        _460 = all(bool4(_450.x == _457.x, _450.y == _457.y, _450.z == _457.z, _450.w == _457.w));
    }
    else
    {
        _460 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _460;
    bool _475 = false;
    if (_460)
    {
        float4 _471 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * _10_colorGreen.x.xxxx;
        _475 = all(bool4(_471.x == 0.0f.xxxx.x, _471.y == 0.0f.xxxx.y, _471.z == 0.0f.xxxx.z, _471.w == 0.0f.xxxx.w));
    }
    else
    {
        _475 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _475;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_5_m = float3x3(float3(_10_colorGreen.y, _132, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, _134));
    bool _488 = false;
    if (_475)
    {
        _488 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[0].x == float3(1.0f, 2.0f, 3.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[0].y == float3(1.0f, 2.0f, 3.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[0].z == float3(1.0f, 2.0f, 3.0f).z));
    }
    else
    {
        _488 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _488;
    bool _496 = false;
    if (_488)
    {
        _496 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[1].x == float3(4.0f, 5.0f, 6.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[1].y == float3(4.0f, 5.0f, 6.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[1].z == float3(4.0f, 5.0f, 6.0f).z));
    }
    else
    {
        _496 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _496;
    bool _503 = false;
    if (_496)
    {
        _503 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[2].x == float3(7.0f, 8.0f, 9.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[2].y == float3(7.0f, 8.0f, 9.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[2].z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _503 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _503;
    bool _510 = false;
    if (_503)
    {
        _510 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].x == 1.0f;
    }
    else
    {
        _510 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _510;
    bool _517 = false;
    if (_510)
    {
        _517 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].y == 2.0f;
    }
    else
    {
        _517 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _517;
    bool _524 = false;
    if (_517)
    {
        _524 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].z == 3.0f;
    }
    else
    {
        _524 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _524;
    bool _531 = false;
    if (_524)
    {
        _531 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].x == 4.0f;
    }
    else
    {
        _531 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _531;
    bool _538 = false;
    if (_531)
    {
        _538 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].y == 5.0f;
    }
    else
    {
        _538 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _538;
    bool _545 = false;
    if (_538)
    {
        _545 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].z == 6.0f;
    }
    else
    {
        _545 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _545;
    bool _552 = false;
    if (_545)
    {
        _552 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].x == 7.0f;
    }
    else
    {
        _552 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _552;
    bool _559 = false;
    if (_552)
    {
        _559 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].y == 8.0f;
    }
    else
    {
        _559 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _559;
    bool _566 = false;
    if (_559)
    {
        _566 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].z == 9.0f;
    }
    else
    {
        _566 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _566;
    float4 _567 = 0.0f.xxxx;
    if (_566)
    {
        _567 = _10_colorGreen;
    }
    else
    {
        _567 = _10_colorRed;
    }
    return _567;
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
