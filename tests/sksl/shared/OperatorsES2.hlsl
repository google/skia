cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float _10_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float x = 1.0f;
    float y = 2.0f;
    int z = 3;
    x = (x - x) + (((y * x) * x) * (y - x));
    y = (x / y) / x;
    z = (((z / 2) * 3) + 4) - 2;
    bool _82 = false;
    if ((x > 4.0f) == (x < 2.0f))
    {
        _82 = true;
    }
    else
    {
        bool _81 = false;
        if (2.0f >= _10_unknownInput)
        {
            _81 = y <= x;
        }
        else
        {
            _81 = false;
        }
        _82 = _81;
    }
    bool b = _82;
    bool c = _10_unknownInput > 2.0f;
    bool d = b != c;
    bool _96 = false;
    if (b)
    {
        _96 = c;
    }
    else
    {
        _96 = false;
    }
    bool e = _96;
    bool _102 = false;
    if (b)
    {
        _102 = true;
    }
    else
    {
        _102 = c;
    }
    bool f = _102;
    x += 12.0f;
    x -= 12.0f;
    float _109 = y;
    float _111 = _109 / 10.0f;
    y = _111;
    x *= _111;
    x = 6.0f;
    y = (((float(b) * float(c)) * float(d)) * float(e)) * float(f);
    y = 6.0f;
    z--;
    z = 6;
    bool _138 = false;
    if (x == 6.0f)
    {
        _138 = y == 6.0f;
    }
    else
    {
        _138 = false;
    }
    bool _143 = false;
    if (_138)
    {
        _143 = z == 6;
    }
    else
    {
        _143 = false;
    }
    float4 _144 = 0.0f.xxxx;
    if (_143)
    {
        _144 = _10_colorGreen;
    }
    else
    {
        _144 = _10_colorRed;
    }
    return _144;
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
