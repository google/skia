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
    if (tanh(_11_inputVal.x) == _11_expected.x)
    {
        float2 _42 = tanh(_11_inputVal.xy);
        _52 = all(bool2(_42.x == _11_expected.xy.x, _42.y == _11_expected.xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _66 = false;
    if (_52)
    {
        float3 _55 = tanh(_11_inputVal.xyz);
        _66 = all(bool3(_55.x == _11_expected.xyz.x, _55.y == _11_expected.xyz.y, _55.z == _11_expected.xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float4 _69 = tanh(_11_inputVal);
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
    bool _92 = false;
    if (_84)
    {
        _92 = all(bool2(0.0f.xx.x == _11_expected.xy.x, 0.0f.xx.y == _11_expected.xy.y));
    }
    else
    {
        _92 = false;
    }
    bool _101 = false;
    if (_92)
    {
        _101 = all(bool3(0.0f.xxx.x == _11_expected.xyz.x, 0.0f.xxx.y == _11_expected.xyz.y, 0.0f.xxx.z == _11_expected.xyz.z));
    }
    else
    {
        _101 = false;
    }
    bool _109 = false;
    if (_101)
    {
        _109 = all(bool4(0.0f.xxxx.x == _11_expected.x, 0.0f.xxxx.y == _11_expected.y, 0.0f.xxxx.z == _11_expected.z, 0.0f.xxxx.w == _11_expected.w));
    }
    else
    {
        _109 = false;
    }
    float4 _110 = 0.0f.xxxx;
    if (_109)
    {
        _110 = _11_colorGreen;
    }
    else
    {
        _110 = _11_colorRed;
    }
    return _110;
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
