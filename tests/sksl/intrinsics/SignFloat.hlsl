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
    float4 expected = float4(-1.0f, 0.0f, 1.0f, 1.0f);
    bool _50 = false;
    if (sign(_10_testInputs.x) == (-1.0f))
    {
        float2 _42 = sign(_10_testInputs.xy);
        _50 = all(bool2(_42.x == float4(-1.0f, 0.0f, 1.0f, 1.0f).xy.x, _42.y == float4(-1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _50 = false;
    }
    bool _62 = false;
    if (_50)
    {
        float3 _53 = sign(_10_testInputs.xyz);
        _62 = all(bool3(_53.x == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.x, _53.y == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.y, _53.z == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _62 = false;
    }
    bool _71 = false;
    if (_62)
    {
        float4 _65 = sign(_10_testInputs);
        _71 = all(bool4(_65.x == float4(-1.0f, 0.0f, 1.0f, 1.0f).x, _65.y == float4(-1.0f, 0.0f, 1.0f, 1.0f).y, _65.z == float4(-1.0f, 0.0f, 1.0f, 1.0f).z, _65.w == float4(-1.0f, 0.0f, 1.0f, 1.0f).w));
    }
    else
    {
        _71 = false;
    }
    bool _75 = false;
    if (_71)
    {
        _75 = true;
    }
    else
    {
        _75 = false;
    }
    bool _82 = false;
    if (_75)
    {
        _82 = all(bool2(float2(-1.0f, 0.0f).x == float4(-1.0f, 0.0f, 1.0f, 1.0f).xy.x, float2(-1.0f, 0.0f).y == float4(-1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _82 = false;
    }
    bool _89 = false;
    if (_82)
    {
        _89 = all(bool3(float3(-1.0f, 0.0f, 1.0f).x == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.x, float3(-1.0f, 0.0f, 1.0f).y == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.y, float3(-1.0f, 0.0f, 1.0f).z == float4(-1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _89 = false;
    }
    bool _92 = false;
    if (_89)
    {
        _92 = true;
    }
    else
    {
        _92 = false;
    }
    float4 _93 = 0.0f.xxxx;
    if (_92)
    {
        _93 = _10_colorGreen;
    }
    else
    {
        _93 = _10_colorRed;
    }
    return _93;
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
