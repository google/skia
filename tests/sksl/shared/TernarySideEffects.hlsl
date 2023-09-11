cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float x = 1.0f;
    float y = 1.0f;
    float _29 = 0.0f;
    if (true)
    {
        float _33 = 1.0f + 1.0f;
        x = _33;
        _29 = _33;
    }
    else
    {
        float _34 = 1.0f + 1.0f;
        y = _34;
        _29 = _34;
    }
    float _39 = 0.0f;
    if (x == y)
    {
        float _43 = x;
        float _45 = _43 + 3.0f;
        x = _45;
        _39 = _45;
    }
    else
    {
        float _46 = y;
        float _47 = _46 + 3.0f;
        y = _47;
        _39 = _47;
    }
    float _52 = 0.0f;
    if (x < y)
    {
        float _56 = x;
        float _58 = _56 + 5.0f;
        x = _58;
        _52 = _58;
    }
    else
    {
        float _59 = y;
        float _60 = _59 + 5.0f;
        y = _60;
        _52 = _60;
    }
    float _65 = 0.0f;
    if (y >= x)
    {
        float _69 = x;
        float _71 = _69 + 9.0f;
        x = _71;
        _65 = _71;
    }
    else
    {
        float _72 = y;
        float _73 = _72 + 9.0f;
        y = _73;
        _65 = _73;
    }
    float _78 = 0.0f;
    if (x != y)
    {
        float _82 = x;
        float _83 = _82 + 1.0f;
        x = _83;
        _78 = _83;
    }
    else
    {
        _78 = y;
    }
    float _89 = 0.0f;
    if (x == y)
    {
        float _93 = x;
        float _95 = _93 + 2.0f;
        x = _95;
        _89 = _95;
    }
    else
    {
        _89 = y;
    }
    float _101 = 0.0f;
    if (x != y)
    {
        _101 = x;
    }
    else
    {
        float _106 = y;
        float _107 = _106 + 3.0f;
        y = _107;
        _101 = _107;
    }
    float _112 = 0.0f;
    if (x == y)
    {
        _112 = x;
    }
    else
    {
        float _117 = y;
        float _119 = _117 + 4.0f;
        y = _119;
        _112 = _119;
    }
    bool b = true;
    b = false;
    bool _125 = false;
    if (false)
    {
        _125 = false;
    }
    else
    {
        _125 = false;
    }
    bool c = _125;
    float4 _130 = 0.0f.xxxx;
    if (_125)
    {
        _130 = _7_colorRed;
    }
    else
    {
        bool _148 = false;
        if (x == 8.0f)
        {
            _148 = y == 17.0f;
        }
        else
        {
            _148 = false;
        }
        float4 _149 = 0.0f.xxxx;
        if (_148)
        {
            _149 = _7_colorGreen;
        }
        else
        {
            _149 = _7_colorRed;
        }
        _130 = _149;
    }
    return _130;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
