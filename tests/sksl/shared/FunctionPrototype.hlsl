cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 this_function_is_defined_before_use_h4h4(float4 _23)
{
    return -_23;
}

float4 main(float2 _28)
{
    float4 _36 = -_8_colorGreen;
    return this_function_is_defined_before_use_h4h4(_36);
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
