cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _35 = _11_testInputs * 100.0f;
    int4 _44 = int4(int(_35.x), int(_35.y), int(_35.z), int(_35.w));
    int4 intValues = _44;
    float4 _49 = _11_colorGreen * 100.0f;
    int4 _58 = int4(int(_49.x), int(_49.y), int(_49.z), int(_49.w));
    int4 intGreen = _58;
    int4 expectedA = int4(50, 50, 75, 225);
    int4 expectedB = int4(0, 100, 75, 225);
    int _70 = _44.x;
    bool _82 = false;
    if (max(_70, 50) == 50)
    {
        int2 _74 = max(_44.xy, int2(50, 50));
        _82 = all(bool2(_74.x == int4(50, 50, 75, 225).xy.x, _74.y == int4(50, 50, 75, 225).xy.y));
    }
    else
    {
        _82 = false;
    }
    bool _93 = false;
    if (_82)
    {
        int3 _85 = max(_44.xyz, int3(50, 50, 50));
        _93 = all(bool3(_85.x == int4(50, 50, 75, 225).xyz.x, _85.y == int4(50, 50, 75, 225).xyz.y, _85.z == int4(50, 50, 75, 225).xyz.z));
    }
    else
    {
        _93 = false;
    }
    bool _101 = false;
    if (_93)
    {
        int4 _96 = max(_44, int4(50, 50, 50, 50));
        _101 = all(bool4(_96.x == int4(50, 50, 75, 225).x, _96.y == int4(50, 50, 75, 225).y, _96.z == int4(50, 50, 75, 225).z, _96.w == int4(50, 50, 75, 225).w));
    }
    else
    {
        _101 = false;
    }
    bool _105 = false;
    if (_101)
    {
        _105 = true;
    }
    else
    {
        _105 = false;
    }
    bool _111 = false;
    if (_105)
    {
        _111 = all(bool2(int2(50, 50).x == int4(50, 50, 75, 225).xy.x, int2(50, 50).y == int4(50, 50, 75, 225).xy.y));
    }
    else
    {
        _111 = false;
    }
    bool _118 = false;
    if (_111)
    {
        _118 = all(bool3(int3(50, 50, 75).x == int4(50, 50, 75, 225).xyz.x, int3(50, 50, 75).y == int4(50, 50, 75, 225).xyz.y, int3(50, 50, 75).z == int4(50, 50, 75, 225).xyz.z));
    }
    else
    {
        _118 = false;
    }
    bool _121 = false;
    if (_118)
    {
        _121 = true;
    }
    else
    {
        _121 = false;
    }
    bool _127 = false;
    if (_121)
    {
        _127 = max(_70, _58.x) == 0;
    }
    else
    {
        _127 = false;
    }
    bool _136 = false;
    if (_127)
    {
        int2 _130 = max(_44.xy, _58.xy);
        _136 = all(bool2(_130.x == int4(0, 100, 75, 225).xy.x, _130.y == int4(0, 100, 75, 225).xy.y));
    }
    else
    {
        _136 = false;
    }
    bool _145 = false;
    if (_136)
    {
        int3 _139 = max(_44.xyz, _58.xyz);
        _145 = all(bool3(_139.x == int4(0, 100, 75, 225).xyz.x, _139.y == int4(0, 100, 75, 225).xyz.y, _139.z == int4(0, 100, 75, 225).xyz.z));
    }
    else
    {
        _145 = false;
    }
    bool _151 = false;
    if (_145)
    {
        int4 _148 = max(_44, _58);
        _151 = all(bool4(_148.x == int4(0, 100, 75, 225).x, _148.y == int4(0, 100, 75, 225).y, _148.z == int4(0, 100, 75, 225).z, _148.w == int4(0, 100, 75, 225).w));
    }
    else
    {
        _151 = false;
    }
    bool _154 = false;
    if (_151)
    {
        _154 = true;
    }
    else
    {
        _154 = false;
    }
    bool _161 = false;
    if (_154)
    {
        _161 = all(bool2(int2(0, 100).x == int4(0, 100, 75, 225).xy.x, int2(0, 100).y == int4(0, 100, 75, 225).xy.y));
    }
    else
    {
        _161 = false;
    }
    bool _168 = false;
    if (_161)
    {
        _168 = all(bool3(int3(0, 100, 75).x == int4(0, 100, 75, 225).xyz.x, int3(0, 100, 75).y == int4(0, 100, 75, 225).xyz.y, int3(0, 100, 75).z == int4(0, 100, 75, 225).xyz.z));
    }
    else
    {
        _168 = false;
    }
    bool _171 = false;
    if (_168)
    {
        _171 = true;
    }
    else
    {
        _171 = false;
    }
    float4 _172 = 0.0f.xxxx;
    if (_171)
    {
        _172 = _11_colorGreen;
    }
    else
    {
        _172 = _11_colorRed;
    }
    return _172;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
