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
    if (log(_11_inputVal.x + sqrt(_11_inputVal.x * _11_inputVal.x + 1.0f)) == _11_expected.x)
    {
        float2 _42 = log(_11_inputVal.xy + sqrt(_11_inputVal.xy * _11_inputVal.xy + 1.0f));
        float2 _42 = log(_11_inputVal.xy + sqrt(_11_inputVal.xy * _11_inputVal.xy + 1.0f));
        _52 = all(bool2(_42.x == _11_expected.xy.x, _42.y == _11_expected.xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _66 = false;
    if (_52)
    {
        float3 _55 = log(_11_inputVal.xyz + sqrt(_11_inputVal.xyz * _11_inputVal.xyz + 1.0f));
        float3 _55 = log(_11_inputVal.xyz + sqrt(_11_inputVal.xyz * _11_inputVal.xyz + 1.0f));
        _66 = all(bool3(_55.x == _11_expected.xyz.x, _55.y == _11_expected.xyz.y, _55.z == _11_expected.xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float4 _69 = log(_11_inputVal + sqrt(_11_inputVal * _11_inputVal + 1.0f));
        float4 _69 = log(_11_inputVal + sqrt(_11_inputVal * _11_inputVal + 1.0f));
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
    bool _102 = false;
    if (_92)
    {
        _102 = all(bool3(float3(0.0f, 0.0f, 1.0f).x == _11_expected.xyz.x, float3(0.0f, 0.0f, 1.0f).y == _11_expected.xyz.y, float3(0.0f, 0.0f, 1.0f).z == _11_expected.xyz.z));
    }
    else
    {
        _102 = false;
    }
    bool _111 = false;
    if (_102)
    {
        _111 = all(bool4(float4(0.0f, 0.0f, 1.0f, -1.0f).x == _11_expected.x, float4(0.0f, 0.0f, 1.0f, -1.0f).y == _11_expected.y, float4(0.0f, 0.0f, 1.0f, -1.0f).z == _11_expected.z, float4(0.0f, 0.0f, 1.0f, -1.0f).w == _11_expected.w));
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
