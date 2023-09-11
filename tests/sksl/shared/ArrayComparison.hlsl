struct S
{
    int x;
    int y;
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float _7_testArray[5] : packoffset(c2);
    float _7_testArrayNegative[5] : packoffset(c7);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float _33[5] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    float f1[5] = _33;
    float f2[5] = _33;
    float _37[5] = { 1.0f, 2.0f, 3.0f, -4.0f, 5.0f };
    float f3[5] = _37;
    int3 _49[2] = { int3(1, 2, 3), int3(4, 5, 6) };
    int3 v1[2] = _49;
    int3 v2[2] = _49;
    int3 _54[2] = { int3(1, 2, 3), int3(4, 5, -6) };
    int3 v3[2] = _54;
    float2x2 _69[3] = { float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f)), float2x2(float2(3.0f, 4.0f), float2(5.0f, 6.0f)) };
    float2x2 m1[3] = _69;
    float2x2 m2[3] = _69;
    float2x2 _78[3] = { float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), float2x2(float2(2.0f, 3.0f), float2(4.0f, 5.0f)), float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)) };
    float2x2 m3[3] = _78;
    S _83 = { 1, 2 };
    S _84 = { 3, 4 };
    S _85 = { 5, 6 };
    S _86[3] = { _83, _84, _85 };
    S s1[3] = _86;
    S _89 = { 0, 0 };
    S _90[3] = { _83, _89, _85 };
    S s2[3] = _90;
    S s3[3] = _86;
    bool _106 = false;
    if (true && (true && (true && (true && true))))
    {
        _106 = false || ((4.0f != (-4.0f)) || (false || (false || false)));
    }
    else
    {
        _106 = false;
    }
    bool _133 = false;
    if (_106)
    {
        _133 = (_7_testArray[4] != _7_testArrayNegative[4]) || ((_7_testArray[3] != _7_testArrayNegative[3]) || ((_7_testArray[2] != _7_testArrayNegative[2]) || ((_7_testArray[1] != _7_testArrayNegative[1]) || (_7_testArray[0] != _7_testArrayNegative[0]))));
    }
    else
    {
        _133 = false;
    }
    bool _152 = false;
    if (_133)
    {
        _152 = (_7_testArray[4] == 5.0f) && ((_7_testArray[3] == 4.0f) && ((_7_testArray[2] == 3.0f) && ((_7_testArray[1] == 2.0f) && (_7_testArray[0] == 1.0f))));
    }
    else
    {
        _152 = false;
    }
    bool _171 = false;
    if (_152)
    {
        _171 = (_7_testArray[4] != 5.0f) || ((_7_testArray[3] != (-4.0f)) || ((_7_testArray[2] != 3.0f) || ((_7_testArray[1] != 2.0f) || (_7_testArray[0] != 1.0f))));
    }
    else
    {
        _171 = false;
    }
    bool _190 = false;
    if (_171)
    {
        _190 = (5.0f == _7_testArray[4]) && ((4.0f == _7_testArray[3]) && ((3.0f == _7_testArray[2]) && ((2.0f == _7_testArray[1]) && (1.0f == _7_testArray[0]))));
    }
    else
    {
        _190 = false;
    }
    bool _209 = false;
    if (_190)
    {
        _209 = (5.0f != _7_testArray[4]) || (((-4.0f) != _7_testArray[3]) || ((3.0f != _7_testArray[2]) || ((2.0f != _7_testArray[1]) || (1.0f != _7_testArray[0]))));
    }
    else
    {
        _209 = false;
    }
    bool _213 = false;
    if (_209)
    {
        _213 = true && true;
    }
    else
    {
        _213 = false;
    }
    bool _220 = false;
    if (_213)
    {
        _220 = any(bool3(int3(4, 5, 6).x != int3(4, 5, -6).x, int3(4, 5, 6).y != int3(4, 5, -6).y, int3(4, 5, 6).z != int3(4, 5, -6).z)) || false;
    }
    else
    {
        _220 = false;
    }
    bool _241 = false;
    if (_220)
    {
        _241 = (all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y)) && all(bool2(float2(5.0f, 6.0f).x == float2(5.0f, 6.0f).x, float2(5.0f, 6.0f).y == float2(5.0f, 6.0f).y))) && ((all(bool2(float2(2.0f, 0.0f).x == float2(2.0f, 0.0f).x, float2(2.0f, 0.0f).y == float2(2.0f, 0.0f).y)) && all(bool2(float2(0.0f, 2.0f).x == float2(0.0f, 2.0f).x, float2(0.0f, 2.0f).y == float2(0.0f, 2.0f).y))) && (all(bool2(float2(1.0f, 0.0f).x == float2(1.0f, 0.0f).x, float2(1.0f, 0.0f).y == float2(1.0f, 0.0f).y)) && all(bool2(float2(0.0f, 1.0f).x == float2(0.0f, 1.0f).x, float2(0.0f, 1.0f).y == float2(0.0f, 1.0f).y))));
    }
    else
    {
        _241 = false;
    }
    bool _261 = false;
    if (_241)
    {
        _261 = (any(bool2(float2(3.0f, 4.0f).x != float2(6.0f, 0.0f).x, float2(3.0f, 4.0f).y != float2(6.0f, 0.0f).y)) || any(bool2(float2(5.0f, 6.0f).x != float2(0.0f, 6.0f).x, float2(5.0f, 6.0f).y != float2(0.0f, 6.0f).y))) || ((any(bool2(float2(2.0f, 0.0f).x != float2(2.0f, 3.0f).x, float2(2.0f, 0.0f).y != float2(2.0f, 3.0f).y)) || any(bool2(float2(0.0f, 2.0f).x != float2(4.0f, 5.0f).x, float2(0.0f, 2.0f).y != float2(4.0f, 5.0f).y))) || (any(bool2(float2(1.0f, 0.0f).x != float2(1.0f, 0.0f).x, float2(1.0f, 0.0f).y != float2(1.0f, 0.0f).y)) || any(bool2(float2(0.0f, 1.0f).x != float2(0.0f, 1.0f).x, float2(0.0f, 1.0f).y != float2(0.0f, 1.0f).y))));
    }
    else
    {
        _261 = false;
    }
    bool _271 = false;
    if (_261)
    {
        _271 = (false || false) || (((4 != 0) || (3 != 0)) || (false || false));
    }
    else
    {
        _271 = false;
    }
    bool _279 = false;
    if (_271)
    {
        _279 = (true && true) && ((true && true) && (true && true));
    }
    else
    {
        _279 = false;
    }
    float4 _280 = 0.0f.xxxx;
    if (_279)
    {
        _280 = _7_colorGreen;
    }
    else
    {
        _280 = _7_colorRed;
    }
    return _280;
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
