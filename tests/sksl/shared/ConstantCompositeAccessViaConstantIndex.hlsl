cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _20_colorRed : packoffset(c0);
    row_major float2x2 _20_testMatrix2x2 : packoffset(c1);
    float _20_testArray[5] : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float globalArray[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
static float2x2 globalMatrix = float2x2(0.0f.xx, 0.0f.xx);

float4 main(float2 _33)
{
    float _13[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    globalArray = _13;
    globalMatrix = float2x2(1.0f.xx, 1.0f.xx);
    float _40[5] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f };
    float localArray[5] = _40;
    float2x2 localMatrix = float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f));
    bool _76 = false;
    if ((1.0f == _20_testArray[4]) && ((1.0f == _20_testArray[3]) && ((1.0f == _20_testArray[2]) && ((1.0f == _20_testArray[1]) && (1.0f == _20_testArray[0])))))
    {
        _76 = true;
    }
    else
    {
        _76 = all(bool2(1.0f.xx.x == _20_colorRed.xy.x, 1.0f.xx.y == _20_colorRed.xy.y));
    }
    bool _90 = false;
    if (_76)
    {
        _90 = true;
    }
    else
    {
        _90 = all(bool2(1.0f.xx.x == _20_testMatrix2x2[0].x, 1.0f.xx.y == _20_testMatrix2x2[0].y)) && all(bool2(1.0f.xx.x == _20_testMatrix2x2[1].x, 1.0f.xx.y == _20_testMatrix2x2[1].y));
    }
    bool _109 = false;
    if (_90)
    {
        _109 = true;
    }
    else
    {
        _109 = (4.0f == _20_testArray[4]) && ((3.0f == _20_testArray[3]) && ((2.0f == _20_testArray[2]) && ((1.0f == _20_testArray[1]) && (0.0f == _20_testArray[0]))));
    }
    bool _117 = false;
    if (_109)
    {
        _117 = true;
    }
    else
    {
        _117 = all(bool2(1.0f.xx.x == _20_colorRed.xy.x, 1.0f.xx.y == _20_colorRed.xy.y));
    }
    bool _129 = false;
    if (_117)
    {
        _129 = true;
    }
    else
    {
        _129 = all(bool2(float2(0.0f, 1.0f).x == _20_testMatrix2x2[0].x, float2(0.0f, 1.0f).y == _20_testMatrix2x2[0].y)) && all(bool2(float2(2.0f, 3.0f).x == _20_testMatrix2x2[1].x, float2(2.0f, 3.0f).y == _20_testMatrix2x2[1].y));
    }
    if (_129)
    {
        return _20_colorRed;
    }
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}

void frag_main()
{
    float2 _29 = 0.0f.xx;
    float4 _31 = main(_29);
    sk_FragColor = _31;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
