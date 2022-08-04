### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-DescriptorSet-06491] Binding decoration on target <id> '3[%3]' must be in the StorageBuffer, Uniform, or UniformConstant storage class
  OpDecorate %3 Binding 456

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %testBlock "testBlock"
OpMemberName %testBlock 0 "x"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %main "main"
OpMemberDecorate %testBlock 0 Offset 0
OpDecorate %testBlock Block
OpDecorate %3 Binding 456
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %21 RelaxedPrecision
%float = OpTypeFloat 32
%testBlock = OpTypeStruct %float
%_ptr_Function_testBlock = OpTypePointer Function %testBlock
%3 = OpVariable %_ptr_Function_testBlock Function
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%14 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Private_float = OpTypePointer Private %float
%main = OpFunction %void None %14
%15 = OpLabel
%18 = OpAccessChain %_ptr_Private_float %3 %int_0
%20 = OpLoad %float %18
%21 = OpCompositeConstruct %v4float %20 %20 %20 %20
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd

1 error
