cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 this_function_is_prototyped_after_its_definition_h4h4(float4 _24)
{
    return -_24;
}

float4 this_function_is_defined_before_use_h4h4(float4 _28)
{
    float4 _32 = -_28;
    return -this_function_is_prototyped_after_its_definition_h4h4(_32);
}

float4 main(float2 _36)
{
    float4 _44 = -_9_colorGreen;
    return this_function_is_defined_before_use_h4h4(_44);
}

void frag_main()
{
    float2 _19 = 0.0f.xx;
    sk_FragColor = main(_19);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
