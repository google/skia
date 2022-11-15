### Compilation failed:

error: SPIR-V validation error: Structure id 11 decorated as Block for variable in Uniform storage class must follow standard uniform buffer layout rules: member 0 contains an array with stride 4 not satisfying alignment to 16
  %testUniforms = OpTypeStruct %_arr_float_int_2

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %testPushConstants "testPushConstants"
OpMemberName %testPushConstants 0 "pushConstantArray"
OpName %testUniforms "testUniforms"
OpMemberName %testUniforms 0 "uboArray"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %main "main"
OpName %S "S"
OpMemberName %S 0 "a"
OpName %s1 "s1"
OpName %s2 "s2"
OpDecorate %_arr_float_int_2 ArrayStride 4
OpMemberDecorate %testPushConstants 0 Offset 0
OpDecorate %testPushConstants Block
OpDecorate %_arr_float_int_2 ArrayStride 16
OpMemberDecorate %testUniforms 0 Offset 0
OpDecorate %testUniforms Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %S 0 Offset 0
OpDecorate %51 RelaxedPrecision
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%testPushConstants = OpTypeStruct %_arr_float_int_2
%_ptr_PushConstant_testPushConstants = OpTypePointer PushConstant %testPushConstants
%3 = OpVariable %_ptr_PushConstant_testPushConstants PushConstant
%testUniforms = OpTypeStruct %_arr_float_int_2
%_ptr_Uniform_testUniforms = OpTypePointer Uniform %testUniforms
%10 = OpVariable %_ptr_Uniform_testUniforms Uniform
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%20 = OpTypeFunction %void
%S = OpTypeStruct %_arr_float_int_2
%_ptr_Function_S = OpTypePointer Function %S
%int_0 = OpConstant %int 0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Uniform__arr_float_int_2 = OpTypePointer Uniform %_arr_float_int_2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%48 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %20
%21 = OpLabel
%s1 = OpVariable %_ptr_Function_S Function
%s2 = OpVariable %_ptr_Function_S Function
%42 = OpVariable %_ptr_Function_v4float Function
%26 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
%28 = OpLoad %_arr_float_int_2 %26
%29 = OpCompositeConstruct %S %28
OpStore %s1 %29
%31 = OpAccessChain %_ptr_Uniform__arr_float_int_2 %10 %int_0
%33 = OpLoad %_arr_float_int_2 %31
%34 = OpCompositeConstruct %S %33
OpStore %s2 %34
%35 = OpCompositeExtract %float %28 0
%36 = OpCompositeExtract %float %33 0
%37 = OpFOrdEqual %bool %35 %36
%38 = OpCompositeExtract %float %28 1
%39 = OpCompositeExtract %float %33 1
%40 = OpFOrdEqual %bool %38 %39
%41 = OpLogicalAnd %bool %40 %37
OpSelectionMerge %46 None
OpBranchConditional %41 %44 %45
%44 = OpLabel
OpStore %42 %48
OpBranch %46
%45 = OpLabel
OpStore %42 %50
OpBranch %46
%46 = OpLabel
%51 = OpLoad %v4float %42
OpStore %sk_FragColor %51
OpReturn
OpFunctionEnd

1 error
