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
    int valueAtRoot;
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
    Root data = { 0, { { { { 0.0f.xxx }, { 0.0f.xxx }, { 0.0f.xxx } } }, { { { 0.0f.xxx }, { 0.0f.xxx }, { 0.0f.xxx } } }, { { { 0.0f.xxx }, { 0.0f.xxx }, { 0.0f.xxx } } } } };
    data.valueAtRoot = 1234;
    float3 values = 0.0f.xxx;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            values += float3(1.0f, 10.0f, 100.0f);
            for (int k = 0; k < 3; k++)
            {
                data.outer[i].inner[j].values[k] = values[k];
            }
        }
    }
    bool _101 = false;
    if (data.valueAtRoot == 1234)
    {
        _101 = all(bool3(data.outer[0].inner[0].values.x == float3(1.0f, 10.0f, 100.0f).x, data.outer[0].inner[0].values.y == float3(1.0f, 10.0f, 100.0f).y, data.outer[0].inner[0].values.z == float3(1.0f, 10.0f, 100.0f).z));
    }
    else
    {
        _101 = false;
    }
    bool _112 = false;
    if (_101)
    {
        _112 = all(bool3(data.outer[0].inner[1].values.x == float3(2.0f, 20.0f, 200.0f).x, data.outer[0].inner[1].values.y == float3(2.0f, 20.0f, 200.0f).y, data.outer[0].inner[1].values.z == float3(2.0f, 20.0f, 200.0f).z));
    }
    else
    {
        _112 = false;
    }
    bool _124 = false;
    if (_112)
    {
        _124 = all(bool3(data.outer[0].inner[2].values.x == float3(3.0f, 30.0f, 300.0f).x, data.outer[0].inner[2].values.y == float3(3.0f, 30.0f, 300.0f).y, data.outer[0].inner[2].values.z == float3(3.0f, 30.0f, 300.0f).z));
    }
    else
    {
        _124 = false;
    }
    bool _135 = false;
    if (_124)
    {
        _135 = all(bool3(data.outer[1].inner[0].values.x == float3(4.0f, 40.0f, 400.0f).x, data.outer[1].inner[0].values.y == float3(4.0f, 40.0f, 400.0f).y, data.outer[1].inner[0].values.z == float3(4.0f, 40.0f, 400.0f).z));
    }
    else
    {
        _135 = false;
    }
    bool _146 = false;
    if (_135)
    {
        _146 = all(bool3(data.outer[1].inner[1].values.x == float3(5.0f, 50.0f, 500.0f).x, data.outer[1].inner[1].values.y == float3(5.0f, 50.0f, 500.0f).y, data.outer[1].inner[1].values.z == float3(5.0f, 50.0f, 500.0f).z));
    }
    else
    {
        _146 = false;
    }
    bool _157 = false;
    if (_146)
    {
        _157 = all(bool3(data.outer[1].inner[2].values.x == float3(6.0f, 60.0f, 600.0f).x, data.outer[1].inner[2].values.y == float3(6.0f, 60.0f, 600.0f).y, data.outer[1].inner[2].values.z == float3(6.0f, 60.0f, 600.0f).z));
    }
    else
    {
        _157 = false;
    }
    bool _168 = false;
    if (_157)
    {
        _168 = all(bool3(data.outer[2].inner[0].values.x == float3(7.0f, 70.0f, 700.0f).x, data.outer[2].inner[0].values.y == float3(7.0f, 70.0f, 700.0f).y, data.outer[2].inner[0].values.z == float3(7.0f, 70.0f, 700.0f).z));
    }
    else
    {
        _168 = false;
    }
    bool _179 = false;
    if (_168)
    {
        _179 = all(bool3(data.outer[2].inner[1].values.x == float3(8.0f, 80.0f, 800.0f).x, data.outer[2].inner[1].values.y == float3(8.0f, 80.0f, 800.0f).y, data.outer[2].inner[1].values.z == float3(8.0f, 80.0f, 800.0f).z));
    }
    else
    {
        _179 = false;
    }
    bool _190 = false;
    if (_179)
    {
        _190 = all(bool3(data.outer[2].inner[2].values.x == float3(9.0f, 90.0f, 900.0f).x, data.outer[2].inner[2].values.y == float3(9.0f, 90.0f, 900.0f).y, data.outer[2].inner[2].values.z == float3(9.0f, 90.0f, 900.0f).z));
    }
    else
    {
        _190 = false;
    }
    bool ok = _190;
    float4 _191 = 0.0f.xxxx;
    if (_190)
    {
        _191 = _10_colorGreen;
    }
    else
    {
        _191 = _10_colorRed;
    }
    return _191;
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
