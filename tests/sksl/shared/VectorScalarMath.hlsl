cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorRed : packoffset(c0);
    float4 _8_colorGreen : packoffset(c1);
    float _8_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_int_b()
{
    bool ok = true;
    int4 _43 = int4(int(_8_colorRed.x), int(_8_colorRed.y), int(_8_colorRed.z), int(_8_colorRed.w));
    int4 inputRed = _43;
    int4 _56 = int4(int(_8_colorGreen.x), int(_8_colorGreen.y), int(_8_colorGreen.z), int(_8_colorGreen.w));
    int4 inputGreen = _56;
    int4 _60 = _43 + int4(2, 2, 2, 2);
    int4 x = _60;
    bool _69 = false;
    if (true)
    {
        _69 = all(bool4(_60.x == int4(3, 2, 2, 3).x, _60.y == int4(3, 2, 2, 3).y, _60.z == int4(3, 2, 2, 3).z, _60.w == int4(3, 2, 2, 3).w));
    }
    else
    {
        _69 = false;
    }
    ok = _69;
    int4 _71 = _56.ywxz - int4(2, 2, 2, 2);
    x = _71;
    bool _79 = false;
    if (_69)
    {
        _79 = all(bool4(_71.x == int4(-1, -1, -2, -2).x, _71.y == int4(-1, -1, -2, -2).y, _71.z == int4(-1, -1, -2, -2).z, _71.w == int4(-1, -1, -2, -2).w));
    }
    else
    {
        _79 = false;
    }
    ok = _79;
    int4 _82 = _43 + _56.y.xxxx;
    x = _82;
    bool _88 = false;
    if (_79)
    {
        _88 = all(bool4(_82.x == int4(2, 1, 1, 2).x, _82.y == int4(2, 1, 1, 2).y, _82.z == int4(2, 1, 1, 2).z, _82.w == int4(2, 1, 1, 2).w));
    }
    else
    {
        _88 = false;
    }
    ok = _88;
    int3 _93 = _56.wyw * int3(9, 9, 9);
    int4 _94 = x;
    int4 _95 = int4(_93.x, _93.y, _93.z, _94.w);
    x = _95;
    bool _101 = false;
    if (_88)
    {
        _101 = all(bool4(_95.x == int4(9, 9, 9, 2).x, _95.y == int4(9, 9, 9, 2).y, _95.z == int4(9, 9, 9, 2).z, _95.w == int4(9, 9, 9, 2).w));
    }
    else
    {
        _101 = false;
    }
    ok = _101;
    int2 _106 = _95.zw / int2(4, 4);
    int4 _107 = x;
    int4 _108 = int4(_106.x, _106.y, _107.z, _107.w);
    x = _108;
    bool _114 = false;
    if (_101)
    {
        _114 = all(bool4(_108.x == int4(2, 0, 9, 2).x, _108.y == int4(2, 0, 9, 2).y, _108.z == int4(2, 0, 9, 2).z, _108.w == int4(2, 0, 9, 2).w));
    }
    else
    {
        _114 = false;
    }
    ok = _114;
    int4 _118 = (_43 * int4(5, 5, 5, 5)).yxwz;
    x = _118;
    bool _124 = false;
    if (_114)
    {
        _124 = all(bool4(_118.x == int4(0, 5, 5, 0).x, _118.y == int4(0, 5, 5, 0).y, _118.z == int4(0, 5, 5, 0).z, _118.w == int4(0, 5, 5, 0).w));
    }
    else
    {
        _124 = false;
    }
    ok = _124;
    int4 _125 = int4(2, 2, 2, 2) + _43;
    x = _125;
    bool _130 = false;
    if (_124)
    {
        _130 = all(bool4(_125.x == int4(3, 2, 2, 3).x, _125.y == int4(3, 2, 2, 3).y, _125.z == int4(3, 2, 2, 3).z, _125.w == int4(3, 2, 2, 3).w));
    }
    else
    {
        _130 = false;
    }
    ok = _130;
    int4 _134 = int4(10, 10, 10, 10) - _56.ywxz;
    x = _134;
    bool _140 = false;
    if (_130)
    {
        _140 = all(bool4(_134.x == int4(9, 9, 10, 10).x, _134.y == int4(9, 9, 10, 10).y, _134.z == int4(9, 9, 10, 10).z, _134.w == int4(9, 9, 10, 10).w));
    }
    else
    {
        _140 = false;
    }
    ok = _140;
    int4 _143 = _43.x.xxxx + _56;
    x = _143;
    bool _149 = false;
    if (_140)
    {
        _149 = all(bool4(_143.x == int4(1, 2, 1, 2).x, _143.y == int4(1, 2, 1, 2).y, _143.z == int4(1, 2, 1, 2).z, _143.w == int4(1, 2, 1, 2).w));
    }
    else
    {
        _149 = false;
    }
    ok = _149;
    int3 _153 = int3(8, 8, 8) * _56.wyw;
    int4 _154 = x;
    int4 _155 = int4(_153.x, _153.y, _153.z, _154.w);
    x = _155;
    bool _161 = false;
    if (_149)
    {
        _161 = all(bool4(_155.x == int4(8, 8, 8, 2).x, _155.y == int4(8, 8, 8, 2).y, _155.z == int4(8, 8, 8, 2).z, _155.w == int4(8, 8, 8, 2).w));
    }
    else
    {
        _161 = false;
    }
    ok = _161;
    int2 _165 = int2(36, 36) / _155.zw;
    int4 _166 = x;
    int4 _167 = int4(_165.x, _165.y, _166.z, _166.w);
    x = _167;
    bool _174 = false;
    if (_161)
    {
        _174 = all(bool4(_167.x == int4(4, 18, 8, 2).x, _167.y == int4(4, 18, 8, 2).y, _167.z == int4(4, 18, 8, 2).z, _167.w == int4(4, 18, 8, 2).w));
    }
    else
    {
        _174 = false;
    }
    ok = _174;
    int4 _178 = (int4(37, 37, 37, 37) / _167).yxwz;
    x = _178;
    bool _184 = false;
    if (_174)
    {
        _184 = all(bool4(_178.x == int4(2, 9, 18, 4).x, _178.y == int4(2, 9, 18, 4).y, _178.z == int4(2, 9, 18, 4).z, _178.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _184 = false;
    }
    ok = _184;
    int4 _185 = _178 + int4(2, 2, 2, 2);
    x = _185;
    int4 _186 = _185 * int4(2, 2, 2, 2);
    x = _186;
    int4 _188 = _186 - int4(4, 4, 4, 4);
    x = _188;
    int4 _189 = _188 / int4(2, 2, 2, 2);
    x = _189;
    bool _194 = false;
    if (_184)
    {
        _194 = all(bool4(_189.x == int4(2, 9, 18, 4).x, _189.y == int4(2, 9, 18, 4).y, _189.z == int4(2, 9, 18, 4).z, _189.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _194 = false;
    }
    ok = _194;
    int4 _195 = _189 + int4(2, 2, 2, 2);
    x = _195;
    int4 _196 = _195 * int4(2, 2, 2, 2);
    x = _196;
    int4 _197 = _196 - int4(4, 4, 4, 4);
    x = _197;
    int4 _198 = _197 / int4(2, 2, 2, 2);
    x = _198;
    bool _203 = false;
    if (_194)
    {
        _203 = all(bool4(_198.x == int4(2, 9, 18, 4).x, _198.y == int4(2, 9, 18, 4).y, _198.z == int4(2, 9, 18, 4).z, _198.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _203 = false;
    }
    ok = _203;
    return _203;
}

float4 main(float2 _205)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float4 _RESERVED_IDENTIFIER_FIXUP_1_inputRed = _8_colorRed;
    float4 _RESERVED_IDENTIFIER_FIXUP_2_inputGreen = _8_colorGreen;
    float4 _218 = _8_colorRed + 2.0f.xxxx;
    float4 _RESERVED_IDENTIFIER_FIXUP_3_x = _218;
    bool _225 = false;
    if (true)
    {
        _225 = all(bool4(_218.x == float4(3.0f, 2.0f, 2.0f, 3.0f).x, _218.y == float4(3.0f, 2.0f, 2.0f, 3.0f).y, _218.z == float4(3.0f, 2.0f, 2.0f, 3.0f).z, _218.w == float4(3.0f, 2.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _225 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _225;
    float4 _227 = _8_colorGreen.ywxz - 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _227;
    bool _235 = false;
    if (_225)
    {
        _235 = all(bool4(_227.x == float4(-1.0f, -1.0f, -2.0f, -2.0f).x, _227.y == float4(-1.0f, -1.0f, -2.0f, -2.0f).y, _227.z == float4(-1.0f, -1.0f, -2.0f, -2.0f).z, _227.w == float4(-1.0f, -1.0f, -2.0f, -2.0f).w));
    }
    else
    {
        _235 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _235;
    float4 _238 = _8_colorRed + _8_colorGreen.y.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _238;
    bool _245 = false;
    if (_235)
    {
        _245 = all(bool4(_238.x == float4(2.0f, 1.0f, 1.0f, 2.0f).x, _238.y == float4(2.0f, 1.0f, 1.0f, 2.0f).y, _238.z == float4(2.0f, 1.0f, 1.0f, 2.0f).z, _238.w == float4(2.0f, 1.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _245 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _245;
    float3 _249 = _8_colorGreen.wyw * 9.0f;
    float4 _250 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _251 = float4(_249.x, _249.y, _249.z, _250.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _251;
    bool _257 = false;
    if (_245)
    {
        _257 = all(bool4(_251.x == float4(9.0f, 9.0f, 9.0f, 2.0f).x, _251.y == float4(9.0f, 9.0f, 9.0f, 2.0f).y, _251.z == float4(9.0f, 9.0f, 9.0f, 2.0f).z, _251.w == float4(9.0f, 9.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _257 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _257;
    float2 _259 = _251.zw * 2.0f;
    float4 _260 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _261 = float4(_259.x, _259.y, _260.z, _260.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _261;
    bool _269 = false;
    if (_257)
    {
        _269 = all(bool4(_261.x == float4(18.0f, 4.0f, 9.0f, 2.0f).x, _261.y == float4(18.0f, 4.0f, 9.0f, 2.0f).y, _261.z == float4(18.0f, 4.0f, 9.0f, 2.0f).z, _261.w == float4(18.0f, 4.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _269 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _269;
    float4 _272 = (_8_colorRed * 5.0f).yxwz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _272;
    bool _278 = false;
    if (_269)
    {
        _278 = all(bool4(_272.x == float4(0.0f, 5.0f, 5.0f, 0.0f).x, _272.y == float4(0.0f, 5.0f, 5.0f, 0.0f).y, _272.z == float4(0.0f, 5.0f, 5.0f, 0.0f).z, _272.w == float4(0.0f, 5.0f, 5.0f, 0.0f).w));
    }
    else
    {
        _278 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _278;
    float4 _279 = 2.0f.xxxx + _8_colorRed;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _279;
    bool _284 = false;
    if (_278)
    {
        _284 = all(bool4(_279.x == float4(3.0f, 2.0f, 2.0f, 3.0f).x, _279.y == float4(3.0f, 2.0f, 2.0f, 3.0f).y, _279.z == float4(3.0f, 2.0f, 2.0f, 3.0f).z, _279.w == float4(3.0f, 2.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _284 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _284;
    float4 _288 = 10.0f.xxxx - _8_colorGreen.ywxz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _288;
    bool _294 = false;
    if (_284)
    {
        _294 = all(bool4(_288.x == float4(9.0f, 9.0f, 10.0f, 10.0f).x, _288.y == float4(9.0f, 9.0f, 10.0f, 10.0f).y, _288.z == float4(9.0f, 9.0f, 10.0f, 10.0f).z, _288.w == float4(9.0f, 9.0f, 10.0f, 10.0f).w));
    }
    else
    {
        _294 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _294;
    float4 _297 = _8_colorRed.x.xxxx + _8_colorGreen;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _297;
    bool _303 = false;
    if (_294)
    {
        _303 = all(bool4(_297.x == float4(1.0f, 2.0f, 1.0f, 2.0f).x, _297.y == float4(1.0f, 2.0f, 1.0f, 2.0f).y, _297.z == float4(1.0f, 2.0f, 1.0f, 2.0f).z, _297.w == float4(1.0f, 2.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _303 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _303;
    float3 _306 = _8_colorGreen.wyw * 8.0f;
    float4 _307 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _308 = float4(_306.x, _306.y, _306.z, _307.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _308;
    bool _314 = false;
    if (_303)
    {
        _314 = all(bool4(_308.x == float4(8.0f, 8.0f, 8.0f, 2.0f).x, _308.y == float4(8.0f, 8.0f, 8.0f, 2.0f).y, _308.z == float4(8.0f, 8.0f, 8.0f, 2.0f).z, _308.w == float4(8.0f, 8.0f, 8.0f, 2.0f).w));
    }
    else
    {
        _314 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _314;
    float2 _318 = 32.0f.xx / _308.zw;
    float4 _319 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _320 = float4(_318.x, _318.y, _319.z, _319.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _320;
    bool _327 = false;
    if (_314)
    {
        _327 = all(bool4(_320.x == float4(4.0f, 16.0f, 8.0f, 2.0f).x, _320.y == float4(4.0f, 16.0f, 8.0f, 2.0f).y, _320.z == float4(4.0f, 16.0f, 8.0f, 2.0f).z, _320.w == float4(4.0f, 16.0f, 8.0f, 2.0f).w));
    }
    else
    {
        _327 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _327;
    float4 _330 = (32.0f.xxxx / _320).yxwz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _330;
    bool _336 = false;
    if (_327)
    {
        _336 = all(bool4(_330.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _330.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _330.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _330.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _336 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _336;
    float4 _337 = _330 + 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _337;
    float4 _338 = _337 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _338;
    float4 _340 = _338 - 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _340;
    float4 _342 = _340 * 0.5f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _342;
    bool _347 = false;
    if (_336)
    {
        _347 = all(bool4(_342.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _342.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _342.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _342.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _347 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _347;
    float4 _348 = _342 + 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _348;
    float4 _349 = _348 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _349;
    float4 _350 = _349 - 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _350;
    float4 _351 = _350 * 0.5f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _351;
    bool _356 = false;
    if (_347)
    {
        _356 = all(bool4(_351.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _351.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _351.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _351.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _356 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _356;
    bool _360 = false;
    if (_356)
    {
        _360 = test_int_b();
    }
    else
    {
        _360 = false;
    }
    float4 _361 = 0.0f.xxxx;
    if (_360)
    {
        _361 = _8_colorGreen;
    }
    else
    {
        _361 = _8_colorRed;
    }
    return _361;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
