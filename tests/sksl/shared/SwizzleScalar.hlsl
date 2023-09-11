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
    float4 h4 = _7_unknownInput.xxxx;
    h4 = float4(_7_unknownInput.xx, 0.0f, 1.0f);
    h4 = float4(0.0f, _7_unknownInput, 1.0f, 0.0f);
    float4 _45 = float4(0.0f, _7_unknownInput, 0.0f, _7_unknownInput);
    h4 = _45;
    return _45;
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
