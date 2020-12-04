OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %v "v"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %20 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
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
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%main = OpFunction %void None %11
%12 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_float Function
%w = OpVariable %_ptr_Function_float Function
%15 = OpExtInst %float %1 Sqrt %float_1
%17 = OpCompositeConstruct %v4float %15 %15 %15 %15
OpStore %v %17
%20 = OpLoad %v4float %v
%23 = OpVectorExtractDynamic %float %20 %int_0
OpStore %x %23
%25 = OpLoad %v4float %v
%27 = OpVectorExtractDynamic %float %25 %int_1
OpStore %y %27
%29 = OpLoad %v4float %v
%31 = OpVectorExtractDynamic %float %29 %int_2
OpStore %z %31
%33 = OpLoad %v4float %v
%35 = OpVectorExtractDynamic %float %33 %int_3
OpStore %w %35
%36 = OpLoad %float %x
%37 = OpLoad %float %y
%38 = OpLoad %float %z
%39 = OpLoad %float %w
%40 = OpCompositeConstruct %v4float %36 %37 %38 %39
OpStore %sk_FragColor %40
OpReturn
OpFunctionEnd
