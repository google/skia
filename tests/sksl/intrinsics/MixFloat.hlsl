cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float4 _10_colorBlack : packoffset(c2);
    float4 _10_colorWhite : packoffset(c3);
    float4 _10_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 expectedBW = float4(0.5f, 0.5f, 0.5f, 1.0f);
    float4 expectedWT = float4(1.0f, 0.5f, 1.0f, 2.25f);
    float4 _35 = lerp(_10_colorGreen, _10_colorRed, 0.0f.xxxx);
    bool _62 = false;
    if (all(bool4(_35.x == float4(0.0f, 1.0f, 0.0f, 1.0f).x, _35.y == float4(0.0f, 1.0f, 0.0f, 1.0f).y, _35.z == float4(0.0f, 1.0f, 0.0f, 1.0f).z, _35.w == float4(0.0f, 1.0f, 0.0f, 1.0f).w)))
    {
        float4 _51 = lerp(_10_colorGreen, _10_colorRed, 0.25f.xxxx);
        _62 = all(bool4(_51.x == float4(0.25f, 0.75f, 0.0f, 1.0f).x, _51.y == float4(0.25f, 0.75f, 0.0f, 1.0f).y, _51.z == float4(0.25f, 0.75f, 0.0f, 1.0f).z, _51.w == float4(0.25f, 0.75f, 0.0f, 1.0f).w));
    }
    else
    {
        _62 = false;
    }
    bool _74 = false;
    if (_62)
    {
        float4 _65 = lerp(_10_colorGreen, _10_colorRed, 0.75f.xxxx);
        _74 = all(bool4(_65.x == float4(0.75f, 0.25f, 0.0f, 1.0f).x, _65.y == float4(0.75f, 0.25f, 0.0f, 1.0f).y, _65.z == float4(0.75f, 0.25f, 0.0f, 1.0f).z, _65.w == float4(0.75f, 0.25f, 0.0f, 1.0f).w));
    }
    else
    {
        _74 = false;
    }
    bool _86 = false;
    if (_74)
    {
        float4 _77 = lerp(_10_colorGreen, _10_colorRed, 1.0f.xxxx);
        _86 = all(bool4(_77.x == float4(1.0f, 0.0f, 0.0f, 1.0f).x, _77.y == float4(1.0f, 0.0f, 0.0f, 1.0f).y, _77.z == float4(1.0f, 0.0f, 0.0f, 1.0f).z, _77.w == float4(1.0f, 0.0f, 0.0f, 1.0f).w));
    }
    else
    {
        _86 = false;
    }
    bool _101 = false;
    if (_86)
    {
        _101 = lerp(_10_colorBlack.x, _10_colorWhite.x, 0.5f) == expectedBW.x;
    }
    else
    {
        _101 = false;
    }
    bool _117 = false;
    if (_101)
    {
        float2 _104 = lerp(_10_colorBlack.xy, _10_colorWhite.xy, 0.5f.xx);
        _117 = all(bool2(_104.x == expectedBW.xy.x, _104.y == expectedBW.xy.y));
    }
    else
    {
        _117 = false;
    }
    bool _134 = false;
    if (_117)
    {
        float3 _120 = lerp(_10_colorBlack.xyz, _10_colorWhite.xyz, 0.5f.xxx);
        _134 = all(bool3(_120.x == expectedBW.xyz.x, _120.y == expectedBW.xyz.y, _120.z == expectedBW.xyz.z));
    }
    else
    {
        _134 = false;
    }
    bool _146 = false;
    if (_134)
    {
        float4 _137 = lerp(_10_colorBlack, _10_colorWhite, 0.5f.xxxx);
        _146 = all(bool4(_137.x == expectedBW.x, _137.y == expectedBW.y, _137.z == expectedBW.z, _137.w == expectedBW.w));
    }
    else
    {
        _146 = false;
    }
    bool _152 = false;
    if (_146)
    {
        _152 = 0.5f == expectedBW.x;
    }
    else
    {
        _152 = false;
    }
    bool _160 = false;
    if (_152)
    {
        _160 = all(bool2(0.5f.xx.x == expectedBW.xy.x, 0.5f.xx.y == expectedBW.xy.y));
    }
    else
    {
        _160 = false;
    }
    bool _168 = false;
    if (_160)
    {
        _168 = all(bool3(0.5f.xxx.x == expectedBW.xyz.x, 0.5f.xxx.y == expectedBW.xyz.y, 0.5f.xxx.z == expectedBW.xyz.z));
    }
    else
    {
        _168 = false;
    }
    bool _174 = false;
    if (_168)
    {
        _174 = all(bool4(float4(0.5f, 0.5f, 0.5f, 1.0f).x == expectedBW.x, float4(0.5f, 0.5f, 0.5f, 1.0f).y == expectedBW.y, float4(0.5f, 0.5f, 0.5f, 1.0f).z == expectedBW.z, float4(0.5f, 0.5f, 0.5f, 1.0f).w == expectedBW.w));
    }
    else
    {
        _174 = false;
    }
    bool _188 = false;
    if (_174)
    {
        _188 = lerp(_10_colorWhite.x, _10_testInputs.x, 0.0f) == expectedWT.x;
    }
    else
    {
        _188 = false;
    }
    bool _203 = false;
    if (_188)
    {
        float2 _191 = lerp(_10_colorWhite.xy, _10_testInputs.xy, float2(0.0f, 0.5f));
        _203 = all(bool2(_191.x == expectedWT.xy.x, _191.y == expectedWT.xy.y));
    }
    else
    {
        _203 = false;
    }
    bool _218 = false;
    if (_203)
    {
        float3 _206 = lerp(_10_colorWhite.xyz, _10_testInputs.xyz, float3(0.0f, 0.5f, 0.0f));
        _218 = all(bool3(_206.x == expectedWT.xyz.x, _206.y == expectedWT.xyz.y, _206.z == expectedWT.xyz.z));
    }
    else
    {
        _218 = false;
    }
    bool _230 = false;
    if (_218)
    {
        float4 _221 = lerp(_10_colorWhite, _10_testInputs, float4(0.0f, 0.5f, 0.0f, 1.0f));
        _230 = all(bool4(_221.x == expectedWT.x, _221.y == expectedWT.y, _221.z == expectedWT.z, _221.w == expectedWT.w));
    }
    else
    {
        _230 = false;
    }
    bool _236 = false;
    if (_230)
    {
        _236 = 1.0f == expectedWT.x;
    }
    else
    {
        _236 = false;
    }
    bool _244 = false;
    if (_236)
    {
        _244 = all(bool2(float2(1.0f, 0.5f).x == expectedWT.xy.x, float2(1.0f, 0.5f).y == expectedWT.xy.y));
    }
    else
    {
        _244 = false;
    }
    bool _252 = false;
    if (_244)
    {
        _252 = all(bool3(float3(1.0f, 0.5f, 1.0f).x == expectedWT.xyz.x, float3(1.0f, 0.5f, 1.0f).y == expectedWT.xyz.y, float3(1.0f, 0.5f, 1.0f).z == expectedWT.xyz.z));
    }
    else
    {
        _252 = false;
    }
    bool _258 = false;
    if (_252)
    {
        _258 = all(bool4(float4(1.0f, 0.5f, 1.0f, 2.25f).x == expectedWT.x, float4(1.0f, 0.5f, 1.0f, 2.25f).y == expectedWT.y, float4(1.0f, 0.5f, 1.0f, 2.25f).z == expectedWT.z, float4(1.0f, 0.5f, 1.0f, 2.25f).w == expectedWT.w));
    }
    else
    {
        _258 = false;
    }
    float4 _259 = 0.0f.xxxx;
    if (_258)
    {
        _259 = _10_colorGreen;
    }
    else
    {
        _259 = _10_colorRed;
    }
    return _259;
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
