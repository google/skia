cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_inputVal : packoffset(c0);
    float4 _11_expected : packoffset(c1);
    float4 _11_colorGreen : packoffset(c2);
    float4 _11_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool _52 = false;
    if (exp2(_11_inputVal.x) == _11_expected.x)
    {
        float2 _42 = exp2(_11_inputVal.xy);
        _52 = all(bool2(_42.x == _11_expected.xy.x, _42.y == _11_expected.xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _66 = false;
    if (_52)
    {
        float3 _55 = exp2(_11_inputVal.xyz);
        _66 = all(bool3(_55.x == _11_expected.xyz.x, _55.y == _11_expected.xyz.y, _55.z == _11_expected.xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float4 _69 = exp2(_11_inputVal);
        _77 = all(bool4(_69.x == _11_expected.x, _69.y == _11_expected.y, _69.z == _11_expected.z, _69.w == _11_expected.w));
    }
    else
    {
        _77 = false;
    }
    bool _85 = false;
    if (_77)
    {
        _85 = 1.0f == _11_expected.x;
    }
    else
    {
        _85 = false;
    }
    bool _95 = false;
    if (_85)
    {
        _95 = all(bool2(float2(1.0f, 2.0f).x == _11_expected.xy.x, float2(1.0f, 2.0f).y == _11_expected.xy.y));
    }
    else
    {
        _95 = false;
    }
    bool _105 = false;
    if (_95)
    {
        _105 = all(bool3(float3(1.0f, 2.0f, 4.0f).x == _11_expected.xyz.x, float3(1.0f, 2.0f, 4.0f).y == _11_expected.xyz.y, float3(1.0f, 2.0f, 4.0f).z == _11_expected.xyz.z));
    }
    else
    {
        _105 = false;
    }
    bool _114 = false;
    if (_105)
    {
        _114 = all(bool4(float4(1.0f, 2.0f, 4.0f, 8.0f).x == _11_expected.x, float4(1.0f, 2.0f, 4.0f, 8.0f).y == _11_expected.y, float4(1.0f, 2.0f, 4.0f, 8.0f).z == _11_expected.z, float4(1.0f, 2.0f, 4.0f, 8.0f).w == _11_expected.w));
    }
    else
    {
        _114 = false;
    }
    float4 _115 = 0.0f.xxxx;
    if (_114)
    {
        _115 = _11_colorGreen;
    }
    else
    {
        _115 = _11_colorRed;
    }
    return _115;
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
