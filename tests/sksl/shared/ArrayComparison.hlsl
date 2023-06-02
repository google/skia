struct S
{
    int x;
    int y;
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float _10_testArray[5] : packoffset(c2);
    float _10_testArrayNegative[5] : packoffset(c7);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    float _36[5] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    float f1[5] = _36;
    float f2[5] = _36;
    float _40[5] = { 1.0f, 2.0f, 3.0f, -4.0f, 5.0f };
    float f3[5] = _40;
    int3 _52[2] = { int3(1, 2, 3), int3(4, 5, 6) };
    int3 v1[2] = _52;
    int3 v2[2] = _52;
    int3 _57[2] = { int3(1, 2, 3), int3(4, 5, -6) };
    int3 v3[2] = _57;
    float2x2 _72[3] = { float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f)), float2x2(float2(3.0f, 4.0f), float2(5.0f, 6.0f)) };
    float2x2 m1[3] = _72;
    float2x2 m2[3] = _72;
    float2x2 _81[3] = { float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), float2x2(float2(2.0f, 3.0f), float2(4.0f, 5.0f)), float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)) };
    float2x2 m3[3] = _81;
    S _86 = { 1, 2 };
    S _87 = { 3, 4 };
    S _88 = { 5, 6 };
    S _89[3] = { _86, _87, _88 };
    S s1[3] = _89;
    S _92 = { 0, 0 };
    S _93[3] = { _86, _92, _88 };
    S s2[3] = _93;
    S s3[3] = _89;
    bool _108 = false;
    if (true && (true && (true && (true && true))))
    {
        _108 = false || ((4.0f != (-4.0f)) || (false || (false || false)));
    }
    else
    {
        _108 = false;
    }
    bool _135 = false;
    if (_108)
    {
        _135 = (_10_testArray[4] != _10_testArrayNegative[4]) || ((_10_testArray[3] != _10_testArrayNegative[3]) || ((_10_testArray[2] != _10_testArrayNegative[2]) || ((_10_testArray[1] != _10_testArrayNegative[1]) || (_10_testArray[0] != _10_testArrayNegative[0]))));
    }
    else
    {
        _135 = false;
    }
    bool _154 = false;
    if (_135)
    {
        _154 = (_10_testArray[4] == 5.0f) && ((_10_testArray[3] == 4.0f) && ((_10_testArray[2] == 3.0f) && ((_10_testArray[1] == 2.0f) && (_10_testArray[0] == 1.0f))));
    }
    else
    {
        _154 = false;
    }
    bool _173 = false;
    if (_154)
    {
        _173 = (_10_testArray[4] != 5.0f) || ((_10_testArray[3] != (-4.0f)) || ((_10_testArray[2] != 3.0f) || ((_10_testArray[1] != 2.0f) || (_10_testArray[0] != 1.0f))));
    }
    else
    {
        _173 = false;
    }
    bool _192 = false;
    if (_173)
    {
        _192 = (5.0f == _10_testArray[4]) && ((4.0f == _10_testArray[3]) && ((3.0f == _10_testArray[2]) && ((2.0f == _10_testArray[1]) && (1.0f == _10_testArray[0]))));
    }
    else
    {
        _192 = false;
    }
    bool _211 = false;
    if (_192)
    {
        _211 = (5.0f != _10_testArray[4]) || (((-4.0f) != _10_testArray[3]) || ((3.0f != _10_testArray[2]) || ((2.0f != _10_testArray[1]) || (1.0f != _10_testArray[0]))));
    }
    else
    {
        _211 = false;
    }
    bool _215 = false;
    if (_211)
    {
        _215 = true && true;
    }
    else
    {
        _215 = false;
    }
    bool _222 = false;
    if (_215)
    {
        _222 = any(bool3(int3(4, 5, 6).x != int3(4, 5, -6).x, int3(4, 5, 6).y != int3(4, 5, -6).y, int3(4, 5, 6).z != int3(4, 5, -6).z)) || false;
    }
    else
    {
        _222 = false;
    }
    bool _243 = false;
    if (_222)
    {
        _243 = (all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y)) && all(bool2(float2(5.0f, 6.0f).x == float2(5.0f, 6.0f).x, float2(5.0f, 6.0f).y == float2(5.0f, 6.0f).y))) && ((all(bool2(float2(2.0f, 0.0f).x == float2(2.0f, 0.0f).x, float2(2.0f, 0.0f).y == float2(2.0f, 0.0f).y)) && all(bool2(float2(0.0f, 2.0f).x == float2(0.0f, 2.0f).x, float2(0.0f, 2.0f).y == float2(0.0f, 2.0f).y))) && (all(bool2(float2(1.0f, 0.0f).x == float2(1.0f, 0.0f).x, float2(1.0f, 0.0f).y == float2(1.0f, 0.0f).y)) && all(bool2(float2(0.0f, 1.0f).x == float2(0.0f, 1.0f).x, float2(0.0f, 1.0f).y == float2(0.0f, 1.0f).y))));
    }
    else
    {
        _243 = false;
    }
    bool _263 = false;
    if (_243)
    {
        _263 = (any(bool2(float2(3.0f, 4.0f).x != float2(6.0f, 0.0f).x, float2(3.0f, 4.0f).y != float2(6.0f, 0.0f).y)) || any(bool2(float2(5.0f, 6.0f).x != float2(0.0f, 6.0f).x, float2(5.0f, 6.0f).y != float2(0.0f, 6.0f).y))) || ((any(bool2(float2(2.0f, 0.0f).x != float2(2.0f, 3.0f).x, float2(2.0f, 0.0f).y != float2(2.0f, 3.0f).y)) || any(bool2(float2(0.0f, 2.0f).x != float2(4.0f, 5.0f).x, float2(0.0f, 2.0f).y != float2(4.0f, 5.0f).y))) || (any(bool2(float2(1.0f, 0.0f).x != float2(1.0f, 0.0f).x, float2(1.0f, 0.0f).y != float2(1.0f, 0.0f).y)) || any(bool2(float2(0.0f, 1.0f).x != float2(0.0f, 1.0f).x, float2(0.0f, 1.0f).y != float2(0.0f, 1.0f).y))));
    }
    else
    {
        _263 = false;
    }
    bool _273 = false;
    if (_263)
    {
        _273 = (false || false) || (((4 != 0) || (3 != 0)) || (false || false));
    }
    else
    {
        _273 = false;
    }
    bool _281 = false;
    if (_273)
    {
        _281 = (true && true) && ((true && true) && (true && true));
    }
    else
    {
        _281 = false;
    }
    float4 _282 = 0.0f.xxxx;
    if (_281)
    {
        _282 = _10_colorGreen;
    }
    else
    {
        _282 = _10_colorRed;
    }
    return _282;
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
