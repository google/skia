cbuffer _UniformBuffer : register(b0, space0)
{
    float _11_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool _34 = _11_unknownInput != 0.0f;
    bool b = _34;
    bool4 b4 = _34.xxxx;
    b4 = bool4(_34.xx, false, true);
    b4 = bool4(false, _34, true, false);
    bool4 _47 = bool4(false, _34, false, _34);
    b4 = _47;
    return float4(float(false), float(_47.y), float(_47.z), float(_47.w));
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
