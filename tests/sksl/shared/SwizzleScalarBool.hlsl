cbuffer _UniformBuffer : register(b0, space0)
{
    float _7_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool _31 = _7_unknownInput != 0.0f;
    bool b = _31;
    bool4 b4 = _31.xxxx;
    b4 = bool4(_31.xx, false, true);
    b4 = bool4(false, _31, true, false);
    bool4 _44 = bool4(false, _31, false, _31);
    b4 = _44;
    return float4(float(false), float(_44.y), float(_44.z), float(_44.w));
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
