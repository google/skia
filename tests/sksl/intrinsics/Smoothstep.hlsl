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
    float4 expectedA = float4(0.0f, 0.0f, 0.84375f, 1.0f);
    float4 expectedB = float4(1.0f, 0.0f, 1.0f, 1.0f);
    bool _43 = false;
    if (true)
    {
        _43 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _43 = false;
    }
    bool _52 = false;
    if (_43)
    {
        _52 = all(bool3(float3(0.0f, 0.0f, 0.84375f).x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.84375f).y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.84375f).z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _52 = false;
    }
    bool _55 = false;
    if (_52)
    {
        _55 = true;
    }
    else
    {
        _55 = false;
    }
    bool _58 = false;
    if (_55)
    {
        _58 = true;
    }
    else
    {
        _58 = false;
    }
    bool _64 = false;
    if (_58)
    {
        _64 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _64 = false;
    }
    bool _70 = false;
    if (_64)
    {
        _70 = all(bool3(float3(0.0f, 0.0f, 0.84375f).x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.84375f).y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.84375f).z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _70 = false;
    }
    bool _73 = false;
    if (_70)
    {
        _73 = true;
    }
    else
    {
        _73 = false;
    }
    bool _88 = false;
    if (_73)
    {
        _88 = smoothstep(_11_colorRed.y, _11_colorGreen.y, -1.25f) == 0.0f;
    }
    else
    {
        _88 = false;
    }
    bool _104 = false;
    if (_88)
    {
        float2 _91 = smoothstep(_11_colorRed.y.xx, _11_colorGreen.y.xx, float2(-1.25f, 0.0f));
        _104 = all(bool2(_91.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, _91.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _104 = false;
    }
    bool _121 = false;
    if (_104)
    {
        float3 _107 = smoothstep(_11_colorRed.y.xxx, _11_colorGreen.y.xxx, float3(-1.25f, 0.0f, 0.75f));
        _121 = all(bool3(_107.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, _107.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, _107.z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _121 = false;
    }
    bool _138 = false;
    if (_121)
    {
        float4 _124 = smoothstep(_11_colorRed.y.xxxx, _11_colorGreen.y.xxxx, float4(-1.25f, 0.0f, 0.75f, 2.25f));
        _138 = all(bool4(_124.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).x, _124.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).y, _124.z == float4(0.0f, 0.0f, 0.84375f, 1.0f).z, _124.w == float4(0.0f, 0.0f, 0.84375f, 1.0f).w));
    }
    else
    {
        _138 = false;
    }
    bool _141 = false;
    if (_138)
    {
        _141 = true;
    }
    else
    {
        _141 = false;
    }
    bool _148 = false;
    if (_141)
    {
        _148 = all(bool2(float2(1.0f, 0.0f).x == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.x, float2(1.0f, 0.0f).y == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _148 = false;
    }
    bool _155 = false;
    if (_148)
    {
        _155 = all(bool3(float3(1.0f, 0.0f, 1.0f).x == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.x, float3(1.0f, 0.0f, 1.0f).y == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.y, float3(1.0f, 0.0f, 1.0f).z == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
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
    bool _169 = false;
    if (_158)
    {
        _169 = smoothstep(_11_colorRed.x, _11_colorGreen.x, -1.25f) == 1.0f;
    }
    else
    {
        _169 = false;
    }
    bool _182 = false;
    if (_169)
    {
        float2 _172 = smoothstep(_11_colorRed.xy, _11_colorGreen.xy, float2(-1.25f, 0.0f));
        _182 = all(bool2(_172.x == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.x, _172.y == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _182 = false;
    }
    bool _195 = false;
    if (_182)
    {
        float3 _185 = smoothstep(_11_colorRed.xyz, _11_colorGreen.xyz, float3(-1.25f, 0.0f, 0.75f));
        _195 = all(bool3(_185.x == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.x, _185.y == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.y, _185.z == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _195 = false;
    }
    bool _205 = false;
    if (_195)
    {
        float4 _198 = smoothstep(_11_colorRed, _11_colorGreen, float4(-1.25f, 0.0f, 0.75f, 2.25f));
        _205 = all(bool4(_198.x == float4(1.0f, 0.0f, 1.0f, 1.0f).x, _198.y == float4(1.0f, 0.0f, 1.0f, 1.0f).y, _198.z == float4(1.0f, 0.0f, 1.0f, 1.0f).z, _198.w == float4(1.0f, 0.0f, 1.0f, 1.0f).w));
    }
    else
    {
        _205 = false;
    }
    float4 _206 = 0.0f.xxxx;
    if (_205)
    {
        _206 = _11_colorGreen;
    }
    else
    {
        _206 = _11_colorRed;
    }
    return _206;
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
