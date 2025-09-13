cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _23_colorRed : packoffset(c0);
    row_major float2x2 _23_testMatrix2x2 : packoffset(c1);
    float _23_testArray[5] : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float globalArray[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
static float2x2 globalMatrix = float2x2(0.0f.xx, 0.0f.xx);

float4 main(float2 _36)
{
    float _16[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    globalArray = _16;
    globalMatrix = float2x2(1.0f.xx, 1.0f.xx);
    float _43[5] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f };
    float localArray[5] = _43;
    float2x2 localMatrix = float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f));
    bool _79 = false;
    if ((1.0f == _23_testArray[4]) && ((1.0f == _23_testArray[3]) && ((1.0f == _23_testArray[2]) && ((1.0f == _23_testArray[1]) && (1.0f == _23_testArray[0])))))
    {
        _79 = true;
    }
    else
    {
        _79 = all(bool2(1.0f.xx.x == _23_colorRed.xy.x, 1.0f.xx.y == _23_colorRed.xy.y));
    }
    bool _93 = false;
    if (_79)
    {
        _93 = true;
    }
    else
    {
        _93 = all(bool2(1.0f.xx.x == _23_testMatrix2x2[0].x, 1.0f.xx.y == _23_testMatrix2x2[0].y)) && all(bool2(1.0f.xx.x == _23_testMatrix2x2[1].x, 1.0f.xx.y == _23_testMatrix2x2[1].y));
    }
    bool _112 = false;
    if (_93)
    {
        _112 = true;
    }
    else
    {
        _112 = (4.0f == _23_testArray[4]) && ((3.0f == _23_testArray[3]) && ((2.0f == _23_testArray[2]) && ((1.0f == _23_testArray[1]) && (0.0f == _23_testArray[0]))));
    }
    bool _120 = false;
    if (_112)
    {
        _120 = true;
    }
    else
    {
        _120 = all(bool2(1.0f.xx.x == _23_colorRed.xy.x, 1.0f.xx.y == _23_colorRed.xy.y));
    }
    bool _132 = false;
    if (_120)
    {
        _132 = true;
    }
    else
    {
        _132 = all(bool2(float2(0.0f, 1.0f).x == _23_testMatrix2x2[0].x, float2(0.0f, 1.0f).y == _23_testMatrix2x2[0].y)) && all(bool2(float2(2.0f, 3.0f).x == _23_testMatrix2x2[1].x, float2(2.0f, 3.0f).y == _23_testMatrix2x2[1].y));
    }
    if (_132)
    {
        return _23_colorRed;
    }
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}

void frag_main()
{
    float2 _32 = 0.0f.xx;
    float4 _34 = main(_32);
    sk_FragColor = _34;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
