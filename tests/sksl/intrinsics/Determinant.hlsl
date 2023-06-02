cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _10_testMatrix2x2 : packoffset(c0);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool4 _36 = (determinant(_10_testMatrix2x2) == (-2.0f)).xxxx;
    return float4(_36.x ? _10_colorGreen.x : _10_colorRed.x, _36.y ? _10_colorGreen.y : _10_colorRed.y, _36.z ? _10_colorGreen.z : _10_colorRed.z, _36.w ? _10_colorGreen.w : _10_colorRed.w);
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
