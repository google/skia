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
    float4 _39 = (_11_testInputs * 100.0f) + 200.0f.xxxx;
    uint4 _48 = uint4(uint(_39.x), uint(_39.y), uint(_39.z), uint(_39.w));
    uint4 uintValues = _48;
    uint4 expectedA = uint4(100u, 200u, 275u, 300u);
    uint4 expectedB = uint4(100u, 200u, 250u, 425u);
    uint _62 = _48.x;
    bool _75 = false;
    if (clamp(_62, 100u, 300u) == 100u)
    {
        uint2 _66 = clamp(_48.xy, uint2(100u, 100u), uint2(300u, 300u));
        _75 = all(bool2(_66.x == uint4(100u, 200u, 275u, 300u).xy.x, _66.y == uint4(100u, 200u, 275u, 300u).xy.y));
    }
    else
    {
        _75 = false;
    }
    bool _87 = false;
    if (_75)
    {
        uint3 _78 = clamp(_48.xyz, uint3(100u, 100u, 100u), uint3(300u, 300u, 300u));
        _87 = all(bool3(_78.x == uint4(100u, 200u, 275u, 300u).xyz.x, _78.y == uint4(100u, 200u, 275u, 300u).xyz.y, _78.z == uint4(100u, 200u, 275u, 300u).xyz.z));
    }
    else
    {
        _87 = false;
    }
    bool _96 = false;
    if (_87)
    {
        uint4 _90 = clamp(_48, uint4(100u, 100u, 100u, 100u), uint4(300u, 300u, 300u, 300u));
        _96 = all(bool4(_90.x == uint4(100u, 200u, 275u, 300u).x, _90.y == uint4(100u, 200u, 275u, 300u).y, _90.z == uint4(100u, 200u, 275u, 300u).z, _90.w == uint4(100u, 200u, 275u, 300u).w));
    }
    else
    {
        _96 = false;
    }
    bool _100 = false;
    if (_96)
    {
        _100 = true;
    }
    else
    {
        _100 = false;
    }
    bool _107 = false;
    if (_100)
    {
        _107 = all(bool2(uint2(100u, 200u).x == uint4(100u, 200u, 275u, 300u).xy.x, uint2(100u, 200u).y == uint4(100u, 200u, 275u, 300u).xy.y));
    }
    else
    {
        _107 = false;
    }
    bool _114 = false;
    if (_107)
    {
        _114 = all(bool3(uint3(100u, 200u, 275u).x == uint4(100u, 200u, 275u, 300u).xyz.x, uint3(100u, 200u, 275u).y == uint4(100u, 200u, 275u, 300u).xyz.y, uint3(100u, 200u, 275u).z == uint4(100u, 200u, 275u, 300u).xyz.z));
    }
    else
    {
        _114 = false;
    }
    bool _117 = false;
    if (_114)
    {
        _117 = true;
    }
    else
    {
        _117 = false;
    }
    bool _122 = false;
    if (_117)
    {
        _122 = clamp(_62, 100u, 300u) == 100u;
    }
    else
    {
        _122 = false;
    }
    bool _134 = false;
    if (_122)
    {
        uint2 _125 = clamp(_48.xy, uint2(100u, 0u), uint2(300u, 400u));
        _134 = all(bool2(_125.x == uint4(100u, 200u, 250u, 425u).xy.x, _125.y == uint4(100u, 200u, 250u, 425u).xy.y));
    }
    else
    {
        _134 = false;
    }
    bool _144 = false;
    if (_134)
    {
        uint3 _137 = clamp(_48.xyz, uint3(100u, 0u, 0u), uint3(300u, 400u, 250u));
        _144 = all(bool3(_137.x == uint4(100u, 200u, 250u, 425u).xyz.x, _137.y == uint4(100u, 200u, 250u, 425u).xyz.y, _137.z == uint4(100u, 200u, 250u, 425u).xyz.z));
    }
    else
    {
        _144 = false;
    }
    bool _153 = false;
    if (_144)
    {
        uint4 _147 = clamp(_48, uint4(100u, 0u, 0u, 300u), uint4(300u, 400u, 250u, 500u));
        _153 = all(bool4(_147.x == uint4(100u, 200u, 250u, 425u).x, _147.y == uint4(100u, 200u, 250u, 425u).y, _147.z == uint4(100u, 200u, 250u, 425u).z, _147.w == uint4(100u, 200u, 250u, 425u).w));
    }
    else
    {
        _153 = false;
    }
    bool _156 = false;
    if (_153)
    {
        _156 = true;
    }
    else
    {
        _156 = false;
    }
    bool _162 = false;
    if (_156)
    {
        _162 = all(bool2(uint2(100u, 200u).x == uint4(100u, 200u, 250u, 425u).xy.x, uint2(100u, 200u).y == uint4(100u, 200u, 250u, 425u).xy.y));
    }
    else
    {
        _162 = false;
    }
    bool _169 = false;
    if (_162)
    {
        _169 = all(bool3(uint3(100u, 200u, 250u).x == uint4(100u, 200u, 250u, 425u).xyz.x, uint3(100u, 200u, 250u).y == uint4(100u, 200u, 250u, 425u).xyz.y, uint3(100u, 200u, 250u).z == uint4(100u, 200u, 250u, 425u).xyz.z));
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
        _173 = _11_colorGreen;
    }
    else
    {
        _173 = _11_colorRed;
    }
    return _173;
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
