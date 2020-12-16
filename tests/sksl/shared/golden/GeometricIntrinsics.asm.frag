OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %_0_scalar "_0_scalar"
OpName %_1_x "_1_x"
OpName %x "x"
OpName %_2_vector "_2_vector"
OpName %_3_x "_3_x"
OpName %y "y"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%32 = OpConstantComposite %v2float %float_1 %float_2
%34 = OpConstantComposite %v2float %float_1 %float_2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%38 = OpConstantComposite %v2float %float_3 %float_4
%44 = OpConstantComposite %v2float %float_3 %float_4
%float_1_0 = OpConstant %float 1
%main = OpFunction %void None %11
%12 = OpLabel
%_0_scalar = OpVariable %_ptr_Function_float Function
%_1_x = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_float Function
%_2_vector = OpVariable %_ptr_Function_v2float Function
%_3_x = OpVariable %_ptr_Function_v2float Function
%y = OpVariable %_ptr_Function_v2float Function
OpStore %_1_x %float_1
%17 = OpExtInst %float %1 Length %float_1
OpStore %_1_x %17
%19 = OpLoad %float %_1_x
%18 = OpExtInst %float %1 Distance %19 %float_2
OpStore %_1_x %18
%22 = OpLoad %float %_1_x
%21 = OpFMul %float %22 %float_2
OpStore %_1_x %21
%24 = OpLoad %float %_1_x
%23 = OpExtInst %float %1 Normalize %24
OpStore %_1_x %23
%25 = OpLoad %float %_1_x
OpStore %_0_scalar %25
%27 = OpLoad %float %_0_scalar
OpStore %x %27
OpStore %_3_x %32
%33 = OpExtInst %float %1 Length %34
%35 = OpCompositeConstruct %v2float %33 %33
OpStore %_3_x %35
%37 = OpLoad %v2float %_3_x
%36 = OpExtInst %float %1 Distance %37 %38
%41 = OpCompositeConstruct %v2float %36 %36
OpStore %_3_x %41
%43 = OpLoad %v2float %_3_x
%42 = OpDot %float %43 %44
%45 = OpCompositeConstruct %v2float %42 %42
OpStore %_3_x %45
%47 = OpLoad %v2float %_3_x
%46 = OpExtInst %v2float %1 Normalize %47
OpStore %_3_x %46
%48 = OpLoad %v2float %_3_x
OpStore %_2_vector %48
%50 = OpLoad %v2float %_2_vector
OpStore %y %50
%51 = OpLoad %float %x
%52 = OpLoad %v2float %y
%53 = OpCompositeExtract %float %52 0
%54 = OpCompositeExtract %float %52 1
%56 = OpCompositeConstruct %v4float %51 %53 %54 %float_1_0
OpStore %sk_FragColor %56
OpReturn
OpFunctionEnd
