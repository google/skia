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
    if (atan(_11_inputVal.x) == _11_expected.x)
    {
        float2 _42 = atan(_11_inputVal.xy);
        _52 = all(bool2(_42.x == _11_expected.xy.x, _42.y == _11_expected.xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _66 = false;
    if (_52)
    {
        float3 _55 = atan(_11_inputVal.xyz);
        _66 = all(bool3(_55.x == _11_expected.xyz.x, _55.y == _11_expected.xyz.y, _55.z == _11_expected.xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _77 = false;
    if (_66)
    {
        float4 _69 = atan(_11_inputVal);
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
    bool _121 = false;
    if (_109)
    {
        _121 = atan2(_11_inputVal.x, 1.0f) == _11_expected.x;
    }
    else
    {
        _121 = false;
    }
    bool _134 = false;
    if (_121)
    {
        float2 _124 = atan2(_11_inputVal.xy, 1.0f.xx);
        _134 = all(bool2(_124.x == _11_expected.xy.x, _124.y == _11_expected.xy.y));
    }
    else
    {
        _134 = false;
    }
    bool _147 = false;
    if (_134)
    {
        float3 _137 = atan2(_11_inputVal.xyz, 1.0f.xxx);
        _147 = all(bool3(_137.x == _11_expected.xyz.x, _137.y == _11_expected.xyz.y, _137.z == _11_expected.xyz.z));
    }
    else
    {
        _147 = false;
    }
    bool _158 = false;
    if (_147)
    {
        float4 _150 = atan2(_11_inputVal, 1.0f.xxxx);
        _158 = all(bool4(_150.x == _11_expected.x, _150.y == _11_expected.y, _150.z == _11_expected.z, _150.w == _11_expected.w));
    }
    else
    {
        _158 = false;
    }
    bool _165 = false;
    if (_158)
    {
        _165 = 0.0f == _11_expected.x;
    }
    else
    {
        _165 = false;
    }
    bool _173 = false;
    if (_165)
    {
        _173 = all(bool2(0.0f.xx.x == _11_expected.xy.x, 0.0f.xx.y == _11_expected.xy.y));
    }
    else
    {
        _173 = false;
    }
    bool _181 = false;
    if (_173)
    {
        _181 = all(bool3(0.0f.xxx.x == _11_expected.xyz.x, 0.0f.xxx.y == _11_expected.xyz.y, 0.0f.xxx.z == _11_expected.xyz.z));
    }
    else
    {
        _181 = false;
    }
    bool _188 = false;
    if (_181)
    {
        _188 = all(bool4(0.0f.xxxx.x == _11_expected.x, 0.0f.xxxx.y == _11_expected.y, 0.0f.xxxx.z == _11_expected.z, 0.0f.xxxx.w == _11_expected.w));
    }
    else
    {
        _188 = false;
    }
    float4 _189 = 0.0f.xxxx;
    if (_188)
    {
        _189 = _11_colorGreen;
    }
    else
    {
        _189 = _11_colorRed;
    }
    return _189;
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
