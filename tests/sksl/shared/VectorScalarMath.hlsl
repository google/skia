cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorRed : packoffset(c0);
    float4 _12_colorGreen : packoffset(c1);
    float _12_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_int_b()
{
    bool ok = true;
    int4 _46 = int4(int(_12_colorRed.x), int(_12_colorRed.y), int(_12_colorRed.z), int(_12_colorRed.w));
    int4 inputRed = _46;
    int4 _59 = int4(int(_12_colorGreen.x), int(_12_colorGreen.y), int(_12_colorGreen.z), int(_12_colorGreen.w));
    int4 inputGreen = _59;
    int4 _63 = _46 + int4(2, 2, 2, 2);
    int4 x = _63;
    bool _72 = false;
    if (true)
    {
        _72 = all(bool4(_63.x == int4(3, 2, 2, 3).x, _63.y == int4(3, 2, 2, 3).y, _63.z == int4(3, 2, 2, 3).z, _63.w == int4(3, 2, 2, 3).w));
    }
    else
    {
        _72 = false;
    }
    ok = _72;
    int4 _74 = _59.ywxz - int4(2, 2, 2, 2);
    x = _74;
    bool _82 = false;
    if (_72)
    {
        _82 = all(bool4(_74.x == int4(-1, -1, -2, -2).x, _74.y == int4(-1, -1, -2, -2).y, _74.z == int4(-1, -1, -2, -2).z, _74.w == int4(-1, -1, -2, -2).w));
    }
    else
    {
        _82 = false;
    }
    ok = _82;
    int4 _85 = _46 + _59.y.xxxx;
    x = _85;
    bool _91 = false;
    if (_82)
    {
        _91 = all(bool4(_85.x == int4(2, 1, 1, 2).x, _85.y == int4(2, 1, 1, 2).y, _85.z == int4(2, 1, 1, 2).z, _85.w == int4(2, 1, 1, 2).w));
    }
    else
    {
        _91 = false;
    }
    ok = _91;
    int3 _96 = _59.wyw * int3(9, 9, 9);
    int4 _97 = x;
    int4 _98 = int4(_96.x, _96.y, _96.z, _97.w);
    x = _98;
    bool _104 = false;
    if (_91)
    {
        _104 = all(bool4(_98.x == int4(9, 9, 9, 2).x, _98.y == int4(9, 9, 9, 2).y, _98.z == int4(9, 9, 9, 2).z, _98.w == int4(9, 9, 9, 2).w));
    }
    else
    {
        _104 = false;
    }
    ok = _104;
    int2 _109 = _98.zw / int2(4, 4);
    int4 _110 = x;
    int4 _111 = int4(_109.x, _109.y, _110.z, _110.w);
    x = _111;
    bool _117 = false;
    if (_104)
    {
        _117 = all(bool4(_111.x == int4(2, 0, 9, 2).x, _111.y == int4(2, 0, 9, 2).y, _111.z == int4(2, 0, 9, 2).z, _111.w == int4(2, 0, 9, 2).w));
    }
    else
    {
        _117 = false;
    }
    ok = _117;
    int4 _121 = (_46 * int4(5, 5, 5, 5)).yxwz;
    x = _121;
    bool _127 = false;
    if (_117)
    {
        _127 = all(bool4(_121.x == int4(0, 5, 5, 0).x, _121.y == int4(0, 5, 5, 0).y, _121.z == int4(0, 5, 5, 0).z, _121.w == int4(0, 5, 5, 0).w));
    }
    else
    {
        _127 = false;
    }
    ok = _127;
    int4 _128 = int4(2, 2, 2, 2) + _46;
    x = _128;
    bool _133 = false;
    if (_127)
    {
        _133 = all(bool4(_128.x == int4(3, 2, 2, 3).x, _128.y == int4(3, 2, 2, 3).y, _128.z == int4(3, 2, 2, 3).z, _128.w == int4(3, 2, 2, 3).w));
    }
    else
    {
        _133 = false;
    }
    ok = _133;
    int4 _137 = int4(10, 10, 10, 10) - _59.ywxz;
    x = _137;
    bool _143 = false;
    if (_133)
    {
        _143 = all(bool4(_137.x == int4(9, 9, 10, 10).x, _137.y == int4(9, 9, 10, 10).y, _137.z == int4(9, 9, 10, 10).z, _137.w == int4(9, 9, 10, 10).w));
    }
    else
    {
        _143 = false;
    }
    ok = _143;
    int4 _146 = _46.x.xxxx + _59;
    x = _146;
    bool _152 = false;
    if (_143)
    {
        _152 = all(bool4(_146.x == int4(1, 2, 1, 2).x, _146.y == int4(1, 2, 1, 2).y, _146.z == int4(1, 2, 1, 2).z, _146.w == int4(1, 2, 1, 2).w));
    }
    else
    {
        _152 = false;
    }
    ok = _152;
    int3 _156 = int3(8, 8, 8) * _59.wyw;
    int4 _157 = x;
    int4 _158 = int4(_156.x, _156.y, _156.z, _157.w);
    x = _158;
    bool _164 = false;
    if (_152)
    {
        _164 = all(bool4(_158.x == int4(8, 8, 8, 2).x, _158.y == int4(8, 8, 8, 2).y, _158.z == int4(8, 8, 8, 2).z, _158.w == int4(8, 8, 8, 2).w));
    }
    else
    {
        _164 = false;
    }
    ok = _164;
    int2 _168 = int2(36, 36) / _158.zw;
    int4 _169 = x;
    int4 _170 = int4(_168.x, _168.y, _169.z, _169.w);
    x = _170;
    bool _177 = false;
    if (_164)
    {
        _177 = all(bool4(_170.x == int4(4, 18, 8, 2).x, _170.y == int4(4, 18, 8, 2).y, _170.z == int4(4, 18, 8, 2).z, _170.w == int4(4, 18, 8, 2).w));
    }
    else
    {
        _177 = false;
    }
    ok = _177;
    int4 _181 = (int4(37, 37, 37, 37) / _170).yxwz;
    x = _181;
    bool _187 = false;
    if (_177)
    {
        _187 = all(bool4(_181.x == int4(2, 9, 18, 4).x, _181.y == int4(2, 9, 18, 4).y, _181.z == int4(2, 9, 18, 4).z, _181.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _187 = false;
    }
    ok = _187;
    int4 _188 = _181 + int4(2, 2, 2, 2);
    x = _188;
    int4 _189 = _188 * int4(2, 2, 2, 2);
    x = _189;
    int4 _191 = _189 - int4(4, 4, 4, 4);
    x = _191;
    int4 _192 = _191 / int4(2, 2, 2, 2);
    x = _192;
    bool _197 = false;
    if (_187)
    {
        _197 = all(bool4(_192.x == int4(2, 9, 18, 4).x, _192.y == int4(2, 9, 18, 4).y, _192.z == int4(2, 9, 18, 4).z, _192.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _197 = false;
    }
    ok = _197;
    int4 _198 = _192 + int4(2, 2, 2, 2);
    x = _198;
    int4 _199 = _198 * int4(2, 2, 2, 2);
    x = _199;
    int4 _200 = _199 - int4(4, 4, 4, 4);
    x = _200;
    int4 _201 = _200 / int4(2, 2, 2, 2);
    x = _201;
    bool _206 = false;
    if (_197)
    {
        _206 = all(bool4(_201.x == int4(2, 9, 18, 4).x, _201.y == int4(2, 9, 18, 4).y, _201.z == int4(2, 9, 18, 4).z, _201.w == int4(2, 9, 18, 4).w));
    }
    else
    {
        _206 = false;
    }
    ok = _206;
    return _206;
}

float4 main(float2 _208)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float4 _RESERVED_IDENTIFIER_FIXUP_1_inputRed = _12_colorRed;
    float4 _RESERVED_IDENTIFIER_FIXUP_2_inputGreen = _12_colorGreen;
    float4 _221 = _12_colorRed + 2.0f.xxxx;
    float4 _RESERVED_IDENTIFIER_FIXUP_3_x = _221;
    bool _228 = false;
    if (true)
    {
        _228 = all(bool4(_221.x == float4(3.0f, 2.0f, 2.0f, 3.0f).x, _221.y == float4(3.0f, 2.0f, 2.0f, 3.0f).y, _221.z == float4(3.0f, 2.0f, 2.0f, 3.0f).z, _221.w == float4(3.0f, 2.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _228 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _228;
    float4 _230 = _12_colorGreen.ywxz - 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _230;
    bool _238 = false;
    if (_228)
    {
        _238 = all(bool4(_230.x == float4(-1.0f, -1.0f, -2.0f, -2.0f).x, _230.y == float4(-1.0f, -1.0f, -2.0f, -2.0f).y, _230.z == float4(-1.0f, -1.0f, -2.0f, -2.0f).z, _230.w == float4(-1.0f, -1.0f, -2.0f, -2.0f).w));
    }
    else
    {
        _238 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _238;
    float4 _241 = _12_colorRed + _12_colorGreen.y.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _241;
    bool _248 = false;
    if (_238)
    {
        _248 = all(bool4(_241.x == float4(2.0f, 1.0f, 1.0f, 2.0f).x, _241.y == float4(2.0f, 1.0f, 1.0f, 2.0f).y, _241.z == float4(2.0f, 1.0f, 1.0f, 2.0f).z, _241.w == float4(2.0f, 1.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _248 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _248;
    float3 _252 = _12_colorGreen.wyw * 9.0f;
    float4 _253 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _254 = float4(_252.x, _252.y, _252.z, _253.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _254;
    bool _260 = false;
    if (_248)
    {
        _260 = all(bool4(_254.x == float4(9.0f, 9.0f, 9.0f, 2.0f).x, _254.y == float4(9.0f, 9.0f, 9.0f, 2.0f).y, _254.z == float4(9.0f, 9.0f, 9.0f, 2.0f).z, _254.w == float4(9.0f, 9.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _260 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _260;
    float2 _262 = _254.zw * 2.0f;
    float4 _263 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _264 = float4(_262.x, _262.y, _263.z, _263.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _264;
    bool _272 = false;
    if (_260)
    {
        _272 = all(bool4(_264.x == float4(18.0f, 4.0f, 9.0f, 2.0f).x, _264.y == float4(18.0f, 4.0f, 9.0f, 2.0f).y, _264.z == float4(18.0f, 4.0f, 9.0f, 2.0f).z, _264.w == float4(18.0f, 4.0f, 9.0f, 2.0f).w));
    }
    else
    {
        _272 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _272;
    float4 _275 = (_12_colorRed * 5.0f).yxwz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _275;
    bool _281 = false;
    if (_272)
    {
        _281 = all(bool4(_275.x == float4(0.0f, 5.0f, 5.0f, 0.0f).x, _275.y == float4(0.0f, 5.0f, 5.0f, 0.0f).y, _275.z == float4(0.0f, 5.0f, 5.0f, 0.0f).z, _275.w == float4(0.0f, 5.0f, 5.0f, 0.0f).w));
    }
    else
    {
        _281 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _281;
    float4 _282 = 2.0f.xxxx + _12_colorRed;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _282;
    bool _287 = false;
    if (_281)
    {
        _287 = all(bool4(_282.x == float4(3.0f, 2.0f, 2.0f, 3.0f).x, _282.y == float4(3.0f, 2.0f, 2.0f, 3.0f).y, _282.z == float4(3.0f, 2.0f, 2.0f, 3.0f).z, _282.w == float4(3.0f, 2.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _287 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _287;
    float4 _291 = 10.0f.xxxx - _12_colorGreen.ywxz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _291;
    bool _297 = false;
    if (_287)
    {
        _297 = all(bool4(_291.x == float4(9.0f, 9.0f, 10.0f, 10.0f).x, _291.y == float4(9.0f, 9.0f, 10.0f, 10.0f).y, _291.z == float4(9.0f, 9.0f, 10.0f, 10.0f).z, _291.w == float4(9.0f, 9.0f, 10.0f, 10.0f).w));
    }
    else
    {
        _297 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _297;
    float4 _300 = _12_colorRed.x.xxxx + _12_colorGreen;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _300;
    bool _306 = false;
    if (_297)
    {
        _306 = all(bool4(_300.x == float4(1.0f, 2.0f, 1.0f, 2.0f).x, _300.y == float4(1.0f, 2.0f, 1.0f, 2.0f).y, _300.z == float4(1.0f, 2.0f, 1.0f, 2.0f).z, _300.w == float4(1.0f, 2.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _306 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _306;
    float3 _309 = _12_colorGreen.wyw * 8.0f;
    float4 _310 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _311 = float4(_309.x, _309.y, _309.z, _310.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _311;
    bool _317 = false;
    if (_306)
    {
        _317 = all(bool4(_311.x == float4(8.0f, 8.0f, 8.0f, 2.0f).x, _311.y == float4(8.0f, 8.0f, 8.0f, 2.0f).y, _311.z == float4(8.0f, 8.0f, 8.0f, 2.0f).z, _311.w == float4(8.0f, 8.0f, 8.0f, 2.0f).w));
    }
    else
    {
        _317 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _317;
    float2 _321 = 32.0f.xx / _311.zw;
    float4 _322 = _RESERVED_IDENTIFIER_FIXUP_3_x;
    float4 _323 = float4(_321.x, _321.y, _322.z, _322.w);
    _RESERVED_IDENTIFIER_FIXUP_3_x = _323;
    bool _330 = false;
    if (_317)
    {
        _330 = all(bool4(_323.x == float4(4.0f, 16.0f, 8.0f, 2.0f).x, _323.y == float4(4.0f, 16.0f, 8.0f, 2.0f).y, _323.z == float4(4.0f, 16.0f, 8.0f, 2.0f).z, _323.w == float4(4.0f, 16.0f, 8.0f, 2.0f).w));
    }
    else
    {
        _330 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _330;
    float4 _333 = (32.0f.xxxx / _323).yxwz;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _333;
    bool _339 = false;
    if (_330)
    {
        _339 = all(bool4(_333.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _333.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _333.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _333.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _339 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _339;
    float4 _340 = _333 + 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _340;
    float4 _341 = _340 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _341;
    float4 _343 = _341 - 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _343;
    float4 _345 = _343 * 0.5f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _345;
    bool _350 = false;
    if (_339)
    {
        _350 = all(bool4(_345.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _345.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _345.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _345.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _350 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _350;
    float4 _351 = _345 + 2.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _351;
    float4 _352 = _351 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _352;
    float4 _353 = _352 - 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _353;
    float4 _354 = _353 * 0.5f;
    _RESERVED_IDENTIFIER_FIXUP_3_x = _354;
    bool _359 = false;
    if (_350)
    {
        _359 = all(bool4(_354.x == float4(2.0f, 8.0f, 16.0f, 4.0f).x, _354.y == float4(2.0f, 8.0f, 16.0f, 4.0f).y, _354.z == float4(2.0f, 8.0f, 16.0f, 4.0f).z, _354.w == float4(2.0f, 8.0f, 16.0f, 4.0f).w));
    }
    else
    {
        _359 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _359;
    bool _363 = false;
    if (_359)
    {
        _363 = test_int_b();
    }
    else
    {
        _363 = false;
    }
    float4 _364 = 0.0f.xxxx;
    if (_363)
    {
        _364 = _12_colorGreen;
    }
    else
    {
        _364 = _12_colorRed;
    }
    return _364;
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
