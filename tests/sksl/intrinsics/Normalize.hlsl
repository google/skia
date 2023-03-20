cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputVal : packoffset(c0);
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
    float4 expectedVec = float4(1.0f, 0.0f, 0.0f, 0.0f);
    bool _49 = false;
    if (sign(_10_inputVal.x) == 1.0f)
    {
        float2 _41 = normalize(_10_inputVal.xy);
        _49 = all(bool2(_41.x == float4(1.0f, 0.0f, 0.0f, 0.0f).xy.x, _41.y == float4(1.0f, 0.0f, 0.0f, 0.0f).xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _61 = false;
    if (_49)
    {
        float3 _52 = normalize(_10_inputVal.xyz);
        _61 = all(bool3(_52.x == float4(1.0f, 0.0f, 0.0f, 0.0f).xyz.x, _52.y == float4(1.0f, 0.0f, 0.0f, 0.0f).xyz.y, _52.z == float4(1.0f, 0.0f, 0.0f, 0.0f).xyz.z));
    }
    else
    {
        _61 = false;
    }
    bool _70 = false;
    if (_61)
    {
        float4 _64 = normalize(_10_inputVal);
        _70 = all(bool4(_64.x == float4(1.0f, 0.0f, 0.0f, 0.0f).x, _64.y == float4(1.0f, 0.0f, 0.0f, 0.0f).y, _64.z == float4(1.0f, 0.0f, 0.0f, 0.0f).z, _64.w == float4(1.0f, 0.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _70 = false;
    }
    bool _74 = false;
    if (_70)
    {
        _74 = true;
    }
    else
    {
        _74 = false;
    }
    bool _81 = false;
    if (_74)
    {
        _81 = all(bool2(float2(0.0f, 1.0f).x == float4(1.0f, 0.0f, 0.0f, 0.0f).yx.x, float2(0.0f, 1.0f).y == float4(1.0f, 0.0f, 0.0f, 0.0f).yx.y));
    }
    else
    {
        _81 = false;
    }
    bool _88 = false;
    if (_81)
    {
        _88 = all(bool3(float3(0.0f, 1.0f, 0.0f).x == float4(1.0f, 0.0f, 0.0f, 0.0f).zxy.x, float3(0.0f, 1.0f, 0.0f).y == float4(1.0f, 0.0f, 0.0f, 0.0f).zxy.y, float3(0.0f, 1.0f, 0.0f).z == float4(1.0f, 0.0f, 0.0f, 0.0f).zxy.z));
    }
    else
    {
        _88 = false;
    }
    bool _91 = false;
    if (_88)
    {
        _91 = true;
    }
    else
    {
        _91 = false;
    }
    float4 _92 = 0.0f.xxxx;
    if (_91)
    {
        _92 = _10_colorGreen;
    }
    else
    {
        _92 = _10_colorRed;
    }
    return _92;
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
