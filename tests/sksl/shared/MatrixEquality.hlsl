cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _7_testMatrix3x3 : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    bool _53 = false;
    if (true)
    {
        _53 = all(bool2(_7_testMatrix2x2[0].x == float2(1.0f, 2.0f).x, _7_testMatrix2x2[0].y == float2(1.0f, 2.0f).y)) && all(bool2(_7_testMatrix2x2[1].x == float2(3.0f, 4.0f).x, _7_testMatrix2x2[1].y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _53 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _53;
    bool _81 = false;
    if (_53)
    {
        _81 = (all(bool3(_7_testMatrix3x3[0].x == float3(1.0f, 2.0f, 3.0f).x, _7_testMatrix3x3[0].y == float3(1.0f, 2.0f, 3.0f).y, _7_testMatrix3x3[0].z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_7_testMatrix3x3[1].x == float3(4.0f, 5.0f, 6.0f).x, _7_testMatrix3x3[1].y == float3(4.0f, 5.0f, 6.0f).y, _7_testMatrix3x3[1].z == float3(4.0f, 5.0f, 6.0f).z))) && all(bool3(_7_testMatrix3x3[2].x == float3(7.0f, 8.0f, 9.0f).x, _7_testMatrix3x3[2].y == float3(7.0f, 8.0f, 9.0f).y, _7_testMatrix3x3[2].z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _81 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _81;
    bool _97 = false;
    if (_81)
    {
        _97 = any(bool2(_7_testMatrix2x2[0].x != float2(100.0f, 0.0f).x, _7_testMatrix2x2[0].y != float2(100.0f, 0.0f).y)) || any(bool2(_7_testMatrix2x2[1].x != float2(0.0f, 100.0f).x, _7_testMatrix2x2[1].y != float2(0.0f, 100.0f).y));
    }
    else
    {
        _97 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _97;
    bool _117 = false;
    if (_97)
    {
        _117 = (any(bool3(_7_testMatrix3x3[0].x != float3(9.0f, 8.0f, 7.0f).x, _7_testMatrix3x3[0].y != float3(9.0f, 8.0f, 7.0f).y, _7_testMatrix3x3[0].z != float3(9.0f, 8.0f, 7.0f).z)) || any(bool3(_7_testMatrix3x3[1].x != float3(6.0f, 5.0f, 4.0f).x, _7_testMatrix3x3[1].y != float3(6.0f, 5.0f, 4.0f).y, _7_testMatrix3x3[1].z != float3(6.0f, 5.0f, 4.0f).z))) || any(bool3(_7_testMatrix3x3[2].x != float3(3.0f, 2.0f, 1.0f).x, _7_testMatrix3x3[2].y != float3(3.0f, 2.0f, 1.0f).y, _7_testMatrix3x3[2].z != float3(3.0f, 2.0f, 1.0f).z));
    }
    else
    {
        _117 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _117;
    float _RESERVED_IDENTIFIER_FIXUP_1_zero = _7_colorGreen.x;
    float _RESERVED_IDENTIFIER_FIXUP_2_one = _7_colorGreen.y;
    float _130 = 2.0f * _7_colorGreen.y;
    float _RESERVED_IDENTIFIER_FIXUP_3_two = _130;
    float _132 = 9.0f * _7_colorGreen.y;
    float _RESERVED_IDENTIFIER_FIXUP_4_nine = _132;
    bool _146 = false;
    if (_117)
    {
        float2 _135 = float2(_7_colorGreen.y, _7_colorGreen.x);
        float2 _136 = float2(_7_colorGreen.x, _7_colorGreen.y);
        _146 = all(bool2(_135.x == float2(1.0f, 0.0f).x, _135.y == float2(1.0f, 0.0f).y)) && all(bool2(_136.x == float2(0.0f, 1.0f).x, _136.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _146 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _146;
    bool _157 = false;
    if (_146)
    {
        float2 _149 = _7_colorGreen.y.xx;
        float2 _150 = float2(_7_colorGreen.y, _7_colorGreen.x);
        _157 = any(bool2(_150.x != float2(1.0f, 0.0f).x, _150.y != float2(1.0f, 0.0f).y)) || any(bool2(_149.x != float2(0.0f, 1.0f).x, _149.y != float2(0.0f, 1.0f).y));
    }
    else
    {
        _157 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _157;
    bool _168 = false;
    if (_157)
    {
        float2 _160 = float2(_7_colorGreen.y, 0.0f);
        float2 _161 = float2(0.0f, _7_colorGreen.y);
        _168 = all(bool2(_160.x == float2(1.0f, 0.0f).x, _160.y == float2(1.0f, 0.0f).y)) && all(bool2(_161.x == float2(0.0f, 1.0f).x, _161.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _168 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _168;
    bool _180 = false;
    if (_168)
    {
        float2 _171 = float2(_7_colorGreen.y, 0.0f);
        float2 _172 = float2(0.0f, _7_colorGreen.y);
        _180 = any(bool2(_171.x != 0.0f.xx.x, _171.y != 0.0f.xx.y)) || any(bool2(_172.x != 0.0f.xx.x, _172.y != 0.0f.xx.y));
    }
    else
    {
        _180 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _180;
    bool _196 = false;
    if (_180)
    {
        float _183 = -_7_colorGreen.y;
        float2 _184 = float2(_183, 0.0f);
        float2 _185 = float2(0.0f, _183);
        _196 = all(bool2(_184.x == float2(-1.0f, 0.0f).x, _184.y == float2(-1.0f, 0.0f).y)) && all(bool2(_185.x == float2(0.0f, -1.0f).x, _185.y == float2(0.0f, -1.0f).y));
    }
    else
    {
        _196 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _196;
    bool _211 = false;
    if (_196)
    {
        float2 _199 = float2(_7_colorGreen.x, 0.0f);
        float2 _200 = float2(0.0f, _7_colorGreen.x);
        _211 = all(bool2(_199.x == float2(-0.0f, 0.0f).x, _199.y == float2(-0.0f, 0.0f).y)) && all(bool2(_200.x == float2(0.0f, -0.0f).x, _200.y == float2(0.0f, -0.0f).y));
    }
    else
    {
        _211 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _211;
    bool _226 = false;
    if (_211)
    {
        float _214 = -_7_colorGreen.y;
        float2 _215 = float2(_214, 0.0f);
        float2 _216 = float2(0.0f, _214);
        float2 _218 = -_215;
        float2 _219 = -_216;
        _226 = all(bool2(_218.x == float2(1.0f, 0.0f).x, _218.y == float2(1.0f, 0.0f).y)) && all(bool2(_219.x == float2(0.0f, 1.0f).x, _219.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _226 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _226;
    bool _240 = false;
    if (_226)
    {
        float2 _229 = float2(_7_colorGreen.x, 0.0f);
        float2 _230 = float2(0.0f, _7_colorGreen.x);
        float2 _232 = -_229;
        float2 _233 = -_230;
        _240 = all(bool2(_232.x == float2(-0.0f, 0.0f).x, _232.y == float2(-0.0f, 0.0f).y)) && all(bool2(_233.x == float2(0.0f, -0.0f).x, _233.y == float2(0.0f, -0.0f).y));
    }
    else
    {
        _240 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _240;
    bool _251 = false;
    if (_240)
    {
        float2 _243 = float2(_7_colorGreen.y, 0.0f);
        float2 _244 = float2(0.0f, _7_colorGreen.y);
        _251 = all(bool2(_243.x == float2(1.0f, 0.0f).x, _243.y == float2(1.0f, 0.0f).y)) && all(bool2(_244.x == float2(0.0f, 1.0f).x, _244.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _251 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _251;
    bool _262 = false;
    if (_251)
    {
        float2 _254 = float2(_130, 0.0f);
        float2 _255 = float2(0.0f, _130);
        _262 = any(bool2(_254.x != float2(1.0f, 0.0f).x, _254.y != float2(1.0f, 0.0f).y)) || any(bool2(_255.x != float2(0.0f, 1.0f).x, _255.y != float2(0.0f, 1.0f).y));
    }
    else
    {
        _262 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _262;
    bool _273 = false;
    if (_262)
    {
        float2 _265 = float2(_7_colorGreen.y, 0.0f);
        float2 _266 = float2(0.0f, _7_colorGreen.y);
        _273 = all(bool2(_265.x == float2(1.0f, 0.0f).x, _265.y == float2(1.0f, 0.0f).y)) && all(bool2(_266.x == float2(0.0f, 1.0f).x, _266.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _273 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _273;
    bool _284 = false;
    if (_273)
    {
        float2 _276 = float2(_7_colorGreen.y, 0.0f);
        float2 _277 = float2(0.0f, _7_colorGreen.y);
        _284 = any(bool2(_276.x != 0.0f.xx.x, _276.y != 0.0f.xx.y)) || any(bool2(_277.x != 0.0f.xx.x, _277.y != 0.0f.xx.y));
    }
    else
    {
        _284 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _284;
    bool _303 = false;
    if (_284)
    {
        float3 _287 = float3(_7_colorGreen.y, _7_colorGreen.xx);
        float3 _288 = float3(_7_colorGreen.x, _7_colorGreen.y, _7_colorGreen.x);
        float3 _289 = float3(_7_colorGreen.xx, _7_colorGreen.y);
        _303 = (all(bool3(_287.x == float3(1.0f, 0.0f, 0.0f).x, _287.y == float3(1.0f, 0.0f, 0.0f).y, _287.z == float3(1.0f, 0.0f, 0.0f).z)) && all(bool3(_288.x == float3(0.0f, 1.0f, 0.0f).x, _288.y == float3(0.0f, 1.0f, 0.0f).y, _288.z == float3(0.0f, 1.0f, 0.0f).z))) && all(bool3(_289.x == float3(0.0f, 0.0f, 1.0f).x, _289.y == float3(0.0f, 0.0f, 1.0f).y, _289.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _303 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _303;
    bool _324 = false;
    if (_303)
    {
        float3 _306 = float3(_132, _7_colorGreen.xx);
        float3 _307 = float3(_7_colorGreen.x, _132, _7_colorGreen.x);
        float3 _308 = float3(_7_colorGreen.xx, _7_colorGreen.y);
        _324 = (all(bool3(_306.x == float3(9.0f, 0.0f, 0.0f).x, _306.y == float3(9.0f, 0.0f, 0.0f).y, _306.z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(_307.x == float3(0.0f, 9.0f, 0.0f).x, _307.y == float3(0.0f, 9.0f, 0.0f).y, _307.z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(_308.x == float3(0.0f, 0.0f, 1.0f).x, _308.y == float3(0.0f, 0.0f, 1.0f).y, _308.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _324 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _324;
    bool _339 = false;
    if (_324)
    {
        float3 _327 = float3(_7_colorGreen.y, 0.0f, 0.0f);
        float3 _328 = float3(0.0f, _7_colorGreen.y, 0.0f);
        float3 _329 = float3(0.0f, 0.0f, _7_colorGreen.y);
        _339 = (all(bool3(_327.x == float3(1.0f, 0.0f, 0.0f).x, _327.y == float3(1.0f, 0.0f, 0.0f).y, _327.z == float3(1.0f, 0.0f, 0.0f).z)) && all(bool3(_328.x == float3(0.0f, 1.0f, 0.0f).x, _328.y == float3(0.0f, 1.0f, 0.0f).y, _328.z == float3(0.0f, 1.0f, 0.0f).z))) && all(bool3(_329.x == float3(0.0f, 0.0f, 1.0f).x, _329.y == float3(0.0f, 0.0f, 1.0f).y, _329.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _339 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _339;
    bool _354 = false;
    if (_339)
    {
        float3 _342 = float3(_132, 0.0f, 0.0f);
        float3 _343 = float3(0.0f, _132, 0.0f);
        float3 _344 = float3(0.0f, 0.0f, _7_colorGreen.y);
        _354 = (all(bool3(_342.x == float3(9.0f, 0.0f, 0.0f).x, _342.y == float3(9.0f, 0.0f, 0.0f).y, _342.z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(_343.x == float3(0.0f, 9.0f, 0.0f).x, _343.y == float3(0.0f, 9.0f, 0.0f).y, _343.z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(_344.x == float3(0.0f, 0.0f, 1.0f).x, _344.y == float3(0.0f, 0.0f, 1.0f).y, _344.z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _354 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _354;
    bool _369 = false;
    if (_354)
    {
        float3 _357 = float3(_7_colorGreen.y, 0.0f, 0.0f);
        float3 _358 = float3(0.0f, _7_colorGreen.y, 0.0f);
        float2 _361 = _357.xy;
        float2 _362 = _358.xy;
        _369 = all(bool2(_361.x == float2(1.0f, 0.0f).x, _361.y == float2(1.0f, 0.0f).y)) && all(bool2(_362.x == float2(0.0f, 1.0f).x, _362.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _369 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _369;
    bool _384 = false;
    if (_369)
    {
        float3 _372 = float3(_7_colorGreen.y, 0.0f, 0.0f);
        float3 _373 = float3(0.0f, _7_colorGreen.y, 0.0f);
        float2 _376 = _372.xy;
        float2 _377 = _373.xy;
        _384 = all(bool2(_376.x == float2(1.0f, 0.0f).x, _376.y == float2(1.0f, 0.0f).y)) && all(bool2(_377.x == float2(0.0f, 1.0f).x, _377.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _384 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _384;
    bool _395 = false;
    if (_384)
    {
        float2 _387 = float2(_7_colorGreen.y, _7_colorGreen.x);
        float2 _388 = float2(_7_colorGreen.x, _7_colorGreen.y);
        _395 = all(bool2(_387.x == float2(1.0f, 0.0f).x, _387.y == float2(1.0f, 0.0f).y)) && all(bool2(_388.x == float2(0.0f, 1.0f).x, _388.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _395 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _395;
    bool _406 = false;
    if (_395)
    {
        float2 _398 = float2(_7_colorGreen.y, _7_colorGreen.x);
        float2 _399 = float2(_7_colorGreen.x, _7_colorGreen.y);
        _406 = all(bool2(_398.x == float2(1.0f, 0.0f).x, _398.y == float2(1.0f, 0.0f).y)) && all(bool2(_399.x == float2(0.0f, 1.0f).x, _399.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _406 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _406;
    bool _417 = false;
    if (_406)
    {
        float2 _409 = float2(_7_colorGreen.y, _7_colorGreen.x);
        float2 _410 = float2(_7_colorGreen.x, _7_colorGreen.y);
        _417 = all(bool2(_409.x == float2(1.0f, 0.0f).x, _409.y == float2(1.0f, 0.0f).y)) && all(bool2(_410.x == float2(0.0f, 1.0f).x, _410.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _417 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _417;
    bool _433 = false;
    if (_417)
    {
        float4 _428 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y) * _7_colorGreen.y.xxxx;
        _433 = all(bool4(_428.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _428.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _428.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _428.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _433 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _433;
    bool _454 = false;
    if (_433)
    {
        float4 _444 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y) * _7_colorGreen.y.xxxx;
        float4 _451 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y);
        _454 = all(bool4(_444.x == _451.x, _444.y == _451.y, _444.z == _451.z, _444.w == _451.w));
    }
    else
    {
        _454 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _454;
    bool _469 = false;
    if (_454)
    {
        float4 _465 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y) * _7_colorGreen.x.xxxx;
        _469 = all(bool4(_465.x == 0.0f.xxxx.x, _465.y == 0.0f.xxxx.y, _465.z == 0.0f.xxxx.z, _465.w == 0.0f.xxxx.w));
    }
    else
    {
        _469 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _469;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_5_m = float3x3(float3(_7_colorGreen.y, _130, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, _132));
    bool _482 = false;
    if (_469)
    {
        _482 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[0].x == float3(1.0f, 2.0f, 3.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[0].y == float3(1.0f, 2.0f, 3.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[0].z == float3(1.0f, 2.0f, 3.0f).z));
    }
    else
    {
        _482 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _482;
    bool _490 = false;
    if (_482)
    {
        _490 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[1].x == float3(4.0f, 5.0f, 6.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[1].y == float3(4.0f, 5.0f, 6.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[1].z == float3(4.0f, 5.0f, 6.0f).z));
    }
    else
    {
        _490 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _490;
    bool _497 = false;
    if (_490)
    {
        _497 = all(bool3(_RESERVED_IDENTIFIER_FIXUP_5_m[2].x == float3(7.0f, 8.0f, 9.0f).x, _RESERVED_IDENTIFIER_FIXUP_5_m[2].y == float3(7.0f, 8.0f, 9.0f).y, _RESERVED_IDENTIFIER_FIXUP_5_m[2].z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _497 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _497;
    bool _504 = false;
    if (_497)
    {
        _504 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].x == 1.0f;
    }
    else
    {
        _504 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _504;
    bool _511 = false;
    if (_504)
    {
        _511 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].y == 2.0f;
    }
    else
    {
        _511 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _511;
    bool _518 = false;
    if (_511)
    {
        _518 = _RESERVED_IDENTIFIER_FIXUP_5_m[0].z == 3.0f;
    }
    else
    {
        _518 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _518;
    bool _525 = false;
    if (_518)
    {
        _525 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].x == 4.0f;
    }
    else
    {
        _525 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _525;
    bool _532 = false;
    if (_525)
    {
        _532 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].y == 5.0f;
    }
    else
    {
        _532 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _532;
    bool _539 = false;
    if (_532)
    {
        _539 = _RESERVED_IDENTIFIER_FIXUP_5_m[1].z == 6.0f;
    }
    else
    {
        _539 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _539;
    bool _546 = false;
    if (_539)
    {
        _546 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].x == 7.0f;
    }
    else
    {
        _546 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _546;
    bool _553 = false;
    if (_546)
    {
        _553 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].y == 8.0f;
    }
    else
    {
        _553 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _553;
    bool _560 = false;
    if (_553)
    {
        _560 = _RESERVED_IDENTIFIER_FIXUP_5_m[2].z == 9.0f;
    }
    else
    {
        _560 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _560;
    float4 _561 = 0.0f.xxxx;
    if (_560)
    {
        _561 = _7_colorGreen;
    }
    else
    {
        _561 = _7_colorRed;
    }
    return _561;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
