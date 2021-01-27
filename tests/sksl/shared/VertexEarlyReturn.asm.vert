OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "zoom"
OpName %zoom "zoom"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpMemberDecorate %_UniformBuffer 0 DescriptorSet 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %8 Binding 0
OpDecorate %8 DescriptorSet 0
OpDecorate %zoom RelaxedPrecision
OpDecorate %zoom DescriptorSet 0
OpDecorate %22 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Private_float = OpTypePointer Private %float
%zoom = OpVariable %_ptr_Private_float Private
%void = OpTypeVoid
%14 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%bool = OpTypeBool
%main = OpFunction %void None %14
%15 = OpLabel
%20 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %20 %17
%22 = OpLoad %float %zoom
%23 = OpFOrdEqual %bool %22 %float_1
OpSelectionMerge %26 None
OpBranchConditional %23 %25 %26
%25 = OpLabel
OpReturn
%26 = OpLabel
%27 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%28 = OpLoad %v4float %27
%29 = OpLoad %float %zoom
%30 = OpVectorTimesScalar %v4float %28 %29
OpStore %27 %30
OpReturn
OpFunctionEnd
