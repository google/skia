cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    float4 _11_colorBlack : packoffset(c2);
    float4 _11_colorWhite : packoffset(c3);
    float4 _11_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 expectedBW = float4(0.5f, 0.5f, 0.5f, 1.0f);
    float4 expectedWT = float4(1.0f, 0.5f, 1.0f, 2.25f);
    float4 _37 = lerp(_11_colorGreen, _11_colorRed, 0.0f.xxxx);
    bool _63 = false;
    if (all(bool4(_37.x == float4(0.0f, 1.0f, 0.0f, 1.0f).x, _37.y == float4(0.0f, 1.0f, 0.0f, 1.0f).y, _37.z == float4(0.0f, 1.0f, 0.0f, 1.0f).z, _37.w == float4(0.0f, 1.0f, 0.0f, 1.0f).w)))
    {
        float4 _52 = lerp(_11_colorGreen, _11_colorRed, 0.25f.xxxx);
        _63 = all(bool4(_52.x == float4(0.25f, 0.75f, 0.0f, 1.0f).x, _52.y == float4(0.25f, 0.75f, 0.0f, 1.0f).y, _52.z == float4(0.25f, 0.75f, 0.0f, 1.0f).z, _52.w == float4(0.25f, 0.75f, 0.0f, 1.0f).w));
    }
    else
    {
        _63 = false;
    }
    bool _75 = false;
    if (_63)
    {
        float4 _66 = lerp(_11_colorGreen, _11_colorRed, 0.75f.xxxx);
        _75 = all(bool4(_66.x == float4(0.75f, 0.25f, 0.0f, 1.0f).x, _66.y == float4(0.75f, 0.25f, 0.0f, 1.0f).y, _66.z == float4(0.75f, 0.25f, 0.0f, 1.0f).z, _66.w == float4(0.75f, 0.25f, 0.0f, 1.0f).w));
    }
    else
    {
        _75 = false;
    }
    bool _87 = false;
    if (_75)
    {
        float4 _78 = lerp(_11_colorGreen, _11_colorRed, 1.0f.xxxx);
        _87 = all(bool4(_78.x == float4(1.0f, 0.0f, 0.0f, 1.0f).x, _78.y == float4(1.0f, 0.0f, 0.0f, 1.0f).y, _78.z == float4(1.0f, 0.0f, 0.0f, 1.0f).z, _78.w == float4(1.0f, 0.0f, 0.0f, 1.0f).w));
    }
    else
    {
        _87 = false;
    }
    bool _100 = false;
    if (_87)
    {
        _100 = lerp(_11_colorBlack.x, _11_colorWhite.x, 0.5f) == 0.5f;
    }
    else
    {
        _100 = false;
    }
    bool _115 = false;
    if (_100)
    {
        float2 _103 = lerp(_11_colorBlack.xy, _11_colorWhite.xy, 0.5f.xx);
        _115 = all(bool2(_103.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.x, _103.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.y));
    }
    else
    {
        _115 = false;
    }
    bool _131 = false;
    if (_115)
    {
        float3 _118 = lerp(_11_colorBlack.xyz, _11_colorWhite.xyz, 0.5f.xxx);
        _131 = all(bool3(_118.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.x, _118.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.y, _118.z == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.z));
    }
    else
    {
        _131 = false;
    }
    bool _142 = false;
    if (_131)
    {
        float4 _134 = lerp(_11_colorBlack, _11_colorWhite, 0.5f.xxxx);
        _142 = all(bool4(_134.x == float4(0.5f, 0.5f, 0.5f, 1.0f).x, _134.y == float4(0.5f, 0.5f, 0.5f, 1.0f).y, _134.z == float4(0.5f, 0.5f, 0.5f, 1.0f).z, _134.w == float4(0.5f, 0.5f, 0.5f, 1.0f).w));
    }
    else
    {
        _142 = false;
    }
    bool _146 = false;
    if (_142)
    {
        _146 = true;
    }
    else
    {
        _146 = false;
    }
    bool _152 = false;
    if (_146)
    {
        _152 = all(bool2(0.5f.xx.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.x, 0.5f.xx.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.y));
    }
    else
    {
        _152 = false;
    }
    bool _158 = false;
    if (_152)
    {
        _158 = all(bool3(0.5f.xxx.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.x, 0.5f.xxx.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.y, 0.5f.xxx.z == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.z));
    }
    else
    {
        _158 = false;
    }
    bool _161 = false;
    if (_158)
    {
        _161 = true;
    }
    else
    {
        _161 = false;
    }
    bool _173 = false;
    if (_161)
    {
        _173 = lerp(_11_colorWhite.x, _11_testInputs.x, 0.0f) == 1.0f;
    }
    else
    {
        _173 = false;
    }
    bool _187 = false;
    if (_173)
    {
        float2 _176 = lerp(_11_colorWhite.xy, _11_testInputs.xy, float2(0.0f, 0.5f));
        _187 = all(bool2(_176.x == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.x, _176.y == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.y));
    }
    else
    {
        _187 = false;
    }
    bool _201 = false;
    if (_187)
    {
        float3 _190 = lerp(_11_colorWhite.xyz, _11_testInputs.xyz, float3(0.0f, 0.5f, 0.0f));
        _201 = all(bool3(_190.x == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.x, _190.y == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.y, _190.z == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.z));
    }
    else
    {
        _201 = false;
    }
    bool _212 = false;
    if (_201)
    {
        float4 _204 = lerp(_11_colorWhite, _11_testInputs, float4(0.0f, 0.5f, 0.0f, 1.0f));
        _212 = all(bool4(_204.x == float4(1.0f, 0.5f, 1.0f, 2.25f).x, _204.y == float4(1.0f, 0.5f, 1.0f, 2.25f).y, _204.z == float4(1.0f, 0.5f, 1.0f, 2.25f).z, _204.w == float4(1.0f, 0.5f, 1.0f, 2.25f).w));
    }
    else
    {
        _212 = false;
    }
    bool _215 = false;
    if (_212)
    {
        _215 = true;
    }
    else
    {
        _215 = false;
    }
    bool _222 = false;
    if (_215)
    {
        _222 = all(bool2(float2(1.0f, 0.5f).x == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.x, float2(1.0f, 0.5f).y == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.y));
    }
    else
    {
        _222 = false;
    }
    bool _229 = false;
    if (_222)
    {
        _229 = all(bool3(float3(1.0f, 0.5f, 1.0f).x == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.x, float3(1.0f, 0.5f, 1.0f).y == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.y, float3(1.0f, 0.5f, 1.0f).z == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.z));
    }
    else
    {
        _229 = false;
    }
    bool _232 = false;
    if (_229)
    {
        _232 = true;
    }
    else
    {
        _232 = false;
    }
    float4 _233 = 0.0f.xxxx;
    if (_232)
    {
        _233 = _11_colorGreen;
    }
    else
    {
        _233 = _11_colorRed;
    }
    return _233;
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
