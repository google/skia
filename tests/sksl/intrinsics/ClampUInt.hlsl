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
    float4 _39 = (_10_testInputs * 100.0f) + 200.0f.xxxx;
    uint4 _48 = uint4(uint(_39.x), uint(_39.y), uint(_39.z), uint(_39.w));
    uint4 uintValues = _48;
    uint4 expectedA = uint4(100u, 200u, 275u, 300u);
    uint4 expectedB = uint4(100u, 200u, 250u, 425u);
    uint _61 = _48.x;
    bool _74 = false;
    if (clamp(_61, 100u, 300u) == 100u)
    {
        uint2 _65 = clamp(_48.xy, uint2(100u, 100u), uint2(300u, 300u));
        _74 = all(bool2(_65.x == uint4(100u, 200u, 275u, 300u).xy.x, _65.y == uint4(100u, 200u, 275u, 300u).xy.y));
    }
    else
    {
        _74 = false;
    }
    bool _86 = false;
    if (_74)
    {
        uint3 _77 = clamp(_48.xyz, uint3(100u, 100u, 100u), uint3(300u, 300u, 300u));
        _86 = all(bool3(_77.x == uint4(100u, 200u, 275u, 300u).xyz.x, _77.y == uint4(100u, 200u, 275u, 300u).xyz.y, _77.z == uint4(100u, 200u, 275u, 300u).xyz.z));
    }
    else
    {
        _86 = false;
    }
    bool _95 = false;
    if (_86)
    {
        uint4 _89 = clamp(_48, uint4(100u, 100u, 100u, 100u), uint4(300u, 300u, 300u, 300u));
        _95 = all(bool4(_89.x == uint4(100u, 200u, 275u, 300u).x, _89.y == uint4(100u, 200u, 275u, 300u).y, _89.z == uint4(100u, 200u, 275u, 300u).z, _89.w == uint4(100u, 200u, 275u, 300u).w));
    }
    else
    {
        _95 = false;
    }
    bool _99 = false;
    if (_95)
    {
        _99 = true;
    }
    else
    {
        _99 = false;
    }
    bool _106 = false;
    if (_99)
    {
        _106 = all(bool2(uint2(100u, 200u).x == uint4(100u, 200u, 275u, 300u).xy.x, uint2(100u, 200u).y == uint4(100u, 200u, 275u, 300u).xy.y));
    }
    else
    {
        _106 = false;
    }
    bool _113 = false;
    if (_106)
    {
        _113 = all(bool3(uint3(100u, 200u, 275u).x == uint4(100u, 200u, 275u, 300u).xyz.x, uint3(100u, 200u, 275u).y == uint4(100u, 200u, 275u, 300u).xyz.y, uint3(100u, 200u, 275u).z == uint4(100u, 200u, 275u, 300u).xyz.z));
    }
    else
    {
        _113 = false;
    }
    bool _116 = false;
    if (_113)
    {
        _116 = true;
    }
    else
    {
        _116 = false;
    }
    bool _121 = false;
    if (_116)
    {
        _121 = clamp(_61, 100u, 300u) == 100u;
    }
    else
    {
        _121 = false;
    }
    bool _133 = false;
    if (_121)
    {
        uint2 _124 = clamp(_48.xy, uint2(100u, 0u), uint2(300u, 400u));
        _133 = all(bool2(_124.x == uint4(100u, 200u, 250u, 425u).xy.x, _124.y == uint4(100u, 200u, 250u, 425u).xy.y));
    }
    else
    {
        _133 = false;
    }
    bool _143 = false;
    if (_133)
    {
        uint3 _136 = clamp(_48.xyz, uint3(100u, 0u, 0u), uint3(300u, 400u, 250u));
        _143 = all(bool3(_136.x == uint4(100u, 200u, 250u, 425u).xyz.x, _136.y == uint4(100u, 200u, 250u, 425u).xyz.y, _136.z == uint4(100u, 200u, 250u, 425u).xyz.z));
    }
    else
    {
        _143 = false;
    }
    bool _152 = false;
    if (_143)
    {
        uint4 _146 = clamp(_48, uint4(100u, 0u, 0u, 300u), uint4(300u, 400u, 250u, 500u));
        _152 = all(bool4(_146.x == uint4(100u, 200u, 250u, 425u).x, _146.y == uint4(100u, 200u, 250u, 425u).y, _146.z == uint4(100u, 200u, 250u, 425u).z, _146.w == uint4(100u, 200u, 250u, 425u).w));
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
    bool _161 = false;
    if (_155)
    {
        _161 = all(bool2(uint2(100u, 200u).x == uint4(100u, 200u, 250u, 425u).xy.x, uint2(100u, 200u).y == uint4(100u, 200u, 250u, 425u).xy.y));
    }
    else
    {
        _161 = false;
    }
    bool _168 = false;
    if (_161)
    {
        _168 = all(bool3(uint3(100u, 200u, 250u).x == uint4(100u, 200u, 250u, 425u).xyz.x, uint3(100u, 200u, 250u).y == uint4(100u, 200u, 250u, 425u).xyz.y, uint3(100u, 200u, 250u).z == uint4(100u, 200u, 250u, 425u).xyz.z));
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
        _172 = _10_colorGreen;
    }
    else
    {
        _172 = _10_colorRed;
    }
    return _172;
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
