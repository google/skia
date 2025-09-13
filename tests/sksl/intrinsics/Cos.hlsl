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
    if (cos(_11_inputVal.x) == _11_expected.x)
    {
        float2 _42 = cos(_11_inputVal.xy);
        _52 = all(bool2(_42.x == _11_expected.xy.x, _42.y == _11_expected.xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _66 = false;
    if (_52)
    {
        float3 _55 = cos(_11_inputVal.xyz);
        _66 = all(bool3(_55.x == _11_expected.xyz.x, _55.y == _11_expected.xyz.y, _55.z == _11_expected.xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float4 _69 = cos(_11_inputVal);
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
    bool _94 = false;
    if (_85)
    {
        _94 = all(bool2(1.0f.xx.x == _11_expected.xy.x, 1.0f.xx.y == _11_expected.xy.y));
    }
    else
    {
        _94 = false;
    }
    bool _103 = false;
    if (_94)
    {
        _103 = all(bool3(1.0f.xxx.x == _11_expected.xyz.x, 1.0f.xxx.y == _11_expected.xyz.y, 1.0f.xxx.z == _11_expected.xyz.z));
    }
    else
    {
        _103 = false;
    }
    bool _111 = false;
    if (_103)
    {
        _111 = all(bool4(1.0f.xxxx.x == _11_expected.x, 1.0f.xxxx.y == _11_expected.y, 1.0f.xxxx.z == _11_expected.z, 1.0f.xxxx.w == _11_expected.w));
    }
    else
    {
        _111 = false;
    }
    float4 _112 = 0.0f.xxxx;
    if (_111)
    {
        _112 = _11_colorGreen;
    }
    else
    {
        _112 = _11_colorRed;
    }
    return _112;
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
