cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool _33 = _10_unknownInput != 0.0f;
    bool b = _33;
    bool4 b4 = _33.xxxx;
    b4 = bool4(_33.xx, false, true);
    b4 = bool4(false, _33, true, false);
    bool4 _46 = bool4(false, _33, false, _33);
    b4 = _46;
    return float4(float(false), float(_46.y), float(_46.z), float(_46.w));
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
