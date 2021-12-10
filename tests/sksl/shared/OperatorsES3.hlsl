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
    z = ((((z / 2) % 3) << 4) >> 2) << 1;
    x += 12.0f;
    x -= 12.0f;
    float _68 = y;
    float _70 = _68 / 10.0f;
    y = _70;
    x *= _70;
    z |= 0;
    z &= (-1);
    z ^= 0;
    z = z >> 2;
    z = z << 4;
    z %= 5;
    x = 6.0f;
    y = 6.0f;
    z = 6;
    int2 w = (~5).xx;
    w = ~w;
    bool _105 = false;
    if (w.x == 5)
    {
        _105 = w.y == 5;
    }
    else
    {
        _105 = false;
    }
    bool _110 = false;
    if (_105)
    {
        _110 = x == 6.0f;
    }
    else
    {
        _110 = false;
    }
    bool _115 = false;
    if (_110)
    {
        _115 = y == 6.0f;
    }
    else
    {
        _115 = false;
    }
    bool _120 = false;
    if (_115)
    {
        _120 = z == 6;
    }
    else
    {
        _120 = false;
    }
    float4 _121 = 0.0f.xxxx;
    if (_120)
    {
        _121 = _10_colorGreen;
    }
    else
    {
        _121 = _10_colorRed;
    }
    return _121;
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
