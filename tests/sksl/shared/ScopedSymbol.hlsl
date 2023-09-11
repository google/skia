struct S
{
    int i;
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _14_colorGreen : packoffset(c0);
    float4 _14_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static int glob = 0;

bool block_variable_hides_global_variable_b()
{
    return glob == 2;
}

bool local_variable_hides_struct_b()
{
    bool S_1 = true;
    return true;
}

bool local_struct_variable_hides_struct_type_b()
{
    S _42 = { 1 };
    S S_1 = _42;
    return S_1.i == 1;
}

bool local_variable_hides_global_variable_b()
{
    int glob_1 = 1;
    return true;
}

float4 main(float2 _51)
{
    glob = 2;
    bool _57 = false;
    if (true)
    {
        _57 = block_variable_hides_global_variable_b();
    }
    else
    {
        _57 = false;
    }
    bool _61 = false;
    if (_57)
    {
        _61 = local_variable_hides_struct_b();
    }
    else
    {
        _61 = false;
    }
    bool _65 = false;
    if (_61)
    {
        _65 = local_struct_variable_hides_struct_type_b();
    }
    else
    {
        _65 = false;
    }
    bool _69 = false;
    if (_65)
    {
        _69 = local_variable_hides_global_variable_b();
    }
    else
    {
        _69 = false;
    }
    float4 _70 = 0.0f.xxxx;
    if (_69)
    {
        _70 = _14_colorGreen;
    }
    else
    {
        _70 = _14_colorRed;
    }
    return _70;
}

void frag_main()
{
    float2 _24 = 0.0f.xx;
    float4 _26 = main(_24);
    sk_FragColor = _26;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
