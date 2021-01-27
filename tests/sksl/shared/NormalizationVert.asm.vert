### Compilation failed:

error: SPIR-V validation error: ID 123456[%123456] has not been defined
  %24 = OpLoad %v4float %123456

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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%main = OpFunction %void None %12
%13 = OpLabel
%18 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %18 %15
%20 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%21 = OpLoad %v4float %20
%22 = OpVectorShuffle %v2float %21 %21 0 1
%24 = OpLoad %v4float %123456
%25 = OpVectorShuffle %v2float %24 %24 0 2
%26 = OpFMul %v2float %22 %25
%27 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%28 = OpLoad %v4float %27
%29 = OpVectorShuffle %v2float %28 %28 3 3
%30 = OpLoad %v4float %123456
%31 = OpVectorShuffle %v2float %30 %30 1 3
%32 = OpFMul %v2float %29 %31
%33 = OpFAdd %v2float %26 %32
%34 = OpCompositeExtract %float %33 0
%35 = OpCompositeExtract %float %33 1
%37 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%38 = OpLoad %v4float %37
%39 = OpCompositeExtract %float %38 3
%40 = OpCompositeConstruct %v4float %34 %35 %float_0 %39
%41 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %41 %40
OpReturn
OpFunctionEnd

1 error
