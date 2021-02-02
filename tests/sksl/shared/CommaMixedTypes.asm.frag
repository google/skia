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
OpName %result "result"
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
OpDecorate %27 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%v2float = OpTypeVector %float 2
%float_2 = OpConstant %float 2
%33 = OpConstantComposite %v2float %float_2 %float_2
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%float_3 = OpConstant %float 3
%41 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%int_2 = OpConstant %int 2
%float_4 = OpConstant %float 4
%float_0 = OpConstant %float 0
%mat2v2float = OpTypeMatrix %v2float 2
%int_3 = OpConstant %int 3
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %23
%28 = OpCompositeExtract %float %27 0
%29 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %29 %28
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %34
%36 = OpCompositeExtract %float %35 1
%37 = OpAccessChain %_ptr_Function_float %result %int_1
OpStore %37 %36
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%43 = OpLoad %v4float %42
%44 = OpCompositeExtract %float %43 2
%45 = OpAccessChain %_ptr_Function_float %result %int_2
OpStore %45 %44
%50 = OpCompositeConstruct %v2float %float_4 %float_0
%51 = OpCompositeConstruct %v2float %float_0 %float_4
%48 = OpCompositeConstruct %mat2v2float %50 %51
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpCompositeExtract %float %54 3
%56 = OpAccessChain %_ptr_Function_float %result %int_3
OpStore %56 %55
%58 = OpLoad %v4float %result
OpReturnValue %58
OpFunctionEnd
