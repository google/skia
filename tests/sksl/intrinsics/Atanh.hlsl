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
    if ((log((1.0f + _11_inputVal.x) / (1.0f - _11_inputVal.x)) * 0.5f) == _11_expected.x)
    {
        float2 _42 = log((1.0f + _11_inputVal.xy) / (1.0f - _11_inputVal.xy)) * 0.5f;
        _52 = all(bool2(_42.x == _11_expected.xy.x, _42.y == _11_expected.xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _66 = false;
    if (_52)
    {
        float3 _55 = log((1.0f + _11_inputVal.xyz) / (1.0f - _11_inputVal.xyz)) * 0.5f;
        _66 = all(bool3(_55.x == _11_expected.xyz.x, _55.y == _11_expected.xyz.y, _55.z == _11_expected.xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float4 _69 = log((1.0f + _11_inputVal) / (1.0f - _11_inputVal)) * 0.5f;
        _77 = all(bool4(_69.x == _11_expected.x, _69.y == _11_expected.y, _69.z == _11_expected.z, _69.w == _11_expected.w));
    }
    else
    {
        _77 = false;
    }
    bool _84 = false;
    if (_77)
    {
        _84 = 0.0f == _11_expected.x;
    }
    else
    {
        _84 = false;
    }
    bool _94 = false;
    if (_84)
    {
        _94 = all(bool2(float2(0.0f, 0.25f).x == _11_expected.xy.x, float2(0.0f, 0.25f).y == _11_expected.xy.y));
    }
    else
    {
        _94 = false;
    }
    bool _104 = false;
    if (_94)
    {
        _104 = all(bool3(float3(0.0f, 0.25f, 0.5f).x == _11_expected.xyz.x, float3(0.0f, 0.25f, 0.5f).y == _11_expected.xyz.y, float3(0.0f, 0.25f, 0.5f).z == _11_expected.xyz.z));
    }
    else
    {
        _104 = false;
    }
    bool _113 = false;
    if (_104)
    {
        _113 = all(bool4(float4(0.0f, 0.25f, 0.5f, 1.0f).x == _11_expected.x, float4(0.0f, 0.25f, 0.5f, 1.0f).y == _11_expected.y, float4(0.0f, 0.25f, 0.5f, 1.0f).z == _11_expected.z, float4(0.0f, 0.25f, 0.5f, 1.0f).w == _11_expected.w));
    }
    else
    {
        _113 = false;
    }
    float4 _114 = 0.0f.xxxx;
    if (_113)
    {
        _114 = _11_colorGreen;
    }
    else
    {
        _114 = _11_colorRed;
    }
    return _114;
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
