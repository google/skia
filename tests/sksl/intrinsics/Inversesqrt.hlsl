cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_inputVal : packoffset(c0);
    float4 _7_expected : packoffset(c1);
    float4 _7_colorGreen : packoffset(c2);
    float4 _7_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool _49 = false;
    if (rsqrt(_7_inputVal.x) == _7_expected.x)
    {
        float2 _39 = rsqrt(_7_inputVal.xy);
        _49 = all(bool2(_39.x == _7_expected.xy.x, _39.y == _7_expected.xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _63 = false;
    if (_49)
    {
        float3 _52 = rsqrt(_7_inputVal.xyz);
        _63 = all(bool3(_52.x == _7_expected.xyz.x, _52.y == _7_expected.xyz.y, _52.z == _7_expected.xyz.z));
    }
    else
    {
        _63 = false;
    }
    bool _74 = false;
    if (_63)
    {
        float4 _66 = rsqrt(_7_inputVal);
        _74 = all(bool4(_66.x == _7_expected.x, _66.y == _7_expected.y, _66.z == _7_expected.z, _66.w == _7_expected.w));
    }
    else
    {
        _74 = false;
    }
    bool _82 = false;
    if (_74)
    {
        _82 = 1.0f == _7_expected.x;
    }
    else
    {
        _82 = false;
    }
    bool _92 = false;
    if (_82)
    {
        _92 = all(bool2(float2(1.0f, 0.5f).x == _7_expected.xy.x, float2(1.0f, 0.5f).y == _7_expected.xy.y));
    }
    else
    {
        _92 = false;
    }
    bool _102 = false;
    if (_92)
    {
        _102 = all(bool3(float3(1.0f, 0.5f, 0.25f).x == _7_expected.xyz.x, float3(1.0f, 0.5f, 0.25f).y == _7_expected.xyz.y, float3(1.0f, 0.5f, 0.25f).z == _7_expected.xyz.z));
    }
    else
    {
        _102 = false;
    }
    bool _111 = false;
    if (_102)
    {
        _111 = all(bool4(float4(1.0f, 0.5f, 0.25f, 0.125f).x == _7_expected.x, float4(1.0f, 0.5f, 0.25f, 0.125f).y == _7_expected.y, float4(1.0f, 0.5f, 0.25f, 0.125f).z == _7_expected.z, float4(1.0f, 0.5f, 0.25f, 0.125f).w == _7_expected.w));
    }
    else
    {
        _111 = false;
    }
    bool _120 = false;
    if (_111)
    {
        _120 = rsqrt(-1.0f) == _7_expected.x;
    }
    else
    {
        _120 = false;
    }
    bool _131 = false;
    if (_120)
    {
        float2 _123 = rsqrt(float2(-1.0f, -4.0f));
        _131 = all(bool2(_123.x == _7_expected.xy.x, _123.y == _7_expected.xy.y));
    }
    else
    {
        _131 = false;
    }
    bool _142 = false;
    if (_131)
    {
        float3 _134 = rsqrt(float3(-1.0f, -4.0f, -16.0f));
        _142 = all(bool3(_134.x == _7_expected.xyz.x, _134.y == _7_expected.xyz.y, _134.z == _7_expected.xyz.z));
    }
    else
    {
        _142 = false;
    }
    bool _152 = false;
    if (_142)
    {
        float4 _145 = rsqrt(float4(-1.0f, -4.0f, -16.0f, -64.0f));
        _152 = all(bool4(_145.x == _7_expected.x, _145.y == _7_expected.y, _145.z == _7_expected.z, _145.w == _7_expected.w));
    }
    else
    {
        _152 = false;
    }
    float4 _153 = 0.0f.xxxx;
    if (_152)
    {
        _153 = _7_colorGreen;
    }
    else
    {
        _153 = _7_colorRed;
    }
    return _153;
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
