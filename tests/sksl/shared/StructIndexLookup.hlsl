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
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
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
            float3 _105 = expected;
            float3 _106 = _105 + float3(1.0f, 10.0f, 100.0f);
            expected = _106;
            if (any(bool3(data.outer[i].inner[j].values.x != _106.x, data.outer[i].inner[j].values.y != _106.y, data.outer[i].inner[j].values.z != _106.z)))
            {
                return _11_colorRed;
            }
            for (int k = 0; k < 3; k++)
            {
                if (data.outer[i].inner[j].values[k] != expected[k])
                {
                    return _11_colorRed;
                }
            }
        }
    }
    return _11_colorGreen;
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
