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
    int4 expectedA = int4(-125, 0, 50, 50);
    int4 expectedB = int4(-125, 0, 0, 100);
    int _69 = _44.x;
    bool _81 = false;
    if (min(_69, 50) == (-125))
    {
        int2 _73 = min(_44.xy, int2(50, 50));
        _81 = all(bool2(_73.x == int4(-125, 0, 50, 50).xy.x, _73.y == int4(-125, 0, 50, 50).xy.y));
    }
    else
    {
        _81 = false;
    }
    bool _92 = false;
    if (_81)
    {
        int3 _84 = min(_44.xyz, int3(50, 50, 50));
        _92 = all(bool3(_84.x == int4(-125, 0, 50, 50).xyz.x, _84.y == int4(-125, 0, 50, 50).xyz.y, _84.z == int4(-125, 0, 50, 50).xyz.z));
    }
    else
    {
        _92 = false;
    }
    bool _100 = false;
    if (_92)
    {
        int4 _95 = min(_44, int4(50, 50, 50, 50));
        _100 = all(bool4(_95.x == int4(-125, 0, 50, 50).x, _95.y == int4(-125, 0, 50, 50).y, _95.z == int4(-125, 0, 50, 50).z, _95.w == int4(-125, 0, 50, 50).w));
    }
    else
    {
        _100 = false;
    }
    bool _104 = false;
    if (_100)
    {
        _104 = true;
    }
    else
    {
        _104 = false;
    }
    bool _111 = false;
    if (_104)
    {
        _111 = all(bool2(int2(-125, 0).x == int4(-125, 0, 50, 50).xy.x, int2(-125, 0).y == int4(-125, 0, 50, 50).xy.y));
    }
    else
    {
        _111 = false;
    }
    bool _118 = false;
    if (_111)
    {
        _118 = all(bool3(int3(-125, 0, 50).x == int4(-125, 0, 50, 50).xyz.x, int3(-125, 0, 50).y == int4(-125, 0, 50, 50).xyz.y, int3(-125, 0, 50).z == int4(-125, 0, 50, 50).xyz.z));
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
        _127 = min(_69, _58.x) == (-125);
    }
    else
    {
        _127 = false;
    }
    bool _136 = false;
    if (_127)
    {
        int2 _130 = min(_44.xy, _58.xy);
        _136 = all(bool2(_130.x == int4(-125, 0, 0, 100).xy.x, _130.y == int4(-125, 0, 0, 100).xy.y));
    }
    else
    {
        _136 = false;
    }
    bool _145 = false;
    if (_136)
    {
        int3 _139 = min(_44.xyz, _58.xyz);
        _145 = all(bool3(_139.x == int4(-125, 0, 0, 100).xyz.x, _139.y == int4(-125, 0, 0, 100).xyz.y, _139.z == int4(-125, 0, 0, 100).xyz.z));
    }
    else
    {
        _145 = false;
    }
    bool _151 = false;
    if (_145)
    {
        int4 _148 = min(_44, _58);
        _151 = all(bool4(_148.x == int4(-125, 0, 0, 100).x, _148.y == int4(-125, 0, 0, 100).y, _148.z == int4(-125, 0, 0, 100).z, _148.w == int4(-125, 0, 0, 100).w));
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
    bool _160 = false;
    if (_154)
    {
        _160 = all(bool2(int2(-125, 0).x == int4(-125, 0, 0, 100).xy.x, int2(-125, 0).y == int4(-125, 0, 0, 100).xy.y));
    }
    else
    {
        _160 = false;
    }
    bool _167 = false;
    if (_160)
    {
        _167 = all(bool3(int3(-125, 0, 0).x == int4(-125, 0, 0, 100).xyz.x, int3(-125, 0, 0).y == int4(-125, 0, 0, 100).xyz.y, int3(-125, 0, 0).z == int4(-125, 0, 0, 100).xyz.z));
    }
    else
    {
        _167 = false;
    }
    bool _170 = false;
    if (_167)
    {
        _170 = true;
    }
    else
    {
        _170 = false;
    }
    float4 _171 = 0.0f.xxxx;
    if (_170)
    {
        _171 = _11_colorGreen;
    }
    else
    {
        _171 = _11_colorRed;
    }
    return _171;
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
