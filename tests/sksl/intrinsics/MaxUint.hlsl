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
    float4 _37 = abs(_11_testInputs) * 100.0f;
    uint4 _46 = uint4(uint(_37.x), uint(_37.y), uint(_37.z), uint(_37.w));
    uint4 uintValues = _46;
    float4 _51 = _11_colorGreen * 100.0f;
    uint4 _60 = uint4(uint(_51.x), uint(_51.y), uint(_51.z), uint(_51.w));
    uint4 uintGreen = _60;
    uint4 expectedA = uint4(125u, 80u, 80u, 225u);
    uint4 expectedB = uint4(125u, 100u, 75u, 225u);
    uint _73 = _46.x;
    bool _85 = false;
    if (max(_73, 80u) == 125u)
    {
        uint2 _77 = max(_46.xy, uint2(80u, 80u));
        _85 = all(bool2(_77.x == uint4(125u, 80u, 80u, 225u).xy.x, _77.y == uint4(125u, 80u, 80u, 225u).xy.y));
    }
    else
    {
        _85 = false;
    }
    bool _96 = false;
    if (_85)
    {
        uint3 _88 = max(_46.xyz, uint3(80u, 80u, 80u));
        _96 = all(bool3(_88.x == uint4(125u, 80u, 80u, 225u).xyz.x, _88.y == uint4(125u, 80u, 80u, 225u).xyz.y, _88.z == uint4(125u, 80u, 80u, 225u).xyz.z));
    }
    else
    {
        _96 = false;
    }
    bool _104 = false;
    if (_96)
    {
        uint4 _99 = max(_46, uint4(80u, 80u, 80u, 80u));
        _104 = all(bool4(_99.x == uint4(125u, 80u, 80u, 225u).x, _99.y == uint4(125u, 80u, 80u, 225u).y, _99.z == uint4(125u, 80u, 80u, 225u).z, _99.w == uint4(125u, 80u, 80u, 225u).w));
    }
    else
    {
        _104 = false;
    }
    bool _108 = false;
    if (_104)
    {
        _108 = true;
    }
    else
    {
        _108 = false;
    }
    bool _115 = false;
    if (_108)
    {
        _115 = all(bool2(uint2(125u, 80u).x == uint4(125u, 80u, 80u, 225u).xy.x, uint2(125u, 80u).y == uint4(125u, 80u, 80u, 225u).xy.y));
    }
    else
    {
        _115 = false;
    }
    bool _122 = false;
    if (_115)
    {
        _122 = all(bool3(uint3(125u, 80u, 80u).x == uint4(125u, 80u, 80u, 225u).xyz.x, uint3(125u, 80u, 80u).y == uint4(125u, 80u, 80u, 225u).xyz.y, uint3(125u, 80u, 80u).z == uint4(125u, 80u, 80u, 225u).xyz.z));
    }
    else
    {
        _122 = false;
    }
    bool _125 = false;
    if (_122)
    {
        _125 = true;
    }
    else
    {
        _125 = false;
    }
    bool _131 = false;
    if (_125)
    {
        _131 = max(_73, _60.x) == 125u;
    }
    else
    {
        _131 = false;
    }
    bool _140 = false;
    if (_131)
    {
        uint2 _134 = max(_46.xy, _60.xy);
        _140 = all(bool2(_134.x == uint4(125u, 100u, 75u, 225u).xy.x, _134.y == uint4(125u, 100u, 75u, 225u).xy.y));
    }
    else
    {
        _140 = false;
    }
    bool _149 = false;
    if (_140)
    {
        uint3 _143 = max(_46.xyz, _60.xyz);
        _149 = all(bool3(_143.x == uint4(125u, 100u, 75u, 225u).xyz.x, _143.y == uint4(125u, 100u, 75u, 225u).xyz.y, _143.z == uint4(125u, 100u, 75u, 225u).xyz.z));
    }
    else
    {
        _149 = false;
    }
    bool _155 = false;
    if (_149)
    {
        uint4 _152 = max(_46, _60);
        _155 = all(bool4(_152.x == uint4(125u, 100u, 75u, 225u).x, _152.y == uint4(125u, 100u, 75u, 225u).y, _152.z == uint4(125u, 100u, 75u, 225u).z, _152.w == uint4(125u, 100u, 75u, 225u).w));
    }
    else
    {
        _155 = false;
    }
    bool _158 = false;
    if (_155)
    {
        _158 = true;
    }
    else
    {
        _158 = false;
    }
    bool _165 = false;
    if (_158)
    {
        _165 = all(bool2(uint2(125u, 100u).x == uint4(125u, 100u, 75u, 225u).xy.x, uint2(125u, 100u).y == uint4(125u, 100u, 75u, 225u).xy.y));
    }
    else
    {
        _165 = false;
    }
    bool _172 = false;
    if (_165)
    {
        _172 = all(bool3(uint3(125u, 100u, 75u).x == uint4(125u, 100u, 75u, 225u).xyz.x, uint3(125u, 100u, 75u).y == uint4(125u, 100u, 75u, 225u).xyz.y, uint3(125u, 100u, 75u).z == uint4(125u, 100u, 75u, 225u).xyz.z));
    }
    else
    {
        _172 = false;
    }
    bool _175 = false;
    if (_172)
    {
        _175 = true;
    }
    else
    {
        _175 = false;
    }
    float4 _176 = 0.0f.xxxx;
    if (_175)
    {
        _176 = _11_colorGreen;
    }
    else
    {
        _176 = _11_colorRed;
    }
    return _176;
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
