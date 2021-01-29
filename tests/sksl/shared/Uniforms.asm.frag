### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '10[%myHalf]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %myHalf = OpVariable %_ptr_Uniform_float Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %myHalf "myHalf"
OpName %myHalf4 "myHalf4"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %myHalf RelaxedPrecision
OpDecorate %myHalf DescriptorSet 0
OpDecorate %myHalf4 RelaxedPrecision
OpDecorate %myHalf4 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Uniform_float = OpTypePointer Uniform %float
%myHalf = OpVariable %_ptr_Uniform_float Uniform
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%myHalf4 = OpVariable %_ptr_Uniform_v4float Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%19 = OpTypeFunction %v4float
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %19
%20 = OpLabel
%21 = OpLoad %v4float %myHalf4
%22 = OpLoad %float %myHalf
%23 = OpVectorTimesScalar %v4float %21 %22
OpReturnValue %23
OpFunctionEnd

1 error
