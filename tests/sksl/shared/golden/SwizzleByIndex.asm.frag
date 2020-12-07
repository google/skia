OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %_0_v "_0_v"
OpName %_1_x "_1_x"
OpName %_2_y "_2_y"
OpName %_3_z "_3_z"
OpName %_4_w "_4_w"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %20 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%36 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%main = OpFunction %void None %11
%12 = OpLabel
%_0_v = OpVariable %_ptr_Function_v4float Function
%_1_x = OpVariable %_ptr_Function_float Function
%_2_y = OpVariable %_ptr_Function_float Function
%_3_z = OpVariable %_ptr_Function_float Function
%_4_w = OpVariable %_ptr_Function_float Function
%15 = OpExtInst %float %1 Sqrt %float_1
%17 = OpCompositeConstruct %v4float %15 %15 %15 %15
OpStore %_0_v %17
%20 = OpLoad %v4float %_0_v
%21 = OpCompositeExtract %float %20 0
OpStore %_1_x %21
%23 = OpLoad %v4float %_0_v
%24 = OpCompositeExtract %float %23 1
OpStore %_2_y %24
%26 = OpLoad %v4float %_0_v
%27 = OpCompositeExtract %float %26 2
OpStore %_3_z %27
%29 = OpLoad %v4float %_0_v
%30 = OpCompositeExtract %float %29 3
OpStore %_4_w %30
%31 = OpLoad %float %_1_x
%32 = OpLoad %float %_2_y
%33 = OpLoad %float %_3_z
%34 = OpLoad %float %_4_w
%35 = OpCompositeConstruct %v4float %31 %32 %33 %34
OpStore %sk_FragColor %35
OpStore %sk_FragColor %36
OpReturn
OpFunctionEnd
