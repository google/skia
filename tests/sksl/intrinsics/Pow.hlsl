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
    float4 expected = float4(-1.5625f, 0.0f, 0.75f, 3.375f);
    bool _52 = false;
    if (pow(_7_testInputs.x, 2.0f) == (-1.5625f))
    {
        float2 _42 = pow(_7_testInputs.xy, float2(2.0f, 3.0f));
        _52 = all(bool2(_42.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.x, _42.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _66 = false;
    if (_52)
    {
        float3 _55 = pow(_7_testInputs.xyz, float3(2.0f, 3.0f, 1.0f));
        _66 = all(bool3(_55.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.x, _55.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.y, _55.z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float4 _69 = pow(_7_testInputs, float4(2.0f, 3.0f, 1.0f, 1.5f));
        _77 = all(bool4(_69.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).x, _69.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).y, _69.z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).z, _69.w == float4(-1.5625f, 0.0f, 0.75f, 3.375f).w));
    }
    else
    {
        _77 = false;
    }
    bool _82 = false;
    if (_77)
    {
        _82 = 1.5625f == (-1.5625f);
    }
    else
    {
        _82 = false;
    }
    bool _89 = false;
    if (_82)
    {
        _89 = all(bool2(float2(1.5625f, 0.0f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.x, float2(1.5625f, 0.0f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.y));
    }
    else
    {
        _89 = false;
    }
    bool _96 = false;
    if (_89)
    {
        _96 = all(bool3(float3(1.5625f, 0.0f, 0.75f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.x, float3(1.5625f, 0.0f, 0.75f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.y, float3(1.5625f, 0.0f, 0.75f).z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.z));
    }
    else
    {
        _96 = false;
    }
    bool _102 = false;
    if (_96)
    {
        _102 = all(bool4(float4(1.5625f, 0.0f, 0.75f, 3.375f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).x, float4(1.5625f, 0.0f, 0.75f, 3.375f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).y, float4(1.5625f, 0.0f, 0.75f, 3.375f).z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).z, float4(1.5625f, 0.0f, 0.75f, 3.375f).w == float4(-1.5625f, 0.0f, 0.75f, 3.375f).w));
    }
    else
    {
        _102 = false;
    }
    float4 _103 = 0.0f.xxxx;
    if (_102)
    {
        _103 = _7_colorGreen;
    }
    else
    {
        _103 = _7_colorRed;
    }
    return _103;
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
