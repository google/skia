OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint "_entrypoint"
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
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %60 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%39 = OpConstantComposite %v2float %float_1 %float_2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%47 = OpConstantComposite %v2float %float_3 %float_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%_0_scalar = OpVariable %_ptr_Function_float Function
%_1_x = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_float Function
%_2_vector = OpVariable %_ptr_Function_v2float Function
%_3_x = OpVariable %_ptr_Function_v2float Function
%y = OpVariable %_ptr_Function_v2float Function
OpStore %_1_x %float_1
%25 = OpLoad %float %_1_x
%24 = OpExtInst %float %1 Length %25
OpStore %_1_x %24
%27 = OpLoad %float %_1_x
%26 = OpExtInst %float %1 Distance %27 %float_2
OpStore %_1_x %26
%30 = OpLoad %float %_1_x
%29 = OpFMul %float %30 %float_2
OpStore %_1_x %29
%32 = OpLoad %float %_1_x
%31 = OpExtInst %float %1 Normalize %32
OpStore %_1_x %31
%34 = OpLoad %float %_1_x
OpStore %x %34
OpStore %_3_x %39
%41 = OpLoad %v2float %_3_x
%40 = OpExtInst %float %1 Length %41
%42 = OpCompositeConstruct %v2float %40 %40
OpStore %_3_x %42
%44 = OpLoad %v2float %_3_x
%43 = OpExtInst %float %1 Distance %44 %47
%48 = OpCompositeConstruct %v2float %43 %43
OpStore %_3_x %48
%50 = OpLoad %v2float %_3_x
%49 = OpDot %float %50 %47
%51 = OpCompositeConstruct %v2float %49 %49
OpStore %_3_x %51
%53 = OpLoad %v2float %_3_x
%52 = OpExtInst %v2float %1 Normalize %53
OpStore %_3_x %52
%55 = OpLoad %v2float %_3_x
OpStore %y %55
%56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %56
OpReturnValue %60
OpFunctionEnd
