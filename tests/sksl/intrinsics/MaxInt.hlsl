cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _35 = _10_testInputs * 100.0f;
    int4 _44 = int4(int(_35.x), int(_35.y), int(_35.z), int(_35.w));
    int4 intValues = _44;
    float4 _49 = _10_colorGreen * 100.0f;
    int4 _58 = int4(int(_49.x), int(_49.y), int(_49.z), int(_49.w));
    int4 intGreen = _58;
    int4 expectedA = int4(50, 50, 75, 225);
    int4 expectedB = int4(0, 100, 75, 225);
    int _69 = _44.x;
    bool _81 = false;
    if (max(_69, 50) == 50)
    {
        int2 _73 = max(_44.xy, int2(50, 50));
        _81 = all(bool2(_73.x == int4(50, 50, 75, 225).xy.x, _73.y == int4(50, 50, 75, 225).xy.y));
    }
    else
    {
        _81 = false;
    }
    bool _92 = false;
    if (_81)
    {
        int3 _84 = max(_44.xyz, int3(50, 50, 50));
        _92 = all(bool3(_84.x == int4(50, 50, 75, 225).xyz.x, _84.y == int4(50, 50, 75, 225).xyz.y, _84.z == int4(50, 50, 75, 225).xyz.z));
    }
    else
    {
        _92 = false;
    }
    bool _100 = false;
    if (_92)
    {
        int4 _95 = max(_44, int4(50, 50, 50, 50));
        _100 = all(bool4(_95.x == int4(50, 50, 75, 225).x, _95.y == int4(50, 50, 75, 225).y, _95.z == int4(50, 50, 75, 225).z, _95.w == int4(50, 50, 75, 225).w));
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
    bool _110 = false;
    if (_104)
    {
        _110 = all(bool2(int2(50, 50).x == int4(50, 50, 75, 225).xy.x, int2(50, 50).y == int4(50, 50, 75, 225).xy.y));
    }
    else
    {
        _110 = false;
    }
    bool _117 = false;
    if (_110)
    {
        _117 = all(bool3(int3(50, 50, 75).x == int4(50, 50, 75, 225).xyz.x, int3(50, 50, 75).y == int4(50, 50, 75, 225).xyz.y, int3(50, 50, 75).z == int4(50, 50, 75, 225).xyz.z));
    }
    else
    {
        _117 = false;
    }
    bool _120 = false;
    if (_117)
    {
        _120 = true;
    }
    else
    {
        _120 = false;
    }
    bool _126 = false;
    if (_120)
    {
        _126 = max(_69, _58.x) == 0;
    }
    else
    {
        _126 = false;
    }
    bool _135 = false;
    if (_126)
    {
        int2 _129 = max(_44.xy, _58.xy);
        _135 = all(bool2(_129.x == int4(0, 100, 75, 225).xy.x, _129.y == int4(0, 100, 75, 225).xy.y));
    }
    else
    {
        _135 = false;
    }
    bool _144 = false;
    if (_135)
    {
        int3 _138 = max(_44.xyz, _58.xyz);
        _144 = all(bool3(_138.x == int4(0, 100, 75, 225).xyz.x, _138.y == int4(0, 100, 75, 225).xyz.y, _138.z == int4(0, 100, 75, 225).xyz.z));
    }
    else
    {
        _144 = false;
    }
    bool _150 = false;
    if (_144)
    {
        int4 _147 = max(_44, _58);
        _150 = all(bool4(_147.x == int4(0, 100, 75, 225).x, _147.y == int4(0, 100, 75, 225).y, _147.z == int4(0, 100, 75, 225).z, _147.w == int4(0, 100, 75, 225).w));
    }
    else
    {
        _150 = false;
    }
    bool _153 = false;
    if (_150)
    {
        _153 = true;
    }
    else
    {
        _153 = false;
    }
    bool _160 = false;
    if (_153)
    {
        _160 = all(bool2(int2(0, 100).x == int4(0, 100, 75, 225).xy.x, int2(0, 100).y == int4(0, 100, 75, 225).xy.y));
    }
    else
    {
        _160 = false;
    }
    bool _167 = false;
    if (_160)
    {
        _167 = all(bool3(int3(0, 100, 75).x == int4(0, 100, 75, 225).xyz.x, int3(0, 100, 75).y == int4(0, 100, 75, 225).xyz.y, int3(0, 100, 75).z == int4(0, 100, 75, 225).xyz.z));
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
        _171 = _10_colorGreen;
    }
    else
    {
        _171 = _10_colorRed;
    }
    return _171;
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
