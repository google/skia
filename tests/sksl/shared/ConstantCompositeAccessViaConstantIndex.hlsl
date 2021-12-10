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
    bool _78 = false;
    if ((1.0f == _23_testArray[4]) && ((1.0f == _23_testArray[3]) && ((1.0f == _23_testArray[2]) && ((1.0f == _23_testArray[1]) && (1.0f == _23_testArray[0])))))
    {
        _78 = true;
    }
    else
    {
        _78 = all(bool2(1.0f.xx.x == _23_colorRed.xy.x, 1.0f.xx.y == _23_colorRed.xy.y));
    }
    bool _92 = false;
    if (_78)
    {
        _92 = true;
    }
    else
    {
        _92 = all(bool2(1.0f.xx.x == _23_testMatrix2x2[0].x, 1.0f.xx.y == _23_testMatrix2x2[0].y)) && all(bool2(1.0f.xx.x == _23_testMatrix2x2[1].x, 1.0f.xx.y == _23_testMatrix2x2[1].y));
    }
    bool _111 = false;
    if (_92)
    {
        _111 = true;
    }
    else
    {
        _111 = (4.0f == _23_testArray[4]) && ((3.0f == _23_testArray[3]) && ((2.0f == _23_testArray[2]) && ((1.0f == _23_testArray[1]) && (0.0f == _23_testArray[0]))));
    }
    bool _119 = false;
    if (_111)
    {
        _119 = true;
    }
    else
    {
        _119 = all(bool2(1.0f.xx.x == _23_colorRed.xy.x, 1.0f.xx.y == _23_colorRed.xy.y));
    }
    bool _131 = false;
    if (_119)
    {
        _131 = true;
    }
    else
    {
        _131 = all(bool2(float2(0.0f, 1.0f).x == _23_testMatrix2x2[0].x, float2(0.0f, 1.0f).y == _23_testMatrix2x2[0].y)) && all(bool2(float2(2.0f, 3.0f).x == _23_testMatrix2x2[1].x, float2(2.0f, 3.0f).y == _23_testMatrix2x2[1].y));
    }
    if (_131)
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
