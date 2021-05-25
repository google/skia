OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "sk_RTAdjust"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpMemberDecorate %_UniformBuffer 0 DescriptorSet 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %8 Binding 0
OpDecorate %8 DescriptorSet 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%12 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%15 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%main = OpFunction %void None %12
%13 = OpLabel
%18 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %18 %15
%20 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
%22 = OpLoad %v4float %20
%23 = OpVectorShuffle %v2float %22 %22 0 2
%25 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%26 = OpLoad %v4float %25
%27 = OpVectorShuffle %v2float %26 %26 0 1
%28 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%29 = OpLoad %v4float %28
%30 = OpVectorShuffle %v2float %29 %29 3 3
%31 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
%32 = OpLoad %v4float %31
%33 = OpVectorShuffle %v2float %32 %32 1 3
%34 = OpFMul %v2float %30 %33
%35 = OpFSub %v2float %27 %34
%36 = OpFMul %v2float %23 %35
%37 = OpCompositeExtract %float %36 0
%38 = OpCompositeExtract %float %36 1
%40 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%41 = OpLoad %v4float %40
%42 = OpCompositeExtract %float %41 3
%43 = OpCompositeConstruct %v4float %37 %38 %float_0 %42
%44 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %44 %43
OpReturn
OpFunctionEnd
