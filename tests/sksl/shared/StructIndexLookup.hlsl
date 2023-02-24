struct InnerLUT
{
    float3 values;
};

struct OuterLUT
{
    InnerLUT inner[3];
};

struct Root
{
    OuterLUT outer[3];
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    Root data = { { { { { 0.0f.xxx }, { 0.0f.xxx }, { 0.0f.xxx } } }, { { { 0.0f.xxx }, { 0.0f.xxx }, { 0.0f.xxx } } }, { { { 0.0f.xxx }, { 0.0f.xxx }, { 0.0f.xxx } } } } };
    data.outer[0].inner[0].values = float3(1.0f, 10.0f, 100.0f);
    data.outer[0].inner[1].values = float3(2.0f, 20.0f, 200.0f);
    data.outer[0].inner[2].values = float3(3.0f, 30.0f, 300.0f);
    data.outer[1].inner[0].values = float3(4.0f, 40.0f, 400.0f);
    data.outer[1].inner[1].values = float3(5.0f, 50.0f, 500.0f);
    data.outer[1].inner[2].values = float3(6.0f, 60.0f, 600.0f);
    data.outer[2].inner[0].values = float3(7.0f, 70.0f, 700.0f);
    data.outer[2].inner[1].values = float3(8.0f, 80.0f, 800.0f);
    data.outer[2].inner[2].values = float3(9.0f, 90.0f, 900.0f);
    float3 expected = 0.0f.xxx;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            float3 _104 = expected;
            float3 _105 = _104 + float3(1.0f, 10.0f, 100.0f);
            expected = _105;
            if (any(bool3(data.outer[i].inner[j].values.x != _105.x, data.outer[i].inner[j].values.y != _105.y, data.outer[i].inner[j].values.z != _105.z)))
            {
                return _10_colorRed;
            }
            for (int k = 0; k < 3; k++)
            {
                if (data.outer[i].inner[j].values[k] != expected[k])
                {
                    return _10_colorRed;
                }
            }
        }
    }
    return _10_colorGreen;
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
