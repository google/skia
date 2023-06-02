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
    if (rsqrt(_10_inputVal.x) == _10_expected.x)
    {
        float2 _41 = rsqrt(_10_inputVal.xy);
        _51 = all(bool2(_41.x == _10_expected.xy.x, _41.y == _10_expected.xy.y));
    }
    else
    {
        _51 = false;
    }
    bool _65 = false;
    if (_51)
    {
        float3 _54 = rsqrt(_10_inputVal.xyz);
        _65 = all(bool3(_54.x == _10_expected.xyz.x, _54.y == _10_expected.xyz.y, _54.z == _10_expected.xyz.z));
    }
    else
    {
        _65 = false;
    }
    bool _76 = false;
    if (_65)
    {
        float4 _68 = rsqrt(_10_inputVal);
        _76 = all(bool4(_68.x == _10_expected.x, _68.y == _10_expected.y, _68.z == _10_expected.z, _68.w == _10_expected.w));
    }
    else
    {
        _76 = false;
    }
    bool _84 = false;
    if (_76)
    {
        _84 = 1.0f == _10_expected.x;
    }
    else
    {
        _84 = false;
    }
    bool _94 = false;
    if (_84)
    {
        _94 = all(bool2(float2(1.0f, 0.5f).x == _10_expected.xy.x, float2(1.0f, 0.5f).y == _10_expected.xy.y));
    }
    else
    {
        _94 = false;
    }
    bool _104 = false;
    if (_94)
    {
        _104 = all(bool3(float3(1.0f, 0.5f, 0.25f).x == _10_expected.xyz.x, float3(1.0f, 0.5f, 0.25f).y == _10_expected.xyz.y, float3(1.0f, 0.5f, 0.25f).z == _10_expected.xyz.z));
    }
    else
    {
        _104 = false;
    }
    bool _113 = false;
    if (_104)
    {
        _113 = all(bool4(float4(1.0f, 0.5f, 0.25f, 0.125f).x == _10_expected.x, float4(1.0f, 0.5f, 0.25f, 0.125f).y == _10_expected.y, float4(1.0f, 0.5f, 0.25f, 0.125f).z == _10_expected.z, float4(1.0f, 0.5f, 0.25f, 0.125f).w == _10_expected.w));
    }
    else
    {
        _113 = false;
    }
    bool _122 = false;
    if (_113)
    {
        _122 = rsqrt(-1.0f) == _10_expected.x;
    }
    else
    {
        _122 = false;
    }
    bool _133 = false;
    if (_122)
    {
        float2 _125 = rsqrt(float2(-1.0f, -4.0f));
        _133 = all(bool2(_125.x == _10_expected.xy.x, _125.y == _10_expected.xy.y));
    }
    else
    {
        _133 = false;
    }
    bool _144 = false;
    if (_133)
    {
        float3 _136 = rsqrt(float3(-1.0f, -4.0f, -16.0f));
        _144 = all(bool3(_136.x == _10_expected.xyz.x, _136.y == _10_expected.xyz.y, _136.z == _10_expected.xyz.z));
    }
    else
    {
        _144 = false;
    }
    bool _154 = false;
    if (_144)
    {
        float4 _147 = rsqrt(float4(-1.0f, -4.0f, -16.0f, -64.0f));
        _154 = all(bool4(_147.x == _10_expected.x, _147.y == _10_expected.y, _147.z == _10_expected.z, _147.w == _10_expected.w));
    }
    else
    {
        _154 = false;
    }
    float4 _155 = 0.0f.xxxx;
    if (_154)
    {
        _155 = _10_colorGreen;
    }
    else
    {
        _155 = _10_colorRed;
    }
    return _155;
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
