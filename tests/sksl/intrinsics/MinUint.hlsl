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
    float4 _37 = abs(_10_testInputs) * 100.0f;
    uint4 _46 = uint4(uint(_37.x), uint(_37.y), uint(_37.z), uint(_37.w));
    uint4 uintValues = _46;
    float4 _51 = _10_colorGreen * 100.0f;
    uint4 _60 = uint4(uint(_51.x), uint(_51.y), uint(_51.z), uint(_51.w));
    uint4 uintGreen = _60;
    uint4 expectedA = uint4(50u, 0u, 50u, 50u);
    uint4 expectedB = uint4(0u, 0u, 0u, 100u);
    uint _70 = _46.x;
    bool _82 = false;
    if (min(_70, 50u) == 50u)
    {
        uint2 _74 = min(_46.xy, uint2(50u, 50u));
        _82 = all(bool2(_74.x == uint4(50u, 0u, 50u, 50u).xy.x, _74.y == uint4(50u, 0u, 50u, 50u).xy.y));
    }
    else
    {
        _82 = false;
    }
    bool _93 = false;
    if (_82)
    {
        uint3 _85 = min(_46.xyz, uint3(50u, 50u, 50u));
        _93 = all(bool3(_85.x == uint4(50u, 0u, 50u, 50u).xyz.x, _85.y == uint4(50u, 0u, 50u, 50u).xyz.y, _85.z == uint4(50u, 0u, 50u, 50u).xyz.z));
    }
    else
    {
        _93 = false;
    }
    bool _101 = false;
    if (_93)
    {
        uint4 _96 = min(_46, uint4(50u, 50u, 50u, 50u));
        _101 = all(bool4(_96.x == uint4(50u, 0u, 50u, 50u).x, _96.y == uint4(50u, 0u, 50u, 50u).y, _96.z == uint4(50u, 0u, 50u, 50u).z, _96.w == uint4(50u, 0u, 50u, 50u).w));
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
        _112 = all(bool2(uint2(50u, 0u).x == uint4(50u, 0u, 50u, 50u).xy.x, uint2(50u, 0u).y == uint4(50u, 0u, 50u, 50u).xy.y));
    }
    else
    {
        _112 = false;
    }
    bool _119 = false;
    if (_112)
    {
        _119 = all(bool3(uint3(50u, 0u, 50u).x == uint4(50u, 0u, 50u, 50u).xyz.x, uint3(50u, 0u, 50u).y == uint4(50u, 0u, 50u, 50u).xyz.y, uint3(50u, 0u, 50u).z == uint4(50u, 0u, 50u, 50u).xyz.z));
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
        _128 = min(_70, _60.x) == 0u;
    }
    else
    {
        _128 = false;
    }
    bool _137 = false;
    if (_128)
    {
        uint2 _131 = min(_46.xy, _60.xy);
        _137 = all(bool2(_131.x == uint4(0u, 0u, 0u, 100u).xy.x, _131.y == uint4(0u, 0u, 0u, 100u).xy.y));
    }
    else
    {
        _137 = false;
    }
    bool _146 = false;
    if (_137)
    {
        uint3 _140 = min(_46.xyz, _60.xyz);
        _146 = all(bool3(_140.x == uint4(0u, 0u, 0u, 100u).xyz.x, _140.y == uint4(0u, 0u, 0u, 100u).xyz.y, _140.z == uint4(0u, 0u, 0u, 100u).xyz.z));
    }
    else
    {
        _146 = false;
    }
    bool _152 = false;
    if (_146)
    {
        uint4 _149 = min(_46, _60);
        _152 = all(bool4(_149.x == uint4(0u, 0u, 0u, 100u).x, _149.y == uint4(0u, 0u, 0u, 100u).y, _149.z == uint4(0u, 0u, 0u, 100u).z, _149.w == uint4(0u, 0u, 0u, 100u).w));
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
        _162 = all(bool2(uint2(0u, 0u).x == uint4(0u, 0u, 0u, 100u).xy.x, uint2(0u, 0u).y == uint4(0u, 0u, 0u, 100u).xy.y));
    }
    else
    {
        _162 = false;
    }
    bool _169 = false;
    if (_162)
    {
        _169 = all(bool3(uint3(0u, 0u, 0u).x == uint4(0u, 0u, 0u, 100u).xyz.x, uint3(0u, 0u, 0u).y == uint4(0u, 0u, 0u, 100u).xyz.y, uint3(0u, 0u, 0u).z == uint4(0u, 0u, 0u, 100u).xyz.z));
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
        _173 = _10_colorGreen;
    }
    else
    {
        _173 = _10_colorRed;
    }
    return _173;
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
