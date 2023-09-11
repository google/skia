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
    int _30 = int(_7_unknownInput);
    int i = _30;
    int4 i4 = _30.xxxx;
    i4 = int4(_30.xx, 0, 1);
    i4 = int4(0, _30, 1, 0);
    int4 _42 = int4(0, _30, 0, _30);
    i4 = _42;
    return float4(float(0), float(_42.y), float(_42.z), float(_42.w));
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
