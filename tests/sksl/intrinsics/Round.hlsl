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
    bool _46 = false;
    if (round(_10_testInputs.x) == (-1.0f))
    {
        float2 _38 = round(_10_testInputs.xy);
        _46 = all(bool2(_38.x == float2(-1.0f, 0.0f).x, _38.y == float2(-1.0f, 0.0f).y));
    }
    else
    {
        _46 = false;
    }
    bool _59 = false;
    if (_46)
    {
        float3 _49 = round(_10_testInputs.xyz);
        _59 = all(bool3(_49.x == float3(-1.0f, 0.0f, 1.0f).x, _49.y == float3(-1.0f, 0.0f, 1.0f).y, _49.z == float3(-1.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _59 = false;
    }
    bool _70 = false;
    if (_59)
    {
        float4 _62 = round(_10_testInputs);
        _70 = all(bool4(_62.x == float4(-1.0f, 0.0f, 1.0f, 2.0f).x, _62.y == float4(-1.0f, 0.0f, 1.0f, 2.0f).y, _62.z == float4(-1.0f, 0.0f, 1.0f, 2.0f).z, _62.w == float4(-1.0f, 0.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _70 = false;
    }
    bool4 _71 = _70.xxxx;
    return float4(_71.x ? _10_colorGreen.x : _10_colorRed.x, _71.y ? _10_colorGreen.y : _10_colorRed.y, _71.z ? _10_colorGreen.z : _10_colorRed.z, _71.w ? _10_colorGreen.w : _10_colorRed.w);
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
