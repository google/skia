cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float4 _7_colorBlack : packoffset(c2);
    float4 _7_colorWhite : packoffset(c3);
    float4 _7_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 expectedBW = float4(0.5f, 0.5f, 0.5f, 1.0f);
    float4 expectedWT = float4(1.0f, 0.5f, 1.0f, 2.25f);
    float4 _33 = lerp(_7_colorGreen, _7_colorRed, 0.0f.xxxx);
    bool _60 = false;
    if (all(bool4(_33.x == float4(0.0f, 1.0f, 0.0f, 1.0f).x, _33.y == float4(0.0f, 1.0f, 0.0f, 1.0f).y, _33.z == float4(0.0f, 1.0f, 0.0f, 1.0f).z, _33.w == float4(0.0f, 1.0f, 0.0f, 1.0f).w)))
    {
        float4 _49 = lerp(_7_colorGreen, _7_colorRed, 0.25f.xxxx);
        _60 = all(bool4(_49.x == float4(0.25f, 0.75f, 0.0f, 1.0f).x, _49.y == float4(0.25f, 0.75f, 0.0f, 1.0f).y, _49.z == float4(0.25f, 0.75f, 0.0f, 1.0f).z, _49.w == float4(0.25f, 0.75f, 0.0f, 1.0f).w));
    }
    else
    {
        _60 = false;
    }
    bool _72 = false;
    if (_60)
    {
        float4 _63 = lerp(_7_colorGreen, _7_colorRed, 0.75f.xxxx);
        _72 = all(bool4(_63.x == float4(0.75f, 0.25f, 0.0f, 1.0f).x, _63.y == float4(0.75f, 0.25f, 0.0f, 1.0f).y, _63.z == float4(0.75f, 0.25f, 0.0f, 1.0f).z, _63.w == float4(0.75f, 0.25f, 0.0f, 1.0f).w));
    }
    else
    {
        _72 = false;
    }
    bool _84 = false;
    if (_72)
    {
        float4 _75 = lerp(_7_colorGreen, _7_colorRed, 1.0f.xxxx);
        _84 = all(bool4(_75.x == float4(1.0f, 0.0f, 0.0f, 1.0f).x, _75.y == float4(1.0f, 0.0f, 0.0f, 1.0f).y, _75.z == float4(1.0f, 0.0f, 0.0f, 1.0f).z, _75.w == float4(1.0f, 0.0f, 0.0f, 1.0f).w));
    }
    else
    {
        _84 = false;
    }
    bool _97 = false;
    if (_84)
    {
        _97 = lerp(_7_colorBlack.x, _7_colorWhite.x, 0.5f) == 0.5f;
    }
    else
    {
        _97 = false;
    }
    bool _112 = false;
    if (_97)
    {
        float2 _100 = lerp(_7_colorBlack.xy, _7_colorWhite.xy, 0.5f.xx);
        _112 = all(bool2(_100.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.x, _100.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.y));
    }
    else
    {
        _112 = false;
    }
    bool _128 = false;
    if (_112)
    {
        float3 _115 = lerp(_7_colorBlack.xyz, _7_colorWhite.xyz, 0.5f.xxx);
        _128 = all(bool3(_115.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.x, _115.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.y, _115.z == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.z));
    }
    else
    {
        _128 = false;
    }
    bool _139 = false;
    if (_128)
    {
        float4 _131 = lerp(_7_colorBlack, _7_colorWhite, 0.5f.xxxx);
        _139 = all(bool4(_131.x == float4(0.5f, 0.5f, 0.5f, 1.0f).x, _131.y == float4(0.5f, 0.5f, 0.5f, 1.0f).y, _131.z == float4(0.5f, 0.5f, 0.5f, 1.0f).z, _131.w == float4(0.5f, 0.5f, 0.5f, 1.0f).w));
    }
    else
    {
        _139 = false;
    }
    bool _143 = false;
    if (_139)
    {
        _143 = true;
    }
    else
    {
        _143 = false;
    }
    bool _149 = false;
    if (_143)
    {
        _149 = all(bool2(0.5f.xx.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.x, 0.5f.xx.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xy.y));
    }
    else
    {
        _149 = false;
    }
    bool _155 = false;
    if (_149)
    {
        _155 = all(bool3(0.5f.xxx.x == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.x, 0.5f.xxx.y == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.y, 0.5f.xxx.z == float4(0.5f, 0.5f, 0.5f, 1.0f).xyz.z));
    }
    else
    {
        _155 = false;
    }
    bool _158 = false;
    if (_155)
    {
        _158 = true;
    }
    else
    {
        _158 = false;
    }
    bool _170 = false;
    if (_158)
    {
        _170 = lerp(_7_colorWhite.x, _7_testInputs.x, 0.0f) == 1.0f;
    }
    else
    {
        _170 = false;
    }
    bool _184 = false;
    if (_170)
    {
        float2 _173 = lerp(_7_colorWhite.xy, _7_testInputs.xy, float2(0.0f, 0.5f));
        _184 = all(bool2(_173.x == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.x, _173.y == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.y));
    }
    else
    {
        _184 = false;
    }
    bool _198 = false;
    if (_184)
    {
        float3 _187 = lerp(_7_colorWhite.xyz, _7_testInputs.xyz, float3(0.0f, 0.5f, 0.0f));
        _198 = all(bool3(_187.x == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.x, _187.y == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.y, _187.z == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.z));
    }
    else
    {
        _198 = false;
    }
    bool _209 = false;
    if (_198)
    {
        float4 _201 = lerp(_7_colorWhite, _7_testInputs, float4(0.0f, 0.5f, 0.0f, 1.0f));
        _209 = all(bool4(_201.x == float4(1.0f, 0.5f, 1.0f, 2.25f).x, _201.y == float4(1.0f, 0.5f, 1.0f, 2.25f).y, _201.z == float4(1.0f, 0.5f, 1.0f, 2.25f).z, _201.w == float4(1.0f, 0.5f, 1.0f, 2.25f).w));
    }
    else
    {
        _209 = false;
    }
    bool _212 = false;
    if (_209)
    {
        _212 = true;
    }
    else
    {
        _212 = false;
    }
    bool _219 = false;
    if (_212)
    {
        _219 = all(bool2(float2(1.0f, 0.5f).x == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.x, float2(1.0f, 0.5f).y == float4(1.0f, 0.5f, 1.0f, 2.25f).xy.y));
    }
    else
    {
        _219 = false;
    }
    bool _226 = false;
    if (_219)
    {
        _226 = all(bool3(float3(1.0f, 0.5f, 1.0f).x == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.x, float3(1.0f, 0.5f, 1.0f).y == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.y, float3(1.0f, 0.5f, 1.0f).z == float4(1.0f, 0.5f, 1.0f, 2.25f).xyz.z));
    }
    else
    {
        _226 = false;
    }
    bool _229 = false;
    if (_226)
    {
        _229 = true;
    }
    else
    {
        _229 = false;
    }
    float4 _230 = 0.0f.xxxx;
    if (_229)
    {
        _230 = _7_colorGreen;
    }
    else
    {
        _230 = _7_colorRed;
    }
    return _230;
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
