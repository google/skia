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
    bool _73 = false;
    if (ok)
    {
        float3x3 _34 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
        float3 _41 = 4.0f.xxx;
        float3x3 _42 = float3x3(_41, _41, _41);
        float3x3 _52 = float3x3(_34[0] + _42[0], _34[1] + _42[1], _34[2] + _42[2]);
        float3x3 _57 = float3x3(float3(6.0f, 4.0f, 4.0f), float3(4.0f, 6.0f, 4.0f), float3(4.0f, 4.0f, 6.0f));
        float3 _59 = _52[0];
        float3 _60 = _57[0];
        float3 _63 = _52[1];
        float3 _64 = _57[1];
        float3 _68 = _52[2];
        float3 _69 = _57[2];
        _73 = (all(bool3(_59.x == _60.x, _59.y == _60.y, _59.z == _60.z)) && all(bool3(_63.x == _64.x, _63.y == _64.y, _63.z == _64.z))) && all(bool3(_68.x == _69.x, _68.y == _69.y, _68.z == _69.z));
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    bool _113 = false;
    if (ok)
    {
        float3x3 _77 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
        float3 _81 = 4.0f.xxx;
        float3x3 _82 = float3x3(_81, _81, _81);
        float3x3 _92 = float3x3(_77[0] - _82[0], _77[1] - _82[1], _77[2] - _82[2]);
        float3x3 _98 = float3x3(float3(-2.0f, -4.0f, -4.0f), float3(-4.0f, -2.0f, -4.0f), float3(-4.0f, -4.0f, -2.0f));
        float3 _99 = _92[0];
        float3 _100 = _98[0];
        float3 _103 = _92[1];
        float3 _104 = _98[1];
        float3 _108 = _92[2];
        float3 _109 = _98[2];
        _113 = (all(bool3(_99.x == _100.x, _99.y == _100.y, _99.z == _100.z)) && all(bool3(_103.x == _104.x, _103.y == _104.y, _103.z == _104.z))) && all(bool3(_108.x == _109.x, _108.y == _109.y, _108.z == _109.z));
    }
    else
    {
        _113 = false;
    }
    ok = _113;
    bool _141 = false;
    if (ok)
    {
        float3x3 _121 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f)) * 4.0f;
        float3x3 _123 = float3x3(float3(8.0f, 0.0f, 0.0f), float3(0.0f, 8.0f, 0.0f), float3(0.0f, 0.0f, 8.0f));
        float3 _127 = _121[0];
        float3 _128 = _123[0];
        float3 _131 = _121[1];
        float3 _132 = _123[1];
        float3 _136 = _121[2];
        float3 _137 = _123[2];
        _141 = (all(bool3(_127.x == _128.x, _127.y == _128.y, _127.z == _128.z)) && all(bool3(_131.x == _132.x, _131.y == _132.y, _131.z == _132.z))) && all(bool3(_136.x == _137.x, _136.y == _137.y, _136.z == _137.z));
    }
    else
    {
        _141 = false;
    }
    ok = _141;
    bool _170 = false;
    if (ok)
    {
        float3x3 _150 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f)) * 0.25f;
        float3x3 _152 = float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f));
        float3 _156 = _150[0];
        float3 _157 = _152[0];
        float3 _160 = _150[1];
        float3 _161 = _152[1];
        float3 _165 = _150[2];
        float3 _166 = _152[2];
        _170 = (all(bool3(_156.x == _157.x, _156.y == _157.y, _156.z == _157.z)) && all(bool3(_160.x == _161.x, _160.y == _161.y, _160.z == _161.z))) && all(bool3(_165.x == _166.x, _165.y == _166.y, _165.z == _166.z));
    }
    else
    {
        _170 = false;
    }
    ok = _170;
    bool _208 = false;
    if (ok)
    {
        float3x3 _174 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
        float3 _178 = 4.0f.xxx;
        float3x3 _179 = float3x3(_178, _178, _178);
        float3x3 _189 = float3x3(_179[0] + _174[0], _179[1] + _174[1], _179[2] + _174[2]);
        float3x3 _193 = float3x3(float3(6.0f, 4.0f, 4.0f), float3(4.0f, 6.0f, 4.0f), float3(4.0f, 4.0f, 6.0f));
        float3 _194 = _189[0];
        float3 _195 = _193[0];
        float3 _198 = _189[1];
        float3 _199 = _193[1];
        float3 _203 = _189[2];
        float3 _204 = _193[2];
        _208 = (all(bool3(_194.x == _195.x, _194.y == _195.y, _194.z == _195.z)) && all(bool3(_198.x == _199.x, _198.y == _199.y, _198.z == _199.z))) && all(bool3(_203.x == _204.x, _203.y == _204.y, _203.z == _204.z));
    }
    else
    {
        _208 = false;
    }
    ok = _208;
    bool _246 = false;
    if (ok)
    {
        float3x3 _212 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
        float3 _216 = 4.0f.xxx;
        float3x3 _217 = float3x3(_216, _216, _216);
        float3x3 _227 = float3x3(_217[0] - _212[0], _217[1] - _212[1], _217[2] - _212[2]);
        float3x3 _231 = float3x3(float3(2.0f, 4.0f, 4.0f), float3(4.0f, 2.0f, 4.0f), float3(4.0f, 4.0f, 2.0f));
        float3 _232 = _227[0];
        float3 _233 = _231[0];
        float3 _236 = _227[1];
        float3 _237 = _231[1];
        float3 _241 = _227[2];
        float3 _242 = _231[2];
        _246 = (all(bool3(_232.x == _233.x, _232.y == _233.y, _232.z == _233.z)) && all(bool3(_236.x == _237.x, _236.y == _237.y, _236.z == _237.z))) && all(bool3(_241.x == _242.x, _241.y == _242.y, _241.z == _242.z));
    }
    else
    {
        _246 = false;
    }
    ok = _246;
    bool _273 = false;
    if (ok)
    {
        float3x3 _254 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f)) * 4.0f;
        float3x3 _255 = float3x3(float3(8.0f, 0.0f, 0.0f), float3(0.0f, 8.0f, 0.0f), float3(0.0f, 0.0f, 8.0f));
        float3 _259 = _254[0];
        float3 _260 = _255[0];
        float3 _263 = _254[1];
        float3 _264 = _255[1];
        float3 _268 = _254[2];
        float3 _269 = _255[2];
        _273 = (all(bool3(_259.x == _260.x, _259.y == _260.y, _259.z == _260.z)) && all(bool3(_263.x == _264.x, _263.y == _264.y, _263.z == _264.z))) && all(bool3(_268.x == _269.x, _268.y == _269.y, _268.z == _269.z));
    }
    else
    {
        _273 = false;
    }
    ok = _273;
    bool _303 = false;
    if (ok)
    {
        float2x2 _279 = float2x2(2.0f.xx, 2.0f.xx);
        float2 _281 = 4.0f.xx;
        float2x2 _282 = float2x2(_281, _281);
        float2x2 _289 = float2x2(_282[0] / _279[0], _282[1] / _279[1]);
        float2x2 _292 = float2x2(2.0f.xx, 2.0f.xx);
        float2 _294 = _289[0];
        float2 _295 = _292[0];
        float2 _298 = _289[1];
        float2 _299 = _292[1];
        _303 = all(bool2(_294.x == _295.x, _294.y == _295.y)) && all(bool2(_298.x == _299.x, _298.y == _299.y));
    }
    else
    {
        _303 = false;
    }
    ok = _303;
    return ok;
}

float4 main(float2 _306)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    bool _346 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _312 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
        float3 _316 = 4.0f.xxx;
        float3x3 _317 = float3x3(_316, _316, _316);
        float3x3 _327 = float3x3(_312[0] + _317[0], _312[1] + _317[1], _312[2] + _317[2]);
        float3x3 _331 = float3x3(float3(6.0f, 4.0f, 4.0f), float3(4.0f, 6.0f, 4.0f), float3(4.0f, 4.0f, 6.0f));
        float3 _332 = _327[0];
        float3 _333 = _331[0];
        float3 _336 = _327[1];
        float3 _337 = _331[1];
        float3 _341 = _327[2];
        float3 _342 = _331[2];
        _346 = (all(bool3(_332.x == _333.x, _332.y == _333.y, _332.z == _333.z)) && all(bool3(_336.x == _337.x, _336.y == _337.y, _336.z == _337.z))) && all(bool3(_341.x == _342.x, _341.y == _342.y, _341.z == _342.z));
    }
    else
    {
        _346 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _346;
    bool _384 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _350 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
        float3 _354 = 4.0f.xxx;
        float3x3 _355 = float3x3(_354, _354, _354);
        float3x3 _365 = float3x3(_350[0] - _355[0], _350[1] - _355[1], _350[2] - _355[2]);
        float3x3 _369 = float3x3(float3(-2.0f, -4.0f, -4.0f), float3(-4.0f, -2.0f, -4.0f), float3(-4.0f, -4.0f, -2.0f));
        float3 _370 = _365[0];
        float3 _371 = _369[0];
        float3 _374 = _365[1];
        float3 _375 = _369[1];
        float3 _379 = _365[2];
        float3 _380 = _369[2];
        _384 = (all(bool3(_370.x == _371.x, _370.y == _371.y, _370.z == _371.z)) && all(bool3(_374.x == _375.x, _374.y == _375.y, _374.z == _375.z))) && all(bool3(_379.x == _380.x, _379.y == _380.y, _379.z == _380.z));
    }
    else
    {
        _384 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _384;
    bool _411 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _392 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f)) * 4.0f;
        float3x3 _393 = float3x3(float3(8.0f, 0.0f, 0.0f), float3(0.0f, 8.0f, 0.0f), float3(0.0f, 0.0f, 8.0f));
        float3 _397 = _392[0];
        float3 _398 = _393[0];
        float3 _401 = _392[1];
        float3 _402 = _393[1];
        float3 _406 = _392[2];
        float3 _407 = _393[2];
        _411 = (all(bool3(_397.x == _398.x, _397.y == _398.y, _397.z == _398.z)) && all(bool3(_401.x == _402.x, _401.y == _402.y, _401.z == _402.z))) && all(bool3(_406.x == _407.x, _406.y == _407.y, _406.z == _407.z));
    }
    else
    {
        _411 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _411;
    bool _438 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _419 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f)) * 0.25f;
        float3x3 _420 = float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f));
        float3 _424 = _419[0];
        float3 _425 = _420[0];
        float3 _428 = _419[1];
        float3 _429 = _420[1];
        float3 _433 = _419[2];
        float3 _434 = _420[2];
        _438 = (all(bool3(_424.x == _425.x, _424.y == _425.y, _424.z == _425.z)) && all(bool3(_428.x == _429.x, _428.y == _429.y, _428.z == _429.z))) && all(bool3(_433.x == _434.x, _433.y == _434.y, _433.z == _434.z));
    }
    else
    {
        _438 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _438;
    bool _476 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _442 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
        float3 _446 = 4.0f.xxx;
        float3x3 _447 = float3x3(_446, _446, _446);
        float3x3 _457 = float3x3(_447[0] + _442[0], _447[1] + _442[1], _447[2] + _442[2]);
        float3x3 _461 = float3x3(float3(6.0f, 4.0f, 4.0f), float3(4.0f, 6.0f, 4.0f), float3(4.0f, 4.0f, 6.0f));
        float3 _462 = _457[0];
        float3 _463 = _461[0];
        float3 _466 = _457[1];
        float3 _467 = _461[1];
        float3 _471 = _457[2];
        float3 _472 = _461[2];
        _476 = (all(bool3(_462.x == _463.x, _462.y == _463.y, _462.z == _463.z)) && all(bool3(_466.x == _467.x, _466.y == _467.y, _466.z == _467.z))) && all(bool3(_471.x == _472.x, _471.y == _472.y, _471.z == _472.z));
    }
    else
    {
        _476 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _476;
    bool _514 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _480 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
        float3 _484 = 4.0f.xxx;
        float3x3 _485 = float3x3(_484, _484, _484);
        float3x3 _495 = float3x3(_485[0] - _480[0], _485[1] - _480[1], _485[2] - _480[2]);
        float3x3 _499 = float3x3(float3(2.0f, 4.0f, 4.0f), float3(4.0f, 2.0f, 4.0f), float3(4.0f, 4.0f, 2.0f));
        float3 _500 = _495[0];
        float3 _501 = _499[0];
        float3 _504 = _495[1];
        float3 _505 = _499[1];
        float3 _509 = _495[2];
        float3 _510 = _499[2];
        _514 = (all(bool3(_500.x == _501.x, _500.y == _501.y, _500.z == _501.z)) && all(bool3(_504.x == _505.x, _504.y == _505.y, _504.z == _505.z))) && all(bool3(_509.x == _510.x, _509.y == _510.y, _509.z == _510.z));
    }
    else
    {
        _514 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _514;
    bool _541 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _522 = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f)) * 4.0f;
        float3x3 _523 = float3x3(float3(8.0f, 0.0f, 0.0f), float3(0.0f, 8.0f, 0.0f), float3(0.0f, 0.0f, 8.0f));
        float3 _527 = _522[0];
        float3 _528 = _523[0];
        float3 _531 = _522[1];
        float3 _532 = _523[1];
        float3 _536 = _522[2];
        float3 _537 = _523[2];
        _541 = (all(bool3(_527.x == _528.x, _527.y == _528.y, _527.z == _528.z)) && all(bool3(_531.x == _532.x, _531.y == _532.y, _531.z == _532.z))) && all(bool3(_536.x == _537.x, _536.y == _537.y, _536.z == _537.z));
    }
    else
    {
        _541 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _541;
    bool _569 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _547 = float2x2(2.0f.xx, 2.0f.xx);
        float2 _548 = 4.0f.xx;
        float2x2 _549 = float2x2(_548, _548);
        float2x2 _556 = float2x2(_549[0] / _547[0], _549[1] / _547[1]);
        float2x2 _559 = float2x2(2.0f.xx, 2.0f.xx);
        float2 _560 = _556[0];
        float2 _561 = _559[0];
        float2 _564 = _556[1];
        float2 _565 = _559[1];
        _569 = all(bool2(_560.x == _561.x, _560.y == _561.y)) && all(bool2(_564.x == _565.x, _564.y == _565.y));
    }
    else
    {
        _569 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _569;
    bool _574 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _574 = test_half_b();
    }
    else
    {
        _574 = false;
    }
    float4 _575 = 0.0f.xxxx;
    if (_574)
    {
        _575 = _11_colorGreen;
    }
    else
    {
        _575 = _11_colorRed;
    }
    return _575;
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
