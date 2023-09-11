struct S
{
    int x;
    int y;
    row_major float2x2 m;
    float a[5];
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float _7_testArray[5] : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float _33[5] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    float array[5] = _33;
    S _43 = { 1, 2, float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), _33 };
    S s1 = _43;
    S _48 = { 1, 2, float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), _7_testArray };
    S s2 = _48;
    S _53 = { 1, 2, float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f)), _33 };
    S s3 = _53;
    bool _94 = false;
    if (((5.0f == _7_testArray[4]) && ((4.0f == _7_testArray[3]) && ((3.0f == _7_testArray[2]) && ((2.0f == _7_testArray[1]) && (1.0f == _7_testArray[0]))))) && ((all(bool2(float2(1.0f, 0.0f).x == float2(1.0f, 0.0f).x, float2(1.0f, 0.0f).y == float2(1.0f, 0.0f).y)) && all(bool2(float2(0.0f, 1.0f).x == float2(0.0f, 1.0f).x, float2(0.0f, 1.0f).y == float2(0.0f, 1.0f).y))) && (true && true)))
    {
        _94 = (false || (false || (false || (false || false)))) || ((any(bool2(float2(1.0f, 0.0f).x != float2(2.0f, 0.0f).x, float2(1.0f, 0.0f).y != float2(2.0f, 0.0f).y)) || any(bool2(float2(0.0f, 1.0f).x != float2(0.0f, 2.0f).x, float2(0.0f, 1.0f).y != float2(0.0f, 2.0f).y))) || (false || false));
    }
    else
    {
        _94 = false;
    }
    float4 _95 = 0.0f.xxxx;
    if (_94)
    {
        _95 = _7_colorGreen;
    }
    else
    {
        _95 = _7_colorRed;
    }
    return _95;
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
