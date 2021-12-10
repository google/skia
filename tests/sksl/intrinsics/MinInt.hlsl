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
    int4 expectedA = int4(-125, 0, 50, 50);
    int4 expectedB = int4(-125, 0, 0, 100);
    int _68 = _44.x;
    bool _80 = false;
    if (min(_68, 50) == (-125))
    {
        int2 _72 = min(_44.xy, int2(50, 50));
        _80 = all(bool2(_72.x == int4(-125, 0, 50, 50).xy.x, _72.y == int4(-125, 0, 50, 50).xy.y));
    }
    else
    {
        _80 = false;
    }
    bool _91 = false;
    if (_80)
    {
        int3 _83 = min(_44.xyz, int3(50, 50, 50));
        _91 = all(bool3(_83.x == int4(-125, 0, 50, 50).xyz.x, _83.y == int4(-125, 0, 50, 50).xyz.y, _83.z == int4(-125, 0, 50, 50).xyz.z));
    }
    else
    {
        _91 = false;
    }
    bool _99 = false;
    if (_91)
    {
        int4 _94 = min(_44, int4(50, 50, 50, 50));
        _99 = all(bool4(_94.x == int4(-125, 0, 50, 50).x, _94.y == int4(-125, 0, 50, 50).y, _94.z == int4(-125, 0, 50, 50).z, _94.w == int4(-125, 0, 50, 50).w));
    }
    else
    {
        _99 = false;
    }
    bool _103 = false;
    if (_99)
    {
        _103 = true;
    }
    else
    {
        _103 = false;
    }
    bool _110 = false;
    if (_103)
    {
        _110 = all(bool2(int2(-125, 0).x == int4(-125, 0, 50, 50).xy.x, int2(-125, 0).y == int4(-125, 0, 50, 50).xy.y));
    }
    else
    {
        _110 = false;
    }
    bool _117 = false;
    if (_110)
    {
        _117 = all(bool3(int3(-125, 0, 50).x == int4(-125, 0, 50, 50).xyz.x, int3(-125, 0, 50).y == int4(-125, 0, 50, 50).xyz.y, int3(-125, 0, 50).z == int4(-125, 0, 50, 50).xyz.z));
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
        _126 = min(_68, _58.x) == (-125);
    }
    else
    {
        _126 = false;
    }
    bool _135 = false;
    if (_126)
    {
        int2 _129 = min(_44.xy, _58.xy);
        _135 = all(bool2(_129.x == int4(-125, 0, 0, 100).xy.x, _129.y == int4(-125, 0, 0, 100).xy.y));
    }
    else
    {
        _135 = false;
    }
    bool _144 = false;
    if (_135)
    {
        int3 _138 = min(_44.xyz, _58.xyz);
        _144 = all(bool3(_138.x == int4(-125, 0, 0, 100).xyz.x, _138.y == int4(-125, 0, 0, 100).xyz.y, _138.z == int4(-125, 0, 0, 100).xyz.z));
    }
    else
    {
        _144 = false;
    }
    bool _150 = false;
    if (_144)
    {
        int4 _147 = min(_44, _58);
        _150 = all(bool4(_147.x == int4(-125, 0, 0, 100).x, _147.y == int4(-125, 0, 0, 100).y, _147.z == int4(-125, 0, 0, 100).z, _147.w == int4(-125, 0, 0, 100).w));
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
    bool _159 = false;
    if (_153)
    {
        _159 = all(bool2(int2(-125, 0).x == int4(-125, 0, 0, 100).xy.x, int2(-125, 0).y == int4(-125, 0, 0, 100).xy.y));
    }
    else
    {
        _159 = false;
    }
    bool _166 = false;
    if (_159)
    {
        _166 = all(bool3(int3(-125, 0, 0).x == int4(-125, 0, 0, 100).xyz.x, int3(-125, 0, 0).y == int4(-125, 0, 0, 100).xyz.y, int3(-125, 0, 0).z == int4(-125, 0, 0, 100).xyz.z));
    }
    else
    {
        _166 = false;
    }
    bool _169 = false;
    if (_166)
    {
        _169 = true;
    }
    else
    {
        _169 = false;
    }
    float4 _170 = 0.0f.xxxx;
    if (_169)
    {
        _170 = _10_colorGreen;
    }
    else
    {
        _170 = _10_colorRed;
    }
    return _170;
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
