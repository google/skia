### Compilation failed:

error: SPIR-V validation error: ID 123456[%123456] has not been defined
  %20 = OpLoad %float %123456

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "zoom"
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
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%12 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%15 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%bool = OpTypeBool
%main = OpFunction %void None %12
%13 = OpLabel
%18 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %18 %15
%20 = OpLoad %float %123456
%21 = OpFOrdEqual %bool %20 %float_1
OpSelectionMerge %24 None
OpBranchConditional %21 %23 %24
%23 = OpLabel
OpReturn
%24 = OpLabel
%25 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%26 = OpLoad %v4float %25
%27 = OpLoad %float %123456
%28 = OpVectorTimesScalar %v4float %26 %27
OpStore %25 %28
OpReturn
OpFunctionEnd

1 error
