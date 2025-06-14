cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _active = _11_colorGreen;
    float4 _centroid = _11_colorGreen;
    float4 _coherent = _11_colorGreen;
    float4 _common = _11_colorGreen;
    float4 _filter = _11_colorGreen;
    float4 _partition = _11_colorGreen;
    float4 _patch = _11_colorGreen;
    float4 _precise = _11_colorGreen;
    float4 _resource = _11_colorGreen;
    float4 _restrict = _11_colorGreen;
    float4 _shared = _11_colorGreen;
    float4 _smooth = _11_colorGreen;
    float4 _subroutine = _11_colorGreen;
    return (((((((((((_11_colorGreen * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen) * _11_colorGreen;
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
