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
    uint4 uintValues = uint4(uint(_39.x), uint(_39.y), uint(_39.z), uint(_39.w));
    uint4 expectedA = uint4(100u, 200u, 275u, 300u);
    uint4 clampLow = uint4(100u, 0u, 0u, 300u);
    uint4 expectedB = uint4(100u, 200u, 250u, 425u);
    uint4 clampHigh = uint4(300u, 400u, 250u, 500u);
    bool _86 = false;
    if (clamp(uintValues.x, 100u, 300u) == expectedA.x)
    {
        uint2 _75 = clamp(uintValues.xy, uint2(100u, 100u), uint2(300u, 300u));
        _86 = all(bool2(_75.x == expectedA.xy.x, _75.y == expectedA.xy.y));
    }
    else
    {
        _86 = false;
    }
    bool _100 = false;
    if (_86)
    {
        uint3 _89 = clamp(uintValues.xyz, uint3(100u, 100u, 100u), uint3(300u, 300u, 300u));
        _100 = all(bool3(_89.x == expectedA.xyz.x, _89.y == expectedA.xyz.y, _89.z == expectedA.xyz.z));
    }
    else
    {
        _100 = false;
    }
    bool _111 = false;
    if (_100)
    {
        uint4 _103 = clamp(uintValues, uint4(100u, 100u, 100u, 100u), uint4(300u, 300u, 300u, 300u));
        _111 = all(bool4(_103.x == expectedA.x, _103.y == expectedA.y, _103.z == expectedA.z, _103.w == expectedA.w));
    }
    else
    {
        _111 = false;
    }
    bool _117 = false;
    if (_111)
    {
        _117 = 100u == expectedA.x;
    }
    else
    {
        _117 = false;
    }
    bool _125 = false;
    if (_117)
    {
        _125 = all(bool2(uint2(100u, 200u).x == expectedA.xy.x, uint2(100u, 200u).y == expectedA.xy.y));
    }
    else
    {
        _125 = false;
    }
    bool _133 = false;
    if (_125)
    {
        _133 = all(bool3(uint3(100u, 200u, 275u).x == expectedA.xyz.x, uint3(100u, 200u, 275u).y == expectedA.xyz.y, uint3(100u, 200u, 275u).z == expectedA.xyz.z));
    }
    else
    {
        _133 = false;
    }
    bool _139 = false;
    if (_133)
    {
        _139 = all(bool4(uint4(100u, 200u, 275u, 300u).x == expectedA.x, uint4(100u, 200u, 275u, 300u).y == expectedA.y, uint4(100u, 200u, 275u, 300u).z == expectedA.z, uint4(100u, 200u, 275u, 300u).w == expectedA.w));
    }
    else
    {
        _139 = false;
    }
    bool _148 = false;
    if (_139)
    {
        _148 = clamp(uintValues.x, 100u, 300u) == expectedB.x;
    }
    else
    {
        _148 = false;
    }
    bool _160 = false;
    if (_148)
    {
        uint2 _151 = clamp(uintValues.xy, uint2(100u, 0u), uint2(300u, 400u));
        _160 = all(bool2(_151.x == expectedB.xy.x, _151.y == expectedB.xy.y));
    }
    else
    {
        _160 = false;
    }
    bool _172 = false;
    if (_160)
    {
        uint3 _163 = clamp(uintValues.xyz, uint3(100u, 0u, 0u), uint3(300u, 400u, 250u));
        _172 = all(bool3(_163.x == expectedB.xyz.x, _163.y == expectedB.xyz.y, _163.z == expectedB.xyz.z));
    }
    else
    {
        _172 = false;
    }
    bool _182 = false;
    if (_172)
    {
        uint4 _175 = clamp(uintValues, clampLow, clampHigh);
        _182 = all(bool4(_175.x == expectedB.x, _175.y == expectedB.y, _175.z == expectedB.z, _175.w == expectedB.w));
    }
    else
    {
        _182 = false;
    }
    bool _188 = false;
    if (_182)
    {
        _188 = 100u == expectedB.x;
    }
    else
    {
        _188 = false;
    }
    bool _195 = false;
    if (_188)
    {
        _195 = all(bool2(uint2(100u, 200u).x == expectedB.xy.x, uint2(100u, 200u).y == expectedB.xy.y));
    }
    else
    {
        _195 = false;
    }
    bool _203 = false;
    if (_195)
    {
        _203 = all(bool3(uint3(100u, 200u, 250u).x == expectedB.xyz.x, uint3(100u, 200u, 250u).y == expectedB.xyz.y, uint3(100u, 200u, 250u).z == expectedB.xyz.z));
    }
    else
    {
        _203 = false;
    }
    bool _209 = false;
    if (_203)
    {
        _209 = all(bool4(uint4(100u, 200u, 250u, 425u).x == expectedB.x, uint4(100u, 200u, 250u, 425u).y == expectedB.y, uint4(100u, 200u, 250u, 425u).z == expectedB.z, uint4(100u, 200u, 250u, 425u).w == expectedB.w));
    }
    else
    {
        _209 = false;
    }
    float4 _210 = 0.0f.xxxx;
    if (_209)
    {
        _210 = _10_colorGreen;
    }
    else
    {
        _210 = _10_colorRed;
    }
    return _210;
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
