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
    uint4 expectedA = uint4(125u, 80u, 80u, 225u);
    uint4 expectedB = uint4(125u, 100u, 75u, 225u);
    uint _72 = _46.x;
    bool _84 = false;
    if (max(_72, 80u) == 125u)
    {
        uint2 _76 = max(_46.xy, uint2(80u, 80u));
        _84 = all(bool2(_76.x == uint4(125u, 80u, 80u, 225u).xy.x, _76.y == uint4(125u, 80u, 80u, 225u).xy.y));
    }
    else
    {
        _84 = false;
    }
    bool _95 = false;
    if (_84)
    {
        uint3 _87 = max(_46.xyz, uint3(80u, 80u, 80u));
        _95 = all(bool3(_87.x == uint4(125u, 80u, 80u, 225u).xyz.x, _87.y == uint4(125u, 80u, 80u, 225u).xyz.y, _87.z == uint4(125u, 80u, 80u, 225u).xyz.z));
    }
    else
    {
        _95 = false;
    }
    bool _103 = false;
    if (_95)
    {
        uint4 _98 = max(_46, uint4(80u, 80u, 80u, 80u));
        _103 = all(bool4(_98.x == uint4(125u, 80u, 80u, 225u).x, _98.y == uint4(125u, 80u, 80u, 225u).y, _98.z == uint4(125u, 80u, 80u, 225u).z, _98.w == uint4(125u, 80u, 80u, 225u).w));
    }
    else
    {
        _103 = false;
    }
    bool _107 = false;
    if (_103)
    {
        _107 = true;
    }
    else
    {
        _107 = false;
    }
    bool _114 = false;
    if (_107)
    {
        _114 = all(bool2(uint2(125u, 80u).x == uint4(125u, 80u, 80u, 225u).xy.x, uint2(125u, 80u).y == uint4(125u, 80u, 80u, 225u).xy.y));
    }
    else
    {
        _114 = false;
    }
    bool _121 = false;
    if (_114)
    {
        _121 = all(bool3(uint3(125u, 80u, 80u).x == uint4(125u, 80u, 80u, 225u).xyz.x, uint3(125u, 80u, 80u).y == uint4(125u, 80u, 80u, 225u).xyz.y, uint3(125u, 80u, 80u).z == uint4(125u, 80u, 80u, 225u).xyz.z));
    }
    else
    {
        _121 = false;
    }
    bool _124 = false;
    if (_121)
    {
        _124 = true;
    }
    else
    {
        _124 = false;
    }
    bool _130 = false;
    if (_124)
    {
        _130 = max(_72, _60.x) == 125u;
    }
    else
    {
        _130 = false;
    }
    bool _139 = false;
    if (_130)
    {
        uint2 _133 = max(_46.xy, _60.xy);
        _139 = all(bool2(_133.x == uint4(125u, 100u, 75u, 225u).xy.x, _133.y == uint4(125u, 100u, 75u, 225u).xy.y));
    }
    else
    {
        _139 = false;
    }
    bool _148 = false;
    if (_139)
    {
        uint3 _142 = max(_46.xyz, _60.xyz);
        _148 = all(bool3(_142.x == uint4(125u, 100u, 75u, 225u).xyz.x, _142.y == uint4(125u, 100u, 75u, 225u).xyz.y, _142.z == uint4(125u, 100u, 75u, 225u).xyz.z));
    }
    else
    {
        _148 = false;
    }
    bool _154 = false;
    if (_148)
    {
        uint4 _151 = max(_46, _60);
        _154 = all(bool4(_151.x == uint4(125u, 100u, 75u, 225u).x, _151.y == uint4(125u, 100u, 75u, 225u).y, _151.z == uint4(125u, 100u, 75u, 225u).z, _151.w == uint4(125u, 100u, 75u, 225u).w));
    }
    else
    {
        _154 = false;
    }
    bool _157 = false;
    if (_154)
    {
        _157 = true;
    }
    else
    {
        _157 = false;
    }
    bool _164 = false;
    if (_157)
    {
        _164 = all(bool2(uint2(125u, 100u).x == uint4(125u, 100u, 75u, 225u).xy.x, uint2(125u, 100u).y == uint4(125u, 100u, 75u, 225u).xy.y));
    }
    else
    {
        _164 = false;
    }
    bool _171 = false;
    if (_164)
    {
        _171 = all(bool3(uint3(125u, 100u, 75u).x == uint4(125u, 100u, 75u, 225u).xyz.x, uint3(125u, 100u, 75u).y == uint4(125u, 100u, 75u, 225u).xyz.y, uint3(125u, 100u, 75u).z == uint4(125u, 100u, 75u, 225u).xyz.z));
    }
    else
    {
        _171 = false;
    }
    bool _174 = false;
    if (_171)
    {
        _174 = true;
    }
    else
    {
        _174 = false;
    }
    float4 _175 = 0.0f.xxxx;
    if (_174)
    {
        _175 = _10_colorGreen;
    }
    else
    {
        _175 = _10_colorRed;
    }
    return _175;
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
