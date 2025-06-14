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
    float4 expected = float4(0.0f, 0.0f, 0.75f, 1.0f);
    bool _52 = false;
    if (clamp(_11_testInputs.x, 0.0f, 1.0f) == 0.0f)
    {
        float2 _43 = clamp(_11_testInputs.xy, 0.0f.xx, 1.0f.xx);
        _52 = all(bool2(_43.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.x, _43.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _66 = false;
    if (_52)
    {
        float3 _55 = clamp(_11_testInputs.xyz, 0.0f.xxx, 1.0f.xxx);
        _66 = all(bool3(_55.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.x, _55.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.y, _55.z == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float4 _69 = clamp(_11_testInputs, 0.0f.xxxx, 1.0f.xxxx);
        _77 = all(bool4(_69.x == float4(0.0f, 0.0f, 0.75f, 1.0f).x, _69.y == float4(0.0f, 0.0f, 0.75f, 1.0f).y, _69.z == float4(0.0f, 0.0f, 0.75f, 1.0f).z, _69.w == float4(0.0f, 0.0f, 0.75f, 1.0f).w));
    }
    else
    {
        _77 = false;
    }
    bool _81 = false;
    if (_77)
    {
        _81 = true;
    }
    else
    {
        _81 = false;
    }
    bool _87 = false;
    if (_81)
    {
        _87 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _87 = false;
    }
    bool _94 = false;
    if (_87)
    {
        _94 = all(bool3(float3(0.0f, 0.0f, 0.75f).x == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.75f).y == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.75f).z == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _94 = false;
    }
    bool _97 = false;
    if (_94)
    {
        _97 = true;
    }
    else
    {
        _97 = false;
    }
    float4 _98 = 0.0f.xxxx;
    if (_97)
    {
        _98 = _11_colorGreen;
    }
    else
    {
        _98 = _11_colorRed;
    }
    return _98;
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
