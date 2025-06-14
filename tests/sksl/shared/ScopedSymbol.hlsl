struct S
{
    int i;
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _17_colorGreen : packoffset(c0);
    float4 _17_colorRed : packoffset(c1);
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
    S _45 = { 1 };
    S S_1 = _45;
    return S_1.i == 1;
}

bool local_variable_hides_global_variable_b()
{
    int glob_1 = 1;
    return true;
}

float4 main(float2 _54)
{
    glob = 2;
    bool _60 = false;
    if (true)
    {
        _60 = block_variable_hides_global_variable_b();
    }
    else
    {
        _60 = false;
    }
    bool _64 = false;
    if (_60)
    {
        _64 = local_variable_hides_struct_b();
    }
    else
    {
        _64 = false;
    }
    bool _68 = false;
    if (_64)
    {
        _68 = local_struct_variable_hides_struct_type_b();
    }
    else
    {
        _68 = false;
    }
    bool _72 = false;
    if (_68)
    {
        _72 = local_variable_hides_global_variable_b();
    }
    else
    {
        _72 = false;
    }
    float4 _73 = 0.0f.xxxx;
    if (_72)
    {
        _73 = _17_colorGreen;
    }
    else
    {
        _73 = _17_colorRed;
    }
    return _73;
}

void frag_main()
{
    float2 _27 = 0.0f.xx;
    float4 _29 = main(_27);
    sk_FragColor = _29;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
