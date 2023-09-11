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
    float4 _36 = (_7_testInputs * 100.0f) + 200.0f.xxxx;
    uint4 _45 = uint4(uint(_36.x), uint(_36.y), uint(_36.z), uint(_36.w));
    uint4 uintValues = _45;
    uint4 expectedA = uint4(100u, 200u, 275u, 300u);
    uint4 expectedB = uint4(100u, 200u, 250u, 425u);
    uint _59 = _45.x;
    bool _72 = false;
    if (clamp(_59, 100u, 300u) == 100u)
    {
        uint2 _63 = clamp(_45.xy, uint2(100u, 100u), uint2(300u, 300u));
        _72 = all(bool2(_63.x == uint4(100u, 200u, 275u, 300u).xy.x, _63.y == uint4(100u, 200u, 275u, 300u).xy.y));
    }
    else
    {
        _72 = false;
    }
    bool _84 = false;
    if (_72)
    {
        uint3 _75 = clamp(_45.xyz, uint3(100u, 100u, 100u), uint3(300u, 300u, 300u));
        _84 = all(bool3(_75.x == uint4(100u, 200u, 275u, 300u).xyz.x, _75.y == uint4(100u, 200u, 275u, 300u).xyz.y, _75.z == uint4(100u, 200u, 275u, 300u).xyz.z));
    }
    else
    {
        _84 = false;
    }
    bool _93 = false;
    if (_84)
    {
        uint4 _87 = clamp(_45, uint4(100u, 100u, 100u, 100u), uint4(300u, 300u, 300u, 300u));
        _93 = all(bool4(_87.x == uint4(100u, 200u, 275u, 300u).x, _87.y == uint4(100u, 200u, 275u, 300u).y, _87.z == uint4(100u, 200u, 275u, 300u).z, _87.w == uint4(100u, 200u, 275u, 300u).w));
    }
    else
    {
        _93 = false;
    }
    bool _97 = false;
    if (_93)
    {
        _97 = true;
    }
    else
    {
        _97 = false;
    }
    bool _104 = false;
    if (_97)
    {
        _104 = all(bool2(uint2(100u, 200u).x == uint4(100u, 200u, 275u, 300u).xy.x, uint2(100u, 200u).y == uint4(100u, 200u, 275u, 300u).xy.y));
    }
    else
    {
        _104 = false;
    }
    bool _111 = false;
    if (_104)
    {
        _111 = all(bool3(uint3(100u, 200u, 275u).x == uint4(100u, 200u, 275u, 300u).xyz.x, uint3(100u, 200u, 275u).y == uint4(100u, 200u, 275u, 300u).xyz.y, uint3(100u, 200u, 275u).z == uint4(100u, 200u, 275u, 300u).xyz.z));
    }
    else
    {
        _111 = false;
    }
    bool _114 = false;
    if (_111)
    {
        _114 = true;
    }
    else
    {
        _114 = false;
    }
    bool _119 = false;
    if (_114)
    {
        _119 = clamp(_59, 100u, 300u) == 100u;
    }
    else
    {
        _119 = false;
    }
    bool _131 = false;
    if (_119)
    {
        uint2 _122 = clamp(_45.xy, uint2(100u, 0u), uint2(300u, 400u));
        _131 = all(bool2(_122.x == uint4(100u, 200u, 250u, 425u).xy.x, _122.y == uint4(100u, 200u, 250u, 425u).xy.y));
    }
    else
    {
        _131 = false;
    }
    bool _141 = false;
    if (_131)
    {
        uint3 _134 = clamp(_45.xyz, uint3(100u, 0u, 0u), uint3(300u, 400u, 250u));
        _141 = all(bool3(_134.x == uint4(100u, 200u, 250u, 425u).xyz.x, _134.y == uint4(100u, 200u, 250u, 425u).xyz.y, _134.z == uint4(100u, 200u, 250u, 425u).xyz.z));
    }
    else
    {
        _141 = false;
    }
    bool _150 = false;
    if (_141)
    {
        uint4 _144 = clamp(_45, uint4(100u, 0u, 0u, 300u), uint4(300u, 400u, 250u, 500u));
        _150 = all(bool4(_144.x == uint4(100u, 200u, 250u, 425u).x, _144.y == uint4(100u, 200u, 250u, 425u).y, _144.z == uint4(100u, 200u, 250u, 425u).z, _144.w == uint4(100u, 200u, 250u, 425u).w));
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
        _159 = all(bool2(uint2(100u, 200u).x == uint4(100u, 200u, 250u, 425u).xy.x, uint2(100u, 200u).y == uint4(100u, 200u, 250u, 425u).xy.y));
    }
    else
    {
        _159 = false;
    }
    bool _166 = false;
    if (_159)
    {
        _166 = all(bool3(uint3(100u, 200u, 250u).x == uint4(100u, 200u, 250u, 425u).xyz.x, uint3(100u, 200u, 250u).y == uint4(100u, 200u, 250u, 425u).xyz.y, uint3(100u, 200u, 250u).z == uint4(100u, 200u, 250u, 425u).xyz.z));
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
        _170 = _7_colorGreen;
    }
    else
    {
        _170 = _7_colorRed;
    }
    return _170;
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
