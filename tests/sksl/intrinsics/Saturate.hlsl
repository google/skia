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
    float4 expected = float4(0.0f, 0.0f, 0.75f, 1.0f);
    bool _51 = false;
    if (clamp(_10_testInputs.x, 0.0f, 1.0f) == 0.0f)
    {
        float2 _42 = clamp(_10_testInputs.xy, 0.0f.xx, 1.0f.xx);
        _51 = all(bool2(_42.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.x, _42.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _51 = false;
    }
    bool _65 = false;
    if (_51)
    {
        float3 _54 = clamp(_10_testInputs.xyz, 0.0f.xxx, 1.0f.xxx);
        _65 = all(bool3(_54.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.x, _54.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.y, _54.z == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _65 = false;
    }
    bool _76 = false;
    if (_65)
    {
        float4 _68 = clamp(_10_testInputs, 0.0f.xxxx, 1.0f.xxxx);
        _76 = all(bool4(_68.x == float4(0.0f, 0.0f, 0.75f, 1.0f).x, _68.y == float4(0.0f, 0.0f, 0.75f, 1.0f).y, _68.z == float4(0.0f, 0.0f, 0.75f, 1.0f).z, _68.w == float4(0.0f, 0.0f, 0.75f, 1.0f).w));
    }
    else
    {
        _76 = false;
    }
    bool _80 = false;
    if (_76)
    {
        _80 = true;
    }
    else
    {
        _80 = false;
    }
    bool _86 = false;
    if (_80)
    {
        _86 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _86 = false;
    }
    bool _93 = false;
    if (_86)
    {
        _93 = all(bool3(float3(0.0f, 0.0f, 0.75f).x == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.75f).y == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.75f).z == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _93 = false;
    }
    bool _96 = false;
    if (_93)
    {
        _96 = true;
    }
    else
    {
        _96 = false;
    }
    float4 _97 = 0.0f.xxxx;
    if (_96)
    {
        _97 = _10_colorGreen;
    }
    else
    {
        _97 = _10_colorRed;
    }
    return _97;
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
