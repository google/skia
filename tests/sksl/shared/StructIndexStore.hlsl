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
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
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
    bool _99 = false;
    if (data.valueAtRoot == 1234)
    {
        _99 = all(bool3(data.outer[0].inner[0].values.x == float3(1.0f, 10.0f, 100.0f).x, data.outer[0].inner[0].values.y == float3(1.0f, 10.0f, 100.0f).y, data.outer[0].inner[0].values.z == float3(1.0f, 10.0f, 100.0f).z));
    }
    else
    {
        _99 = false;
    }
    bool _110 = false;
    if (_99)
    {
        _110 = all(bool3(data.outer[0].inner[1].values.x == float3(2.0f, 20.0f, 200.0f).x, data.outer[0].inner[1].values.y == float3(2.0f, 20.0f, 200.0f).y, data.outer[0].inner[1].values.z == float3(2.0f, 20.0f, 200.0f).z));
    }
    else
    {
        _110 = false;
    }
    bool _122 = false;
    if (_110)
    {
        _122 = all(bool3(data.outer[0].inner[2].values.x == float3(3.0f, 30.0f, 300.0f).x, data.outer[0].inner[2].values.y == float3(3.0f, 30.0f, 300.0f).y, data.outer[0].inner[2].values.z == float3(3.0f, 30.0f, 300.0f).z));
    }
    else
    {
        _122 = false;
    }
    bool _133 = false;
    if (_122)
    {
        _133 = all(bool3(data.outer[1].inner[0].values.x == float3(4.0f, 40.0f, 400.0f).x, data.outer[1].inner[0].values.y == float3(4.0f, 40.0f, 400.0f).y, data.outer[1].inner[0].values.z == float3(4.0f, 40.0f, 400.0f).z));
    }
    else
    {
        _133 = false;
    }
    bool _144 = false;
    if (_133)
    {
        _144 = all(bool3(data.outer[1].inner[1].values.x == float3(5.0f, 50.0f, 500.0f).x, data.outer[1].inner[1].values.y == float3(5.0f, 50.0f, 500.0f).y, data.outer[1].inner[1].values.z == float3(5.0f, 50.0f, 500.0f).z));
    }
    else
    {
        _144 = false;
    }
    bool _155 = false;
    if (_144)
    {
        _155 = all(bool3(data.outer[1].inner[2].values.x == float3(6.0f, 60.0f, 600.0f).x, data.outer[1].inner[2].values.y == float3(6.0f, 60.0f, 600.0f).y, data.outer[1].inner[2].values.z == float3(6.0f, 60.0f, 600.0f).z));
    }
    else
    {
        _155 = false;
    }
    bool _166 = false;
    if (_155)
    {
        _166 = all(bool3(data.outer[2].inner[0].values.x == float3(7.0f, 70.0f, 700.0f).x, data.outer[2].inner[0].values.y == float3(7.0f, 70.0f, 700.0f).y, data.outer[2].inner[0].values.z == float3(7.0f, 70.0f, 700.0f).z));
    }
    else
    {
        _166 = false;
    }
    bool _177 = false;
    if (_166)
    {
        _177 = all(bool3(data.outer[2].inner[1].values.x == float3(8.0f, 80.0f, 800.0f).x, data.outer[2].inner[1].values.y == float3(8.0f, 80.0f, 800.0f).y, data.outer[2].inner[1].values.z == float3(8.0f, 80.0f, 800.0f).z));
    }
    else
    {
        _177 = false;
    }
    bool _188 = false;
    if (_177)
    {
        _188 = all(bool3(data.outer[2].inner[2].values.x == float3(9.0f, 90.0f, 900.0f).x, data.outer[2].inner[2].values.y == float3(9.0f, 90.0f, 900.0f).y, data.outer[2].inner[2].values.z == float3(9.0f, 90.0f, 900.0f).z));
    }
    else
    {
        _188 = false;
    }
    bool ok = _188;
    float4 _189 = 0.0f.xxxx;
    if (_188)
    {
        _189 = _7_colorGreen;
    }
    else
    {
        _189 = _7_colorRed;
    }
    return _189;
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
