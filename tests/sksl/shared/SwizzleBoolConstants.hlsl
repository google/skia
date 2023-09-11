cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool4 _34 = (_7_colorGreen.y != 0.0f).xxxx;
    bool4 v = _34;
    bool _36 = _34.x;
    bool4 result = bool4(_36, true, true, true);
    result = bool4(_34.xy, false, true);
    result = bool4(_36, true, true, false);
    bool _46 = _34.y;
    result = bool4(false, _46, true, true);
    result = bool4(_34.xyz, true);
    result = bool4(_34.xy, true, true);
    bool _58 = _34.z;
    bool4 _59 = bool4(_36, false, _58, true);
    result = _59;
    result = bool4(_36, true, false, false);
    result = bool4(true, _34.yz, false);
    result = bool4(false, _46, true, false);
    result = bool4(true, true, _58, false);
    result = _34;
    result = bool4(_34.xyz, true);
    bool _75 = _34.w;
    result = bool4(_34.xy, false, _75);
    result = bool4(_34.xy, true, false);
    result = bool4(_36, true, _34.zw);
    result = _59;
    result = bool4(_36, true, true, _75);
    result = bool4(_36, true, false, true);
    result = bool4(true, _34.yzw);
    result = bool4(false, _34.yz, true);
    result = bool4(false, _46, true, _75);
    result = bool4(true, _46, true, true);
    result = bool4(false, false, _34.zw);
    result = bool4(false, false, _58, true);
    bool4 _103 = bool4(false, true, true, _75);
    result = _103;
    float4 _105 = 0.0f.xxxx;
    if (any(_103))
    {
        _105 = _7_colorGreen;
    }
    else
    {
        _105 = _7_colorRed;
    }
    return _105;
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
