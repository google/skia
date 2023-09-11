cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 _34 = abs(_7_testInputs) * 100.0f;
    uint4 _43 = uint4(uint(_34.x), uint(_34.y), uint(_34.z), uint(_34.w));
    uint4 uintValues = _43;
    float4 _48 = _7_colorGreen * 100.0f;
    uint4 _57 = uint4(uint(_48.x), uint(_48.y), uint(_48.z), uint(_48.w));
    uint4 uintGreen = _57;
    uint4 expectedA = uint4(50u, 0u, 50u, 50u);
    uint4 expectedB = uint4(0u, 0u, 0u, 100u);
    uint _68 = _43.x;
    bool _80 = false;
    if (min(_68, 50u) == 50u)
    {
        uint2 _72 = min(_43.xy, uint2(50u, 50u));
        _80 = all(bool2(_72.x == uint4(50u, 0u, 50u, 50u).xy.x, _72.y == uint4(50u, 0u, 50u, 50u).xy.y));
    }
    else
    {
        _80 = false;
    }
    bool _91 = false;
    if (_80)
    {
        uint3 _83 = min(_43.xyz, uint3(50u, 50u, 50u));
        _91 = all(bool3(_83.x == uint4(50u, 0u, 50u, 50u).xyz.x, _83.y == uint4(50u, 0u, 50u, 50u).xyz.y, _83.z == uint4(50u, 0u, 50u, 50u).xyz.z));
    }
    else
    {
        _91 = false;
    }
    bool _99 = false;
    if (_91)
    {
        uint4 _94 = min(_43, uint4(50u, 50u, 50u, 50u));
        _99 = all(bool4(_94.x == uint4(50u, 0u, 50u, 50u).x, _94.y == uint4(50u, 0u, 50u, 50u).y, _94.z == uint4(50u, 0u, 50u, 50u).z, _94.w == uint4(50u, 0u, 50u, 50u).w));
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
        _110 = all(bool2(uint2(50u, 0u).x == uint4(50u, 0u, 50u, 50u).xy.x, uint2(50u, 0u).y == uint4(50u, 0u, 50u, 50u).xy.y));
    }
    else
    {
        _110 = false;
    }
    bool _117 = false;
    if (_110)
    {
        _117 = all(bool3(uint3(50u, 0u, 50u).x == uint4(50u, 0u, 50u, 50u).xyz.x, uint3(50u, 0u, 50u).y == uint4(50u, 0u, 50u, 50u).xyz.y, uint3(50u, 0u, 50u).z == uint4(50u, 0u, 50u, 50u).xyz.z));
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
        _126 = min(_68, _57.x) == 0u;
    }
    else
    {
        _126 = false;
    }
    bool _135 = false;
    if (_126)
    {
        uint2 _129 = min(_43.xy, _57.xy);
        _135 = all(bool2(_129.x == uint4(0u, 0u, 0u, 100u).xy.x, _129.y == uint4(0u, 0u, 0u, 100u).xy.y));
    }
    else
    {
        _135 = false;
    }
    bool _144 = false;
    if (_135)
    {
        uint3 _138 = min(_43.xyz, _57.xyz);
        _144 = all(bool3(_138.x == uint4(0u, 0u, 0u, 100u).xyz.x, _138.y == uint4(0u, 0u, 0u, 100u).xyz.y, _138.z == uint4(0u, 0u, 0u, 100u).xyz.z));
    }
    else
    {
        _144 = false;
    }
    bool _150 = false;
    if (_144)
    {
        uint4 _147 = min(_43, _57);
        _150 = all(bool4(_147.x == uint4(0u, 0u, 0u, 100u).x, _147.y == uint4(0u, 0u, 0u, 100u).y, _147.z == uint4(0u, 0u, 0u, 100u).z, _147.w == uint4(0u, 0u, 0u, 100u).w));
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
        _160 = all(bool2(uint2(0u, 0u).x == uint4(0u, 0u, 0u, 100u).xy.x, uint2(0u, 0u).y == uint4(0u, 0u, 0u, 100u).xy.y));
    }
    else
    {
        _160 = false;
    }
    bool _167 = false;
    if (_160)
    {
        _167 = all(bool3(uint3(0u, 0u, 0u).x == uint4(0u, 0u, 0u, 100u).xyz.x, uint3(0u, 0u, 0u).y == uint4(0u, 0u, 0u, 100u).xyz.y, uint3(0u, 0u, 0u).z == uint4(0u, 0u, 0u, 100u).xyz.z));
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
        _171 = _7_colorGreen;
    }
    else
    {
        _171 = _7_colorRed;
    }
    return _171;
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
