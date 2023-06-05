cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputVal : packoffset(c0);
    float4 _10_expected : packoffset(c1);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool _51 = false;
    if (log(_10_inputVal.x + sqrt(_10_inputVal.x * _10_inputVal.x - 1.0f)) == _10_expected.x)
    {
        float2 _41 = log(_10_inputVal.xy + sqrt(_10_inputVal.xy * _10_inputVal.xy - 1.0f));
        _51 = all(bool2(_41.x == _10_expected.xy.x, _41.y == _10_expected.xy.y));
    }
    else
    {
        _51 = false;
    }
    bool _65 = false;
    if (_51)
    {
        float3 _54 = log(_10_inputVal.xyz + sqrt(_10_inputVal.xyz * _10_inputVal.xyz - 1.0f));
        _65 = all(bool3(_54.x == _10_expected.xyz.x, _54.y == _10_expected.xyz.y, _54.z == _10_expected.xyz.z));
    }
    else
    {
        _65 = false;
    }
    bool _76 = false;
    if (_65)
    {
        float4 _68 = log(_10_inputVal + sqrt(_10_inputVal * _10_inputVal - 1.0f));
        _76 = all(bool4(_68.x == _10_expected.x, _68.y == _10_expected.y, _68.z == _10_expected.z, _68.w == _10_expected.w));
    }
    else
    {
        _76 = false;
    }
    bool _83 = false;
    if (_76)
    {
        _83 = 0.0f == _10_expected.x;
    }
    else
    {
        _83 = false;
    }
    bool _91 = false;
    if (_83)
    {
        _91 = all(bool2(0.0f.xx.x == _10_expected.xy.x, 0.0f.xx.y == _10_expected.xy.y));
    }
    else
    {
        _91 = false;
    }
    bool _101 = false;
    if (_91)
    {
        _101 = all(bool3(float3(0.0f, 0.0f, 1.0f).x == _10_expected.xyz.x, float3(0.0f, 0.0f, 1.0f).y == _10_expected.xyz.y, float3(0.0f, 0.0f, 1.0f).z == _10_expected.xyz.z));
    }
    else
    {
        _101 = false;
    }
    bool _110 = false;
    if (_101)
    {
        _110 = all(bool4(float4(0.0f, 0.0f, 1.0f, 2.0f).x == _10_expected.x, float4(0.0f, 0.0f, 1.0f, 2.0f).y == _10_expected.y, float4(0.0f, 0.0f, 1.0f, 2.0f).z == _10_expected.z, float4(0.0f, 0.0f, 1.0f, 2.0f).w == _10_expected.w));
    }
    else
    {
        _110 = false;
    }
    float4 _111 = 0.0f.xxxx;
    if (_110)
    {
        _111 = _10_colorGreen;
    }
    else
    {
        _111 = _10_colorRed;
    }
    return _111;
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
