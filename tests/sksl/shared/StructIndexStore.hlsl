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
    bool _102 = false;
    if (data.valueAtRoot == 1234)
    {
        _102 = all(bool3(data.outer[0].inner[0].values.x == float3(1.0f, 10.0f, 100.0f).x, data.outer[0].inner[0].values.y == float3(1.0f, 10.0f, 100.0f).y, data.outer[0].inner[0].values.z == float3(1.0f, 10.0f, 100.0f).z));
    }
    else
    {
        _102 = false;
    }
    bool _113 = false;
    if (_102)
    {
        _113 = all(bool3(data.outer[0].inner[1].values.x == float3(2.0f, 20.0f, 200.0f).x, data.outer[0].inner[1].values.y == float3(2.0f, 20.0f, 200.0f).y, data.outer[0].inner[1].values.z == float3(2.0f, 20.0f, 200.0f).z));
    }
    else
    {
        _113 = false;
    }
    bool _125 = false;
    if (_113)
    {
        _125 = all(bool3(data.outer[0].inner[2].values.x == float3(3.0f, 30.0f, 300.0f).x, data.outer[0].inner[2].values.y == float3(3.0f, 30.0f, 300.0f).y, data.outer[0].inner[2].values.z == float3(3.0f, 30.0f, 300.0f).z));
    }
    else
    {
        _125 = false;
    }
    bool _136 = false;
    if (_125)
    {
        _136 = all(bool3(data.outer[1].inner[0].values.x == float3(4.0f, 40.0f, 400.0f).x, data.outer[1].inner[0].values.y == float3(4.0f, 40.0f, 400.0f).y, data.outer[1].inner[0].values.z == float3(4.0f, 40.0f, 400.0f).z));
    }
    else
    {
        _136 = false;
    }
    bool _147 = false;
    if (_136)
    {
        _147 = all(bool3(data.outer[1].inner[1].values.x == float3(5.0f, 50.0f, 500.0f).x, data.outer[1].inner[1].values.y == float3(5.0f, 50.0f, 500.0f).y, data.outer[1].inner[1].values.z == float3(5.0f, 50.0f, 500.0f).z));
    }
    else
    {
        _147 = false;
    }
    bool _158 = false;
    if (_147)
    {
        _158 = all(bool3(data.outer[1].inner[2].values.x == float3(6.0f, 60.0f, 600.0f).x, data.outer[1].inner[2].values.y == float3(6.0f, 60.0f, 600.0f).y, data.outer[1].inner[2].values.z == float3(6.0f, 60.0f, 600.0f).z));
    }
    else
    {
        _158 = false;
    }
    bool _169 = false;
    if (_158)
    {
        _169 = all(bool3(data.outer[2].inner[0].values.x == float3(7.0f, 70.0f, 700.0f).x, data.outer[2].inner[0].values.y == float3(7.0f, 70.0f, 700.0f).y, data.outer[2].inner[0].values.z == float3(7.0f, 70.0f, 700.0f).z));
    }
    else
    {
        _169 = false;
    }
    bool _180 = false;
    if (_169)
    {
        _180 = all(bool3(data.outer[2].inner[1].values.x == float3(8.0f, 80.0f, 800.0f).x, data.outer[2].inner[1].values.y == float3(8.0f, 80.0f, 800.0f).y, data.outer[2].inner[1].values.z == float3(8.0f, 80.0f, 800.0f).z));
    }
    else
    {
        _180 = false;
    }
    bool _191 = false;
    if (_180)
    {
        _191 = all(bool3(data.outer[2].inner[2].values.x == float3(9.0f, 90.0f, 900.0f).x, data.outer[2].inner[2].values.y == float3(9.0f, 90.0f, 900.0f).y, data.outer[2].inner[2].values.z == float3(9.0f, 90.0f, 900.0f).z));
    }
    else
    {
        _191 = false;
    }
    bool ok = _191;
    float4 _192 = 0.0f.xxxx;
    if (_191)
    {
        _192 = _11_colorGreen;
    }
    else
    {
        _192 = _11_colorRed;
    }
    return _192;
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
