struct S
{
    float y;
};

RWByteAddressBuffer _11 : register(u0, space0);
RWByteAddressBuffer _16 : register(u1, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 getColor_h4f_testArr()
{
    return float4(asfloat(_11.Load(0)), asfloat(_11.Load(4)), asfloat(_11.Load(8)), asfloat(_11.Load(12)));
}

float4 getColor_helper_h4f_testArr()
{
    return getColor_h4f_testArr();
}

float unsizedInParameterA_ff_testArr()
{
    return asfloat(_11.Load(0));
}

float unsizedInParameterB_fS_testArrStruct()
{
    return asfloat(_16.Load(0));
}

float unsizedInParameterC_ff_testArr()
{
    return asfloat(_11.Load(0));
}

float unsizedInParameterD_fS_testArrStruct()
{
    return asfloat(_16.Load(0));
}

float unsizedInParameterE_ff_testArr()
{
    return 0.0f;
}

float unsizedInParameterF_fS_testArrStruct()
{
    return 0.0f;
}

void frag_main()
{
    sk_FragColor = getColor_helper_h4f_testArr();
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
