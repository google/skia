OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3 %pos
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %pos "pos"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "sk_RTAdjust"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpDecorate %pos Location 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%pos = OpVariable %_ptr_Input_v4float Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_0 = OpConstant %float 0
%main = OpFunction %void None %14
%15 = OpLabel
%16 = OpLoad %v4float %pos
%19 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %19 %16
%21 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%22 = OpLoad %v4float %21
%23 = OpVectorShuffle %v2float %22 %22 0 1
%25 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %25
%28 = OpVectorShuffle %v2float %27 %27 0 2
%29 = OpFMul %v2float %23 %28
%30 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%31 = OpLoad %v4float %30
%32 = OpVectorShuffle %v2float %31 %31 3 3
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %33
%35 = OpVectorShuffle %v2float %34 %34 1 3
%36 = OpFMul %v2float %32 %35
%37 = OpFAdd %v2float %29 %36
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%41 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%42 = OpLoad %v4float %41
%43 = OpCompositeExtract %float %42 3
%44 = OpCompositeConstruct %v4float %38 %39 %float_0 %43
%45 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %45 %44
OpReturn
OpFunctionEnd
