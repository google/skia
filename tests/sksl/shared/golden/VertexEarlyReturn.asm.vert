### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '8[%zoom]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %zoom = OpVariable %_ptr_Uniform_float Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %zoom "zoom"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpDecorate %zoom RelaxedPrecision
OpDecorate %zoom DescriptorSet 0
OpDecorate %19 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_ptr_Uniform_float = OpTypePointer Uniform %float
%zoom = OpVariable %_ptr_Uniform_float Uniform
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%13 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%bool = OpTypeBool
%main = OpFunction %void None %11
%12 = OpLabel
%17 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %17 %13
%19 = OpLoad %float %zoom
%20 = OpFOrdEqual %bool %19 %float_1
OpSelectionMerge %23 None
OpBranchConditional %20 %22 %23
%22 = OpLabel
OpReturn
%23 = OpLabel
%24 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%25 = OpLoad %v4float %24
%26 = OpLoad %float %zoom
%27 = OpVectorTimesScalar %v4float %25 %26
OpStore %24 %27
OpReturn
OpFunctionEnd

1 error
