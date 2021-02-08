OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%float_14 = OpConstant %float 14
%float_0_0500000007 = OpConstant %float 0.0500000007
%float_6 = OpConstant %float 6
%float_0 = OpConstant %float 0
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
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %x %float_2
OpStore %y %float_0_5
%28 = OpExtInst %float %1 Sqrt %float_2
%29 = OpFOrdGreaterThan %bool %28 %float_2
OpStore %c %29
%32 = OpLoad %bool %c
%33 = OpLogicalNotEqual %bool %true %32
OpStore %d %33
%35 = OpLoad %bool %c
OpStore %e %35
OpStore %x %float_14
OpStore %x %float_2
OpStore %y %float_0_0500000007
%38 = OpFMul %float %float_2 %float_0_0500000007
OpStore %x %38
OpStore %x %float_6
%40 = OpLoad %bool %c
%41 = OpSelect %float %40 %float_1 %float_0
%43 = OpLoad %bool %d
%44 = OpSelect %float %43 %float_1 %float_0
%45 = OpFMul %float %41 %44
%46 = OpLoad %bool %e
%47 = OpSelect %float %46 %float_1 %float_0
%48 = OpFMul %float %45 %47
OpStore %y %48
OpStore %y %float_6
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %49
OpReturnValue %53
OpFunctionEnd
