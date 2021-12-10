struct S
{
    int x;
    int y;
    row_major float2x2 m;
    float a[5];
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float _10_testArray[5] : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    float _36[5] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    float array[5] = _36;
    S _46 = { 1, 2, float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), _36 };
    S s1 = _46;
    S _51 = { 1, 2, float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), _10_testArray };
    S s2 = _51;
    S _56 = { 1, 2, float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f)), _36 };
    S s3 = _56;
    bool _96 = false;
    if (((5.0f == _10_testArray[4]) && ((4.0f == _10_testArray[3]) && ((3.0f == _10_testArray[2]) && ((2.0f == _10_testArray[1]) && (1.0f == _10_testArray[0]))))) && ((all(bool2(float2(1.0f, 0.0f).x == float2(1.0f, 0.0f).x, float2(1.0f, 0.0f).y == float2(1.0f, 0.0f).y)) && all(bool2(float2(0.0f, 1.0f).x == float2(0.0f, 1.0f).x, float2(0.0f, 1.0f).y == float2(0.0f, 1.0f).y))) && (true && true)))
    {
        _96 = (false || (false || (false || (false || false)))) || ((any(bool2(float2(1.0f, 0.0f).x != float2(2.0f, 0.0f).x, float2(1.0f, 0.0f).y != float2(2.0f, 0.0f).y)) || any(bool2(float2(0.0f, 1.0f).x != float2(0.0f, 2.0f).x, float2(0.0f, 1.0f).y != float2(0.0f, 2.0f).y))) || (false || false));
    }
    else
    {
        _96 = false;
    }
    float4 _97 = 0.0f.xxxx;
    if (_96)
    {
        _97 = _10_colorGreen;
    }
    else
    {
        _97 = _10_colorRed;
    }
    return _97;
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
