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
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
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
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
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
%34 = OpConvertSToF %float %int_0
%36 = OpConvertSToF %float %int_1
%37 = OpCompositeConstruct %v4float %32 %33 %34 %36
OpStore %v %37
%38 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%39 = OpLoad %float %38
%40 = OpCompositeConstruct %v2float %39 %39
%41 = OpCompositeExtract %float %40 0
%42 = OpCompositeExtract %float %40 1
%43 = OpConvertSToF %float %int_0
%44 = OpConvertSToF %float %int_1
%45 = OpCompositeConstruct %v4float %41 %42 %43 %44
OpStore %v %45
%46 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%47 = OpLoad %float %46
%48 = OpConvertSToF %float %int_0
%49 = OpConvertSToF %float %int_1
%50 = OpCompositeConstruct %v3float %47 %48 %49
%52 = OpVectorShuffle %v4float %50 %50 1 0 2 1
OpStore %v %52
%53 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%54 = OpLoad %float %53
%55 = OpCompositeConstruct %v2float %54 %54
%56 = OpCompositeExtract %float %55 0
%57 = OpCompositeExtract %float %55 1
%58 = OpConvertSToF %float %int_0
%59 = OpCompositeConstruct %v3float %56 %57 %58
%60 = OpVectorShuffle %v4float %59 %59 2 0 2 1
OpStore %v %60
%61 = OpLoad %v4float %v
OpReturnValue %61
OpFunctionEnd
