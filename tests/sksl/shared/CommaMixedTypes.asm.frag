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
OpDecorate %28 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
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
%34 = OpConstantComposite %v2float %float_2 %float_2
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%float_3 = OpConstant %float 3
%42 = OpConstantComposite %v3float %float_3 %float_3 %float_3
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
%22 = OpExtInst %float %1 Sqrt %float_1
%24 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%28 = OpLoad %v4float %24
%29 = OpCompositeExtract %float %28 0
%30 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %30 %29
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%36 = OpLoad %v4float %35
%37 = OpCompositeExtract %float %36 1
%38 = OpAccessChain %_ptr_Function_float %result %int_1
OpStore %38 %37
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpCompositeExtract %float %44 2
%46 = OpAccessChain %_ptr_Function_float %result %int_2
OpStore %46 %45
%51 = OpCompositeConstruct %v2float %float_4 %float_0
%52 = OpCompositeConstruct %v2float %float_0 %float_4
%49 = OpCompositeConstruct %mat2v2float %51 %52
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%55 = OpLoad %v4float %54
%56 = OpCompositeExtract %float %55 3
%57 = OpAccessChain %_ptr_Function_float %result %int_3
OpStore %57 %56
%59 = OpLoad %v4float %result
OpReturnValue %59
OpFunctionEnd
