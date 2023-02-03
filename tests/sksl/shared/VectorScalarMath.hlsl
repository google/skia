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
    int4 _45 = int4(int(_11_colorRed.x), int(_11_colorRed.y), int(_11_colorRed.z), int(_11_colorRed.w));
    int4 inputRed = _45;
    int4 _58 = int4(int(_11_colorGreen.x), int(_11_colorGreen.y), int(_11_colorGreen.z), int(_11_colorGreen.w));
    int4 inputGreen = _58;
    int4 _62 = _45 + int4(2, 2, 2, 2);
    int4 x = _62;
    bool _71 = false;
    if (true)
    {
        _71 = all(bool4(_62.x == int4(3, 2, 2, 3).x, _62.y == int4(3, 2, 2, 3).y, _62.z == int4(3, 2, 2, 3).z, _62.w == int4(3, 2, 2, 3).w));
    }
    else
    {
        _71 = false;
    }
    ok = _71;
    int4 _73 = _58.ywxz - int4(2, 2, 2, 2);
    x = _73;
    bool _81 = false;
    if (_71)
    {
        _81 = all(bool4(_73.x == int4(-1, -1, -2, -2).x, _73.y == int4(-1, -1, -2, -2).y, _73.z == int4(-1, -1, -2, -2).z, _73.w == int4(-1, -1, -2, -2).w));
    }
    else
    {
        _81 = false;
    }
    ok = _81;
    int4 _84 = _45 + _58.y.xxxx;
    x = _84;
    bool _90 = false;
    if (_81)
    {
        _90 = all(bool4(_84.x == int4(2, 1, 1, 2).x, _84.y == int4(2, 1, 1, 2).y, _84.z == int4(2, 1, 1, 2).z, _84.w == int4(2, 1, 1, 2).w));
    }
    else
    {
        _90 = false;
    }
    ok = _90;
    int3 _95 = _58.wyw * int3(9, 9, 9);
    int4 _96 = x;
    int4 _97 = int4(_95.x, _95.y, _95.z, _96.w);
    x = _97;
    bool _103 = false;
    if (_90)
    {
        _103 = all(bool4(_97.x == int4(9, 9, 9, 2).x, _97.y == int4(9, 9, 9, 2).y, _97.z == int4(9, 9, 9, 2).z, _97.w == int4(9, 9, 9, 2).w));
    }
    else
    {
        _103 = false;
    }
    ok = _103;
    int2 _108 = _97.zw / int2(4, 4);
    int4 _109 = x;
    int4 _110 = int4(_108.x, _108.y, _109.z, _109.w);
    x = _110;
    bool _116 = false;
    if (_103)
    {
        _116 = all(bool4(_110.x == int4(2, 0, 9, 2).x, _110.y == int4(2, 0, 9, 2).y, _110.z == int4(2, 0, 9, 2).z, _110.w == int4(2, 0, 9, 2).w));
    }
    else
    {
        _116 = false;
    }
    ok = _116;
    int4 _120 = (_45 * int4(5, 5, 5, 5)).yxwz;
    x = _120;
    bool _126 = false;
    if (_116)
    {
        _126 = all(bool4(_120.x == int4(0, 5, 5, 0).x, _120.y == int4(0, 5, 5, 0).y, _120.z == int4(0, 5, 5, 0).z, _120.w == int4(0, 5, 5, 0).w));
    }
    else
    {
        _126 = false;
    }
    ok = _126;
    int4 _127 = int4(2, 2, 2, 2) + _45;
    x = _127;
    bool _132 = false;
    if (_126)
    {
        _132 = all(bool4(_127.x == int4(3, 2, 2, 3).x, _127.y == int4(3, 2, 2, 3).y, _127.z == int4(3, 2, 2, 3).z, _127.w == int4(3, 2, 2, 3).w));
    }
    else
    {
        _132 = false;
    }
    ok = _132;
    int4 _136 = int4(10, 10, 10, 10) - _58.ywxz;
    x = _136;
    bool _142 = false;
    if (_132)
    {
        _142 = all(bool4(_136.x == int4(9, 9, 10, 10).x, _136.y == int4(9, 9, 10, 10).y, _136.z == int4(9, 9, 10, 10).z, _136.w == int4(9, 9, 10, 10).w));
    }
    else
    {
        _142 = false;
    }
    ok = _142;
    int4 _145 = _45.x.xxxx + _58;
    x = _145;
    bool _151 = false;
    if (_142)
    {
        _151 = all(bool4(_145.x == int4(1, 2, 1, 2).x, _145.y == int4(1, 2, 1, 2).y, _145.z == int4(1, 2, 1, 2).z, _145.w == int4(1, 2, 1, 2).w));
    }
    else
    {
        _151 = false;
    }
    ok = _151;
    int3 _155 = int3(8, 8, 8) * _58.wyw;
    int4 _156 = x;
    int4 _157 = int4(_155.x, _155.y, _155.z, _156.w);
    x = _157;
    bool _163 = false;
    if (_151)
    {
        _163 = all(bool4(_157.x == int4(8, 8, 8, 2).x, _157.y == int4(8, 8, 8, 2).y, _157.z == int4(8, 8, 8, 2).z, _157.w == int4(8, 8, 8, 2).w));
    }
    else
    {
        _163 = false;
    }
    ok = _163;
    int2 _167 = int2(36, 36) / _157.zw;
    int4 _168 = x;
    int4 _169 = int4(_167.x, _167.y, _168.z, _168.w);
    x = _169;
    bool _176 = false;
    if (_163)
    {
        _176 = all(bool4(_169.x == int4(4, 18, 8, 2).x, _169.y == int4(4, 18, 8, 2).y, _169.z == int4(4, 18, 8, 2).z, _169.w == int4(4, 18, 8, 2).w));
    }
    else
    {
        _176 = false;
    }
    ok = _176;
    int4 _180 = (int4(37, 37, 37, 37) / _169).yxwz;
    x = _180;
    bool _186 = false;
    if (_176)
    {
        _186 = all(bool4(_180.x == int4(2, 9, 18, 4).x, _180.y == int4(2, 9, 18, 4).y, _180.z == int4(2, 9, 18, 4).z, _180.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _186 = false;
    }
    ok = _186;
    int4 _187 = _180 + int4(2, 2, 2, 2);
    x = _187;
    int4 _188 = _187 * int4(2, 2, 2, 2);
    x = _188;
    int4 _190 = _188 - int4(4, 4, 4, 4);
    x = _190;
    int4 _191 = _190 / int4(2, 2, 2, 2);
    x = _191;
    bool _196 = false;
    if (_186)
    {
        _196 = all(bool4(_191.x == int4(2, 9, 18, 4).x, _191.y == int4(2, 9, 18, 4).y, _191.z == int4(2, 9, 18, 4).z, _191.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _196 = false;
    }
    ok = _196;
    int4 _197 = _191 + int4(2, 2, 2, 2);
    x = _197;
    int4 _198 = _197 * int4(2, 2, 2, 2);
    x = _198;
    int4 _199 = _198 - int4(4, 4, 4, 4);
    x = _199;
    int4 _200 = _199 / int4(2, 2, 2, 2);
    x = _200;
    bool _205 = false;
    if (_196)
    {
        _205 = all(bool4(_200.x == int4(2, 9, 18, 4).x, _200.y == int4(2, 9, 18, 4).y, _200.z == int4(2, 9, 18, 4).z, _200.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _205 = false;
    }
    ok = _205;
    return _205;
}

float4 main(float2 _207)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float4 _RESERVED_IDENTIFIER_FIXUP_1_inputRed = _11_colorRed;
    float4 _RESERVED_IDENTIFIER_FIXUP_2_inputGreen = _11_colorGreen;
    float4 _220 = _11_colorRed + 2.0f.xxxx;
    float4 _RESERVED_IDENTIFIER_FIXUP_3_x = _220;
    bool _227 = false;
    if (true)
    {
        _227 = all(bool4(_220.x == float4(3.0f, 2.0f, 2.0f, 3.0f).x, _220.y == float4(3.0f, 2.0f, 2.0f, 3.0f).y, _220.z == float4(3.0f, 2.0f, 2.0f, 3.0f).z, _220.w == float4(3.0f, 2.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _227 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _227;
    float4 _229 = _11_colorGreen.ywxz - 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _229;
    bool _237 = false;
    if (_227)
    {
        _237 = all(bool4(_229.x == float4(-1.0f, -1.0f, -2.0f, -2.0f).x, _229.y == float4(-1.0f, -1.0f, -2.0f, -2.0f).y, _229.z == float4(-1.0f, -1.0f, -2.0f, -2.0f).z, _229.w == float4(-1.0f, -1.0f, -2.0f, -2.0f).w));
    }
    else
    {
        _237 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _237;
    float4 _240 = _11_colorRed + _11_colorGreen.y.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _240;
    bool _247 = false;
    if (_237)
    {
        _247 = all(bool4(_240.x == float4(2.0f, 1.0f, 1.0f, 2.0f).x, _240.y == float4(2.0f, 1.0f, 1.0f, 2.0f).y, _240.z == float4(2.0f, 1.0f, 1.0f, 2.0f).z, _240.w == float4(2.0f, 1.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _247 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _247;
    float3 _251 = _11_colorGreen.wyw * 9.0f;
    float4 _252 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _253 = float4(_251.x, _251.y, _251.z, _252.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _253;
    bool _259 = false;
    if (_247)
    {
        _259 = all(bool4(_253.x == float4(9.0f, 9.0f, 9.0f, 2.0f).x, _253.y == float4(9.0f, 9.0f, 9.0f, 2.0f).y, _253.z == float4(9.0f, 9.0f, 9.0f, 2.0f).z, _253.w == float4(9.0f, 9.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _259 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _259;
    float2 _261 = _253.zw * 2.0f;
    float4 _262 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _263 = float4(_261.x, _261.y, _262.z, _262.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _263;
    bool _271 = false;
    if (_259)
    {
        _271 = all(bool4(_263.x == float4(18.0f, 4.0f, 9.0f, 2.0f).x, _263.y == float4(18.0f, 4.0f, 9.0f, 2.0f).y, _263.z == float4(18.0f, 4.0f, 9.0f, 2.0f).z, _263.w == float4(18.0f, 4.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _271 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _271;
    float4 _274 = (_11_colorRed * 5.0f).yxwz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _274;
    bool _280 = false;
    if (_271)
    {
        _280 = all(bool4(_274.x == float4(0.0f, 5.0f, 5.0f, 0.0f).x, _274.y == float4(0.0f, 5.0f, 5.0f, 0.0f).y, _274.z == float4(0.0f, 5.0f, 5.0f, 0.0f).z, _274.w == float4(0.0f, 5.0f, 5.0f, 0.0f).w));
    }
    else
    {
        _280 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _280;
    float4 _281 = 2.0f.xxxx + _11_colorRed;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _281;
    bool _286 = false;
    if (_280)
    {
        _286 = all(bool4(_281.x == float4(3.0f, 2.0f, 2.0f, 3.0f).x, _281.y == float4(3.0f, 2.0f, 2.0f, 3.0f).y, _281.z == float4(3.0f, 2.0f, 2.0f, 3.0f).z, _281.w == float4(3.0f, 2.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _286 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _286;
    float4 _290 = 10.0f.xxxx - _11_colorGreen.ywxz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _290;
    bool _296 = false;
    if (_286)
    {
        _296 = all(bool4(_290.x == float4(9.0f, 9.0f, 10.0f, 10.0f).x, _290.y == float4(9.0f, 9.0f, 10.0f, 10.0f).y, _290.z == float4(9.0f, 9.0f, 10.0f, 10.0f).z, _290.w == float4(9.0f, 9.0f, 10.0f, 10.0f).w));
    }
    else
    {
        _296 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _296;
    float4 _299 = _11_colorRed.x.xxxx + _11_colorGreen;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _299;
    bool _305 = false;
    if (_296)
    {
        _305 = all(bool4(_299.x == float4(1.0f, 2.0f, 1.0f, 2.0f).x, _299.y == float4(1.0f, 2.0f, 1.0f, 2.0f).y, _299.z == float4(1.0f, 2.0f, 1.0f, 2.0f).z, _299.w == float4(1.0f, 2.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _305 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _305;
    float3 _308 = _11_colorGreen.wyw * 8.0f;
    float4 _309 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _310 = float4(_308.x, _308.y, _308.z, _309.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _310;
    bool _316 = false;
    if (_305)
    {
        _316 = all(bool4(_310.x == float4(8.0f, 8.0f, 8.0f, 2.0f).x, _310.y == float4(8.0f, 8.0f, 8.0f, 2.0f).y, _310.z == float4(8.0f, 8.0f, 8.0f, 2.0f).z, _310.w == float4(8.0f, 8.0f, 8.0f, 2.0f).w));
    }
    else
    {
        _316 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _316;
    float2 _320 = 32.0f.xx / _310.zw;
    float4 _321 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _322 = float4(_320.x, _320.y, _321.z, _321.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _322;
    bool _329 = false;
    if (_316)
    {
        _329 = all(bool4(_322.x == float4(4.0f, 16.0f, 8.0f, 2.0f).x, _322.y == float4(4.0f, 16.0f, 8.0f, 2.0f).y, _322.z == float4(4.0f, 16.0f, 8.0f, 2.0f).z, _322.w == float4(4.0f, 16.0f, 8.0f, 2.0f).w));
    }
    else
    {
        _329 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _329;
    float4 _332 = (32.0f.xxxx / _322).yxwz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _332;
    bool _338 = false;
    if (_329)
    {
        _338 = all(bool4(_332.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _332.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _332.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _332.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _338 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _338;
    float4 _339 = _332 + 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _339;
    float4 _340 = _339 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _340;
    float4 _342 = _340 - 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _342;
    float4 _344 = _342 * 0.5f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _344;
    bool _349 = false;
    if (_338)
    {
        _349 = all(bool4(_344.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _344.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _344.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _344.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _349 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _349;
    float4 _350 = _344 + 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _350;
    float4 _351 = _350 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _351;
    float4 _352 = _351 - 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _352;
    float4 _353 = _352 * 0.5f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _353;
    bool _358 = false;
    if (_349)
    {
        _358 = all(bool4(_353.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _353.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _353.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _353.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _358 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _358;
    bool _362 = false;
    if (_358)
    {
        _362 = test_int_b();
    }
    else
    {
        _362 = false;
    }
    float4 _363 = 0.0f.xxxx;
    if (_362)
    {
        _363 = _11_colorGreen;
    }
    else
    {
        _363 = _11_colorRed;
    }
    return _363;
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
