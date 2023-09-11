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
    uint4 expectedA = uint4(125u, 80u, 80u, 225u);
    uint4 expectedB = uint4(125u, 100u, 75u, 225u);
    uint _70 = _43.x;
    bool _82 = false;
    if (max(_70, 80u) == 125u)
    {
        uint2 _74 = max(_43.xy, uint2(80u, 80u));
        _82 = all(bool2(_74.x == uint4(125u, 80u, 80u, 225u).xy.x, _74.y == uint4(125u, 80u, 80u, 225u).xy.y));
    }
    else
    {
        _82 = false;
    }
    bool _93 = false;
    if (_82)
    {
        uint3 _85 = max(_43.xyz, uint3(80u, 80u, 80u));
        _93 = all(bool3(_85.x == uint4(125u, 80u, 80u, 225u).xyz.x, _85.y == uint4(125u, 80u, 80u, 225u).xyz.y, _85.z == uint4(125u, 80u, 80u, 225u).xyz.z));
    }
    else
    {
        _93 = false;
    }
    bool _101 = false;
    if (_93)
    {
        uint4 _96 = max(_43, uint4(80u, 80u, 80u, 80u));
        _101 = all(bool4(_96.x == uint4(125u, 80u, 80u, 225u).x, _96.y == uint4(125u, 80u, 80u, 225u).y, _96.z == uint4(125u, 80u, 80u, 225u).z, _96.w == uint4(125u, 80u, 80u, 225u).w));
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
    bool _112 = false;
    if (_105)
    {
        _112 = all(bool2(uint2(125u, 80u).x == uint4(125u, 80u, 80u, 225u).xy.x, uint2(125u, 80u).y == uint4(125u, 80u, 80u, 225u).xy.y));
    }
    else
    {
        _112 = false;
    }
    bool _119 = false;
    if (_112)
    {
        _119 = all(bool3(uint3(125u, 80u, 80u).x == uint4(125u, 80u, 80u, 225u).xyz.x, uint3(125u, 80u, 80u).y == uint4(125u, 80u, 80u, 225u).xyz.y, uint3(125u, 80u, 80u).z == uint4(125u, 80u, 80u, 225u).xyz.z));
    }
    else
    {
        _119 = false;
    }
    bool _122 = false;
    if (_119)
    {
        _122 = true;
    }
    else
    {
        _122 = false;
    }
    bool _128 = false;
    if (_122)
    {
        _128 = max(_70, _57.x) == 125u;
    }
    else
    {
        _128 = false;
    }
    bool _137 = false;
    if (_128)
    {
        uint2 _131 = max(_43.xy, _57.xy);
        _137 = all(bool2(_131.x == uint4(125u, 100u, 75u, 225u).xy.x, _131.y == uint4(125u, 100u, 75u, 225u).xy.y));
    }
    else
    {
        _137 = false;
    }
    bool _146 = false;
    if (_137)
    {
        uint3 _140 = max(_43.xyz, _57.xyz);
        _146 = all(bool3(_140.x == uint4(125u, 100u, 75u, 225u).xyz.x, _140.y == uint4(125u, 100u, 75u, 225u).xyz.y, _140.z == uint4(125u, 100u, 75u, 225u).xyz.z));
    }
    else
    {
        _146 = false;
    }
    bool _152 = false;
    if (_146)
    {
        uint4 _149 = max(_43, _57);
        _152 = all(bool4(_149.x == uint4(125u, 100u, 75u, 225u).x, _149.y == uint4(125u, 100u, 75u, 225u).y, _149.z == uint4(125u, 100u, 75u, 225u).z, _149.w == uint4(125u, 100u, 75u, 225u).w));
    }
    else
    {
        _152 = false;
    }
    bool _155 = false;
    if (_152)
    {
        _155 = true;
    }
    else
    {
        _155 = false;
    }
    bool _162 = false;
    if (_155)
    {
        _162 = all(bool2(uint2(125u, 100u).x == uint4(125u, 100u, 75u, 225u).xy.x, uint2(125u, 100u).y == uint4(125u, 100u, 75u, 225u).xy.y));
    }
    else
    {
        _162 = false;
    }
    bool _169 = false;
    if (_162)
    {
        _169 = all(bool3(uint3(125u, 100u, 75u).x == uint4(125u, 100u, 75u, 225u).xyz.x, uint3(125u, 100u, 75u).y == uint4(125u, 100u, 75u, 225u).xyz.y, uint3(125u, 100u, 75u).z == uint4(125u, 100u, 75u, 225u).xyz.z));
    }
    else
    {
        _169 = false;
    }
    bool _172 = false;
    if (_169)
    {
        _172 = true;
    }
    else
    {
        _172 = false;
    }
    float4 _173 = 0.0f.xxxx;
    if (_172)
    {
        _173 = _7_colorGreen;
    }
    else
    {
        _173 = _7_colorRed;
    }
    return _173;
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
