struct A
{
    int x;
    int y;
};

struct B
{
    float x;
    float y[2];
    A z;
};

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static A a1 = { 0, 0 };
static B b1 = { 0.0f, { 0.0f, 0.0f }, { 0, 0 } };

void frag_main()
{
    a1.x = 0;
    b1.x = 0.0f;
    sk_FragColor.x = float(a1.x) + b1.x;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
