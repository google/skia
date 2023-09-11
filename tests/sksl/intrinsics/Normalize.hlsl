cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_inputVal : packoffset(c0);
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
    float4 expectedVec = float4(1.0f, 0.0f, 0.0f, 0.0f);
    bool _47 = false;
    if (sign(_7_inputVal.x) == 1.0f)
    {
        float2 _39 = normalize(_7_inputVal.xy);
        _47 = all(bool2(_39.x == float4(1.0f, 0.0f, 0.0f, 0.0f).xy.x, _39.y == float4(1.0f, 0.0f, 0.0f, 0.0f).xy.y));
    }
    else
    {
        _47 = false;
    }
    bool _59 = false;
    if (_47)
    {
        float3 _50 = normalize(_7_inputVal.xyz);
        _59 = all(bool3(_50.x == float4(1.0f, 0.0f, 0.0f, 0.0f).xyz.x, _50.y == float4(1.0f, 0.0f, 0.0f, 0.0f).xyz.y, _50.z == float4(1.0f, 0.0f, 0.0f, 0.0f).xyz.z));
    }
    else
    {
        _59 = false;
    }
    bool _68 = false;
    if (_59)
    {
        float4 _62 = normalize(_7_inputVal);
        _68 = all(bool4(_62.x == float4(1.0f, 0.0f, 0.0f, 0.0f).x, _62.y == float4(1.0f, 0.0f, 0.0f, 0.0f).y, _62.z == float4(1.0f, 0.0f, 0.0f, 0.0f).z, _62.w == float4(1.0f, 0.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _68 = false;
    }
    bool _72 = false;
    if (_68)
    {
        _72 = true;
    }
    else
    {
        _72 = false;
    }
    bool _79 = false;
    if (_72)
    {
        _79 = all(bool2(float2(0.0f, 1.0f).x == float4(1.0f, 0.0f, 0.0f, 0.0f).yx.x, float2(0.0f, 1.0f).y == float4(1.0f, 0.0f, 0.0f, 0.0f).yx.y));
    }
    else
    {
        _79 = false;
    }
    bool _86 = false;
    if (_79)
    {
        _86 = all(bool3(float3(0.0f, 1.0f, 0.0f).x == float4(1.0f, 0.0f, 0.0f, 0.0f).zxy.x, float3(0.0f, 1.0f, 0.0f).y == float4(1.0f, 0.0f, 0.0f, 0.0f).zxy.y, float3(0.0f, 1.0f, 0.0f).z == float4(1.0f, 0.0f, 0.0f, 0.0f).zxy.z));
    }
    else
    {
        _86 = false;
    }
    bool _89 = false;
    if (_86)
    {
        _89 = true;
    }
    else
    {
        _89 = false;
    }
    float4 _90 = 0.0f.xxxx;
    if (_89)
    {
        _90 = _7_colorGreen;
    }
    else
    {
        _90 = _7_colorRed;
    }
    return _90;
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
