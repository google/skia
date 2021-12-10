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
    float4 negativeVal = float4(-1.0f, -4.0f, -16.0f, -64.0f);
    bool _58 = false;
    if (sqrt(_10_inputVal.x) == _10_expected.x)
    {
        float2 _48 = sqrt(_10_inputVal.xy);
        _58 = all(bool2(_48.x == _10_expected.xy.x, _48.y == _10_expected.xy.y));
    }
    else
    {
        _58 = false;
    }
    bool _72 = false;
    if (_58)
    {
        float3 _61 = sqrt(_10_inputVal.xyz);
        _72 = all(bool3(_61.x == _10_expected.xyz.x, _61.y == _10_expected.xyz.y, _61.z == _10_expected.xyz.z));
    }
    else
    {
        _72 = false;
    }
    bool _83 = false;
    if (_72)
    {
        float4 _75 = sqrt(_10_inputVal);
        _83 = all(bool4(_75.x == _10_expected.x, _75.y == _10_expected.y, _75.z == _10_expected.z, _75.w == _10_expected.w));
    }
    else
    {
        _83 = false;
    }
    bool _91 = false;
    if (_83)
    {
        _91 = 1.0f == _10_expected.x;
    }
    else
    {
        _91 = false;
    }
    bool _101 = false;
    if (_91)
    {
        _101 = all(bool2(float2(1.0f, 2.0f).x == _10_expected.xy.x, float2(1.0f, 2.0f).y == _10_expected.xy.y));
    }
    else
    {
        _101 = false;
    }
    bool _111 = false;
    if (_101)
    {
        _111 = all(bool3(float3(1.0f, 2.0f, 4.0f).x == _10_expected.xyz.x, float3(1.0f, 2.0f, 4.0f).y == _10_expected.xyz.y, float3(1.0f, 2.0f, 4.0f).z == _10_expected.xyz.z));
    }
    else
    {
        _111 = false;
    }
    bool _120 = false;
    if (_111)
    {
        _120 = all(bool4(float4(1.0f, 2.0f, 4.0f, 8.0f).x == _10_expected.x, float4(1.0f, 2.0f, 4.0f, 8.0f).y == _10_expected.y, float4(1.0f, 2.0f, 4.0f, 8.0f).z == _10_expected.z, float4(1.0f, 2.0f, 4.0f, 8.0f).w == _10_expected.w));
    }
    else
    {
        _120 = false;
    }
    bool _128 = false;
    if (_120)
    {
        _128 = sqrt(-1.0f) == _10_expected.x;
    }
    else
    {
        _128 = false;
    }
    bool _138 = false;
    if (_128)
    {
        float2 _131 = sqrt(float2(-1.0f, -4.0f));
        _138 = all(bool2(_131.x == _10_expected.xy.x, _131.y == _10_expected.xy.y));
    }
    else
    {
        _138 = false;
    }
    bool _148 = false;
    if (_138)
    {
        float3 _141 = sqrt(float3(-1.0f, -4.0f, -16.0f));
        _148 = all(bool3(_141.x == _10_expected.xyz.x, _141.y == _10_expected.xyz.y, _141.z == _10_expected.xyz.z));
    }
    else
    {
        _148 = false;
    }
    bool _157 = false;
    if (_148)
    {
        float4 _151 = sqrt(negativeVal);
        _157 = all(bool4(_151.x == _10_expected.x, _151.y == _10_expected.y, _151.z == _10_expected.z, _151.w == _10_expected.w));
    }
    else
    {
        _157 = false;
    }
    float4 _158 = 0.0f.xxxx;
    if (_157)
    {
        _158 = _10_colorGreen;
    }
    else
    {
        _158 = _10_colorRed;
    }
    return _158;
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
