OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %x "x"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%v = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%26 = OpLoad %float %22
OpStore %x %26
%29 = OpLoad %float %x
%30 = OpCompositeConstruct %v2float %29 %29
%32 = OpCompositeExtract %float %30 0
%33 = OpCompositeExtract %float %30 1
%36 = OpCompositeConstruct %v4float %32 %33 %float_0 %float_1
OpStore %v %36
%37 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%38 = OpLoad %float %37
%39 = OpCompositeConstruct %v2float %38 %38
%40 = OpCompositeExtract %float %39 0
%41 = OpCompositeExtract %float %39 1
%42 = OpCompositeConstruct %v4float %40 %41 %float_0 %float_1
OpStore %v %42
%43 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%44 = OpLoad %float %43
%45 = OpCompositeConstruct %v4float %float_0 %44 %float_1 %float_0
OpStore %v %45
%46 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%47 = OpLoad %float %46
%48 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%49 = OpLoad %float %48
%50 = OpCompositeConstruct %v4float %float_0 %47 %float_0 %49
OpStore %v %50
%51 = OpLoad %v4float %v
OpReturnValue %51
OpFunctionEnd
