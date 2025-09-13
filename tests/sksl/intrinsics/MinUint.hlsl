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
    uint4 expectedA = uint4(50u, 0u, 50u, 50u);
    uint4 expectedB = uint4(0u, 0u, 0u, 100u);
    uint _71 = _46.x;
    bool _83 = false;
    if (min(_71, 50u) == 50u)
    {
        uint2 _75 = min(_46.xy, uint2(50u, 50u));
        _83 = all(bool2(_75.x == uint4(50u, 0u, 50u, 50u).xy.x, _75.y == uint4(50u, 0u, 50u, 50u).xy.y));
    }
    else
    {
        _83 = false;
    }
    bool _94 = false;
    if (_83)
    {
        uint3 _86 = min(_46.xyz, uint3(50u, 50u, 50u));
        _94 = all(bool3(_86.x == uint4(50u, 0u, 50u, 50u).xyz.x, _86.y == uint4(50u, 0u, 50u, 50u).xyz.y, _86.z == uint4(50u, 0u, 50u, 50u).xyz.z));
    }
    else
    {
        _94 = false;
    }
    bool _102 = false;
    if (_94)
    {
        uint4 _97 = min(_46, uint4(50u, 50u, 50u, 50u));
        _102 = all(bool4(_97.x == uint4(50u, 0u, 50u, 50u).x, _97.y == uint4(50u, 0u, 50u, 50u).y, _97.z == uint4(50u, 0u, 50u, 50u).z, _97.w == uint4(50u, 0u, 50u, 50u).w));
    }
    else
    {
        _102 = false;
    }
    bool _106 = false;
    if (_102)
    {
        _106 = true;
    }
    else
    {
        _106 = false;
    }
    bool _113 = false;
    if (_106)
    {
        _113 = all(bool2(uint2(50u, 0u).x == uint4(50u, 0u, 50u, 50u).xy.x, uint2(50u, 0u).y == uint4(50u, 0u, 50u, 50u).xy.y));
    }
    else
    {
        _113 = false;
    }
    bool _120 = false;
    if (_113)
    {
        _120 = all(bool3(uint3(50u, 0u, 50u).x == uint4(50u, 0u, 50u, 50u).xyz.x, uint3(50u, 0u, 50u).y == uint4(50u, 0u, 50u, 50u).xyz.y, uint3(50u, 0u, 50u).z == uint4(50u, 0u, 50u, 50u).xyz.z));
    }
    else
    {
        _120 = false;
    }
    bool _123 = false;
    if (_120)
    {
        _123 = true;
    }
    else
    {
        _123 = false;
    }
    bool _129 = false;
    if (_123)
    {
        _129 = min(_71, _60.x) == 0u;
    }
    else
    {
        _129 = false;
    }
    bool _138 = false;
    if (_129)
    {
        uint2 _132 = min(_46.xy, _60.xy);
        _138 = all(bool2(_132.x == uint4(0u, 0u, 0u, 100u).xy.x, _132.y == uint4(0u, 0u, 0u, 100u).xy.y));
    }
    else
    {
        _138 = false;
    }
    bool _147 = false;
    if (_138)
    {
        uint3 _141 = min(_46.xyz, _60.xyz);
        _147 = all(bool3(_141.x == uint4(0u, 0u, 0u, 100u).xyz.x, _141.y == uint4(0u, 0u, 0u, 100u).xyz.y, _141.z == uint4(0u, 0u, 0u, 100u).xyz.z));
    }
    else
    {
        _147 = false;
    }
    bool _153 = false;
    if (_147)
    {
        uint4 _150 = min(_46, _60);
        _153 = all(bool4(_150.x == uint4(0u, 0u, 0u, 100u).x, _150.y == uint4(0u, 0u, 0u, 100u).y, _150.z == uint4(0u, 0u, 0u, 100u).z, _150.w == uint4(0u, 0u, 0u, 100u).w));
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
    bool _163 = false;
    if (_156)
    {
        _163 = all(bool2(uint2(0u, 0u).x == uint4(0u, 0u, 0u, 100u).xy.x, uint2(0u, 0u).y == uint4(0u, 0u, 0u, 100u).xy.y));
    }
    else
    {
        _163 = false;
    }
    bool _170 = false;
    if (_163)
    {
        _170 = all(bool3(uint3(0u, 0u, 0u).x == uint4(0u, 0u, 0u, 100u).xyz.x, uint3(0u, 0u, 0u).y == uint4(0u, 0u, 0u, 100u).xyz.y, uint3(0u, 0u, 0u).z == uint4(0u, 0u, 0u, 100u).xyz.z));
    }
    else
    {
        _170 = false;
    }
    bool _173 = false;
    if (_170)
    {
        _173 = true;
    }
    else
    {
        _173 = false;
    }
    float4 _174 = 0.0f.xxxx;
    if (_173)
    {
        _174 = _11_colorGreen;
    }
    else
    {
        _174 = _11_colorRed;
    }
    return _174;
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
