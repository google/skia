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
    float4 expected = float4(-1.0f, 0.0f, 1.0f, 1.0f);
    bool _48 = false;
    if (sign(_7_testInputs.x) == (-1.0f))
    {
        float2 _40 = sign(_7_testInputs.xy);
        _48 = all(bool2(_40.x == float4(-1.0f, 0.0f, 1.0f, 1.0f).xy.x, _40.y == float4(-1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _48 = false;
    }
    bool _60 = false;
    if (_48)
    {
        float3 _51 = sign(_7_testInputs.xyz);
        _60 = all(bool3(_51.x == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.x, _51.y == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.y, _51.z == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _60 = false;
    }
    bool _69 = false;
    if (_60)
    {
        float4 _63 = sign(_7_testInputs);
        _69 = all(bool4(_63.x == float4(-1.0f, 0.0f, 1.0f, 1.0f).x, _63.y == float4(-1.0f, 0.0f, 1.0f, 1.0f).y, _63.z == float4(-1.0f, 0.0f, 1.0f, 1.0f).z, _63.w == float4(-1.0f, 0.0f, 1.0f, 1.0f).w));
    }
    else
    {
        _69 = false;
    }
    bool _73 = false;
    if (_69)
    {
        _73 = true;
    }
    else
    {
        _73 = false;
    }
    bool _80 = false;
    if (_73)
    {
        _80 = all(bool2(float2(-1.0f, 0.0f).x == float4(-1.0f, 0.0f, 1.0f, 1.0f).xy.x, float2(-1.0f, 0.0f).y == float4(-1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _80 = false;
    }
    bool _87 = false;
    if (_80)
    {
        _87 = all(bool3(float3(-1.0f, 0.0f, 1.0f).x == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.x, float3(-1.0f, 0.0f, 1.0f).y == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.y, float3(-1.0f, 0.0f, 1.0f).z == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _87 = false;
    }
    bool _90 = false;
    if (_87)
    {
        _90 = true;
    }
    else
    {
        _90 = false;
    }
    float4 _91 = 0.0f.xxxx;
    if (_90)
    {
        _91 = _7_colorGreen;
    }
    else
    {
        _91 = _7_colorRed;
    }
    return _91;
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
