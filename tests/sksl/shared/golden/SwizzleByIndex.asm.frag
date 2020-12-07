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
%38 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
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
%21 = OpCompositeExtract %float %20 0
OpStore %x %21
%23 = OpLoad %v4float %v
%24 = OpCompositeExtract %float %23 1
OpStore %y %24
%26 = OpLoad %v4float %v
%27 = OpCompositeExtract %float %26 2
OpStore %z %27
%29 = OpLoad %v4float %v
%30 = OpCompositeExtract %float %29 3
OpStore %w %30
%31 = OpLoad %float %x
%32 = OpLoad %float %y
%33 = OpLoad %float %z
%34 = OpLoad %float %w
%35 = OpCompositeConstruct %v4float %31 %32 %33 %34
OpStore %sk_FragColor %35
OpStore %v %36
OpStore %x %float_2
OpStore %y %float_2
OpStore %z %float_2
OpStore %w %float_2
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
