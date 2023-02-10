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
    bool _99 = false;
    if (_86)
    {
        _99 = lerp(_10_colorBlack.x, _10_colorWhite.x, 0.5f) == 0.5f;
    }
    else
    {
        _99 = false;
    }
    bool _114 = false;
    if (_99)
    {
        float2 _102 = lerp(_10_colorBlack.xy, _10_colorWhite.xy, 0.5f.xx);
        _114 = all(bool2(_102.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.x, _102.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.y));
    }
    else
    {
        _114 = false;
    }
    bool _130 = false;
    if (_114)
    {
        float3 _117 = lerp(_10_colorBlack.xyz, _10_colorWhite.xyz, 0.5f.xxx);
        _130 = all(bool3(_117.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.x, _117.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.y, _117.z == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.z));
    }
    else
    {
        _130 = false;
    }
    bool _141 = false;
    if (_130)
    {
        float4 _133 = lerp(_10_colorBlack, _10_colorWhite, 0.5f.xxxx);
        _141 = all(bool4(_133.x == float4(0.5f, 0.5f, 0.5f, 1.0f).x, _133.y == float4(0.5f, 0.5f, 0.5f, 1.0f).y, _133.z == float4(0.5f, 0.5f, 0.5f, 1.0f).z, _133.w == float4(0.5f, 0.5f, 0.5f, 1.0f).w));
    }
    else
    {
        _141 = false;
    }
    bool _145 = false;
    if (_141)
    {
        _145 = true;
    }
    else
    {
        _145 = false;
    }
    bool _151 = false;
    if (_145)
    {
        _151 = all(bool2(0.5f.xx.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.x, 0.5f.xx.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.y));
    }
    else
    {
        _151 = false;
    }
    bool _157 = false;
    if (_151)
    {
        _157 = all(bool3(0.5f.xxx.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.x, 0.5f.xxx.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.y, 0.5f.xxx.z == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.z));
    }
    else
    {
        _157 = false;
    }
    bool _160 = false;
    if (_157)
    {
        _160 = true;
    }
    else
    {
        _160 = false;
    }
    bool _172 = false;
    if (_160)
    {
        _172 = lerp(_10_colorWhite.x, _10_testInputs.x, 0.0f) == 1.0f;
    }
    else
    {
        _172 = false;
    }
    bool _186 = false;
    if (_172)
    {
        float2 _175 = lerp(_10_colorWhite.xy, _10_testInputs.xy, float2(0.0f, 0.5f));
        _186 = all(bool2(_175.x == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.x, _175.y == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.y));
    }
    else
    {
        _186 = false;
    }
    bool _200 = false;
    if (_186)
    {
        float3 _189 = lerp(_10_colorWhite.xyz, _10_testInputs.xyz, float3(0.0f, 0.5f, 0.0f));
        _200 = all(bool3(_189.x == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.x, _189.y == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.y, _189.z == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.z));
    }
    else
    {
        _200 = false;
    }
    bool _211 = false;
    if (_200)
    {
        float4 _203 = lerp(_10_colorWhite, _10_testInputs, float4(0.0f, 0.5f, 0.0f, 1.0f));
        _211 = all(bool4(_203.x == float4(1.0f, 0.5f, 1.0f, 2.25f).x, _203.y == float4(1.0f, 0.5f, 1.0f, 2.25f).y, _203.z == float4(1.0f, 0.5f, 1.0f, 2.25f).z, _203.w == float4(1.0f, 0.5f, 1.0f, 2.25f).w));
    }
    else
    {
        _211 = false;
    }
    bool _214 = false;
    if (_211)
    {
        _214 = true;
    }
    else
    {
        _214 = false;
    }
    bool _221 = false;
    if (_214)
    {
        _221 = all(bool2(float2(1.0f, 0.5f).x == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.x, float2(1.0f, 0.5f).y == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.y));
    }
    else
    {
        _221 = false;
    }
    bool _228 = false;
    if (_221)
    {
        _228 = all(bool3(float3(1.0f, 0.5f, 1.0f).x == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.x, float3(1.0f, 0.5f, 1.0f).y == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.y, float3(1.0f, 0.5f, 1.0f).z == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.z));
    }
    else
    {
        _228 = false;
    }
    bool _231 = false;
    if (_228)
    {
        _231 = true;
    }
    else
    {
        _231 = false;
    }
    float4 _232 = 0.0f.xxxx;
    if (_231)
    {
        _232 = _10_colorGreen;
    }
    else
    {
        _232 = _10_colorRed;
    }
    return _232;
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
