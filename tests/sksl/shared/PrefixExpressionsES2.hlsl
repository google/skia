cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    bool ok = true;
    int i = 5;
    int _33 = 5 + 1;
    i = _33;
    bool _39 = false;
    if (true)
    {
        _39 = _33 == 6;
    }
    else
    {
        _39 = false;
    }
    ok = _39;
    bool _45 = false;
    if (_39)
    {
        int _42 = _33 + 1;
        i = _42;
        _45 = _42 == 7;
    }
    else
    {
        _45 = false;
    }
    ok = _45;
    bool _51 = false;
    if (_45)
    {
        int _48 = i;
        int _49 = _48 - 1;
        i = _49;
        _51 = _49 == 6;
    }
    else
    {
        _51 = false;
    }
    ok = _51;
    int _52 = i;
    int _53 = _52 - 1;
    i = _53;
    bool _57 = false;
    if (_51)
    {
        _57 = _53 == 5;
    }
    else
    {
        _57 = false;
    }
    ok = _57;
    float f = 0.5f;
    float _62 = 0.5f + 1.0f;
    f = _62;
    bool _67 = false;
    if (_57)
    {
        _67 = _62 == 1.5f;
    }
    else
    {
        _67 = false;
    }
    ok = _67;
    bool _73 = false;
    if (_67)
    {
        float _70 = _62 + 1.0f;
        f = _70;
        _73 = _70 == 2.5f;
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    bool _79 = false;
    if (_73)
    {
        float _76 = f;
        float _77 = _76 - 1.0f;
        f = _77;
        _79 = _77 == 1.5f;
    }
    else
    {
        _79 = false;
    }
    ok = _79;
    float _80 = f;
    float _81 = _80 - 1.0f;
    f = _81;
    bool _85 = false;
    if (_79)
    {
        _85 = _81 == 0.5f;
    }
    else
    {
        _85 = false;
    }
    ok = _85;
    float2 f2 = 0.5f.xx;
    f2.x += 1.0f;
    bool _98 = false;
    if (ok)
    {
        _98 = f2.x == 1.5f;
    }
    else
    {
        _98 = false;
    }
    ok = _98;
    bool _105 = false;
    if (_98)
    {
        float _102 = f2.x;
        float _103 = _102 + 1.0f;
        f2.x = _103;
        _105 = _103 == 2.5f;
    }
    else
    {
        _105 = false;
    }
    ok = _105;
    bool _112 = false;
    if (_105)
    {
        float _109 = f2.x;
        float _110 = _109 - 1.0f;
        f2.x = _110;
        _112 = _110 == 1.5f;
    }
    else
    {
        _112 = false;
    }
    ok = _112;
    f2.x -= 1.0f;
    bool _122 = false;
    if (ok)
    {
        _122 = f2.x == 0.5f;
    }
    else
    {
        _122 = false;
    }
    ok = _122;
    bool _130 = false;
    if (_122)
    {
        _130 = _7_colorGreen.x != 1.0f;
    }
    else
    {
        _130 = false;
    }
    ok = _130;
    bool _139 = false;
    if (_130)
    {
        _139 = (-1.0f) == (-_7_colorGreen.y);
    }
    else
    {
        _139 = false;
    }
    ok = _139;
    bool _149 = false;
    if (_139)
    {
        float4 _145 = -_7_colorGreen;
        _149 = all(bool4(float4(0.0f, -1.0f, 0.0f, -1.0f).x == _145.x, float4(0.0f, -1.0f, 0.0f, -1.0f).y == _145.y, float4(0.0f, -1.0f, 0.0f, -1.0f).z == _145.z, float4(0.0f, -1.0f, 0.0f, -1.0f).w == _145.w));
    }
    else
    {
        _149 = false;
    }
    ok = _149;
    bool _173 = false;
    if (_149)
    {
        float2 _163 = -_7_testMatrix2x2[0];
        float2 _165 = -_7_testMatrix2x2[1];
        _173 = all(bool2(float2(-1.0f, -2.0f).x == _163.x, float2(-1.0f, -2.0f).y == _163.y)) && all(bool2(float2(-3.0f, -4.0f).x == _165.x, float2(-3.0f, -4.0f).y == _165.y));
    }
    else
    {
        _173 = false;
    }
    ok = _173;
    int2 _180 = int2(i, -i);
    int2 iv = _180;
    bool _187 = false;
    if (_173)
    {
        _187 = (-i) == (-5);
    }
    else
    {
        _187 = false;
    }
    ok = _187;
    bool _194 = false;
    if (_187)
    {
        int2 _190 = -_180;
        _194 = all(bool2(_190.x == int2(-5, 5).x, _190.y == int2(-5, 5).y));
    }
    else
    {
        _194 = false;
    }
    ok = _194;
    float4 _195 = 0.0f.xxxx;
    if (_194)
    {
        _195 = _7_colorGreen;
    }
    else
    {
        _195 = _7_colorRed;
    }
    return _195;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
