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
OpMemberName %testUniforms 0 "uniformArray"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %main "main"
OpName %localArray "localArray"
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
OpDecorate %60 RelaxedPrecision
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
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_float_int_2 = OpTypePointer Uniform %_arr_float_int_2
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%57 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%59 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %20
%21 = OpLabel
%localArray = OpVariable %_ptr_Function__arr_float_int_2 Function
%52 = OpVariable %_ptr_Function_v4float Function
%26 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %localArray %26
%29 = OpAccessChain %_ptr_Uniform__arr_float_int_2 %10 %int_0
%31 = OpLoad %_arr_float_int_2 %29
%32 = OpCompositeExtract %float %31 0
%33 = OpFOrdEqual %bool %float_1 %32
%34 = OpCompositeExtract %float %31 1
%35 = OpFOrdEqual %bool %float_2 %34
%36 = OpLogicalAnd %bool %35 %33
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%39 = OpAccessChain %_ptr_Uniform__arr_float_int_2 %10 %int_0
%40 = OpLoad %_arr_float_int_2 %39
%41 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
%43 = OpLoad %_arr_float_int_2 %41
%44 = OpCompositeExtract %float %40 0
%45 = OpCompositeExtract %float %43 0
%46 = OpFOrdEqual %bool %44 %45
%47 = OpCompositeExtract %float %40 1
%48 = OpCompositeExtract %float %43 1
%49 = OpFOrdEqual %bool %47 %48
%50 = OpLogicalAnd %bool %49 %46
OpBranch %38
%38 = OpLabel
%51 = OpPhi %bool %false %21 %50 %37
OpSelectionMerge %56 None
OpBranchConditional %51 %54 %55
%54 = OpLabel
OpStore %52 %57
OpBranch %56
%55 = OpLabel
OpStore %52 %59
OpBranch %56
%56 = OpLabel
%60 = OpLoad %v4float %52
OpStore %sk_FragColor %60
OpReturn
OpFunctionEnd

1 error
