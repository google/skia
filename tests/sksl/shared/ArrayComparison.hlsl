struct S
{
    int x;
    int y;
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    float _11_testArray[5] : packoffset(c2);
    float _11_testArrayNegative[5] : packoffset(c7);
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
    bool _109 = false;
    if (true && (true && (true && (true && true))))
    {
        _109 = false || ((4.0f != (-4.0f)) || (false || (false || false)));
    }
    else
    {
        _109 = false;
    }
    bool _136 = false;
    if (_109)
    {
        _136 = (_11_testArray[4] != _11_testArrayNegative[4]) || ((_11_testArray[3] != _11_testArrayNegative[3]) || ((_11_testArray[2] != _11_testArrayNegative[2]) || ((_11_testArray[1] != _11_testArrayNegative[1]) || (_11_testArray[0] != _11_testArrayNegative[0]))));
    }
    else
    {
        _136 = false;
    }
    bool _155 = false;
    if (_136)
    {
        _155 = (_11_testArray[4] == 5.0f) && ((_11_testArray[3] == 4.0f) && ((_11_testArray[2] == 3.0f) && ((_11_testArray[1] == 2.0f) && (_11_testArray[0] == 1.0f))));
    }
    else
    {
        _155 = false;
    }
    bool _174 = false;
    if (_155)
    {
        _174 = (_11_testArray[4] != 5.0f) || ((_11_testArray[3] != (-4.0f)) || ((_11_testArray[2] != 3.0f) || ((_11_testArray[1] != 2.0f) || (_11_testArray[0] != 1.0f))));
    }
    else
    {
        _174 = false;
    }
    bool _193 = false;
    if (_174)
    {
        _193 = (5.0f == _11_testArray[4]) && ((4.0f == _11_testArray[3]) && ((3.0f == _11_testArray[2]) && ((2.0f == _11_testArray[1]) && (1.0f == _11_testArray[0]))));
    }
    else
    {
        _193 = false;
    }
    bool _212 = false;
    if (_193)
    {
        _212 = (5.0f != _11_testArray[4]) || (((-4.0f) != _11_testArray[3]) || ((3.0f != _11_testArray[2]) || ((2.0f != _11_testArray[1]) || (1.0f != _11_testArray[0]))));
    }
    else
    {
        _212 = false;
    }
    bool _216 = false;
    if (_212)
    {
        _216 = true && true;
    }
    else
    {
        _216 = false;
    }
    bool _223 = false;
    if (_216)
    {
        _223 = any(bool3(int3(4, 5, 6).x != int3(4, 5, -6).x, int3(4, 5, 6).y != int3(4, 5, -6).y, int3(4, 5, 6).z != int3(4, 5, -6).z)) || false;
    }
    else
    {
        _223 = false;
    }
    bool _244 = false;
    if (_223)
    {
        _244 = (all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y)) && all(bool2(float2(5.0f, 6.0f).x == float2(5.0f, 6.0f).x, float2(5.0f, 6.0f).y == float2(5.0f, 6.0f).y))) && ((all(bool2(float2(2.0f, 0.0f).x == float2(2.0f, 0.0f).x, float2(2.0f, 0.0f).y == float2(2.0f, 0.0f).y)) && all(bool2(float2(0.0f, 2.0f).x == float2(0.0f, 2.0f).x, float2(0.0f, 2.0f).y == float2(0.0f, 2.0f).y))) && (all(bool2(float2(1.0f, 0.0f).x == float2(1.0f, 0.0f).x, float2(1.0f, 0.0f).y == float2(1.0f, 0.0f).y)) && all(bool2(float2(0.0f, 1.0f).x == float2(0.0f, 1.0f).x, float2(0.0f, 1.0f).y == float2(0.0f, 1.0f).y))));
    }
    else
    {
        _244 = false;
    }
    bool _264 = false;
    if (_244)
    {
        _264 = (any(bool2(float2(3.0f, 4.0f).x != float2(6.0f, 0.0f).x, float2(3.0f, 4.0f).y != float2(6.0f, 0.0f).y)) || any(bool2(float2(5.0f, 6.0f).x != float2(0.0f, 6.0f).x, float2(5.0f, 6.0f).y != float2(0.0f, 6.0f).y))) || ((any(bool2(float2(2.0f, 0.0f).x != float2(2.0f, 3.0f).x, float2(2.0f, 0.0f).y != float2(2.0f, 3.0f).y)) || any(bool2(float2(0.0f, 2.0f).x != float2(4.0f, 5.0f).x, float2(0.0f, 2.0f).y != float2(4.0f, 5.0f).y))) || (any(bool2(float2(1.0f, 0.0f).x != float2(1.0f, 0.0f).x, float2(1.0f, 0.0f).y != float2(1.0f, 0.0f).y)) || any(bool2(float2(0.0f, 1.0f).x != float2(0.0f, 1.0f).x, float2(0.0f, 1.0f).y != float2(0.0f, 1.0f).y))));
    }
    else
    {
        _264 = false;
    }
    bool _274 = false;
    if (_264)
    {
        _274 = (false || false) || (((4 != 0) || (3 != 0)) || (false || false));
    }
    else
    {
        _274 = false;
    }
    bool _282 = false;
    if (_274)
    {
        _282 = (true && true) && ((true && true) && (true && true));
    }
    else
    {
        _282 = false;
    }
    float4 _283 = 0.0f.xxxx;
    if (_282)
    {
        _283 = _11_colorGreen;
    }
    else
    {
        _283 = _11_colorRed;
    }
    return _283;
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
