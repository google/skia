cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 _active = _7_colorGreen;
    float4 _centroid = _7_colorGreen;
    float4 _coherent = _7_colorGreen;
    float4 _common = _7_colorGreen;
    float4 _filter = _7_colorGreen;
    float4 _partition = _7_colorGreen;
    float4 _patch = _7_colorGreen;
    float4 _precise = _7_colorGreen;
    float4 _resource = _7_colorGreen;
    float4 _restrict = _7_colorGreen;
    float4 _shared = _7_colorGreen;
    float4 _smooth = _7_colorGreen;
    float4 _subroutine = _7_colorGreen;
    return (((((((((((_7_colorGreen * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen) * _7_colorGreen;
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
