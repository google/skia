OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_v "_0_v"
OpName %_1_x "_1_x"
OpName %_2_y "_2_y"
OpName %_3_z "_3_z"
OpName %_4_w "_4_w"
OpName %a "a"
OpName %_9_x "_9_x"
OpName %_10_y "_10_y"
OpName %_11_z "_11_z"
OpName %_12_w "_12_w"
OpName %b "b"
OpName %_13_v "_13_v"
OpName %_14_x "_14_x"
OpName %_15_y "_15_y"
OpName %_16_z "_16_z"
OpName %_17_w "_17_w"
OpName %c "c"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_0_v RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %_1_x RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %_2_y RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %_3_z RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %_4_w RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %_9_x RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %_10_y RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %_11_z RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %_12_w RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %_13_v RelaxedPrecision
OpDecorate %_14_x RelaxedPrecision
OpDecorate %_15_y RelaxedPrecision
OpDecorate %_16_z RelaxedPrecision
OpDecorate %_17_w RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%66 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%76 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%_0_v = OpVariable %_ptr_Function_v4float Function
%_1_x = OpVariable %_ptr_Function_float Function
%_2_y = OpVariable %_ptr_Function_float Function
%_3_z = OpVariable %_ptr_Function_float Function
%_4_w = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_v4float Function
%_9_x = OpVariable %_ptr_Function_float Function
%_10_y = OpVariable %_ptr_Function_float Function
%_11_z = OpVariable %_ptr_Function_float Function
%_12_w = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_v4float Function
%_13_v = OpVariable %_ptr_Function_v4float Function
%_14_x = OpVariable %_ptr_Function_float Function
%_15_y = OpVariable %_ptr_Function_float Function
%_16_z = OpVariable %_ptr_Function_float Function
%_17_w = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_v4float Function
%90 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
OpStore %_0_v %32
%35 = OpCompositeExtract %float %32 0
OpStore %_1_x %35
%37 = OpCompositeExtract %float %32 1
OpStore %_2_y %37
%39 = OpCompositeExtract %float %32 2
OpStore %_3_z %39
%41 = OpCompositeExtract %float %32 3
OpStore %_4_w %41
%43 = OpCompositeConstruct %v4float %35 %37 %39 %41
OpStore %a %43
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%46 = OpLoad %v4float %45
%47 = OpCompositeExtract %float %46 0
OpStore %_9_x %47
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpCompositeExtract %float %50 1
OpStore %_10_y %51
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpCompositeExtract %float %54 2
OpStore %_11_z %55
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpCompositeExtract %float %58 3
OpStore %_12_w %59
%61 = OpCompositeConstruct %v4float %47 %51 %55 %59
OpStore %b %61
OpStore %_13_v %66
OpStore %_14_x %float_0
OpStore %_15_y %float_1
OpStore %_16_z %float_2
OpStore %_17_w %float_3
OpStore %c %66
%77 = OpFOrdEqual %v4bool %43 %76
%79 = OpAll %bool %77
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpFOrdEqual %v4bool %61 %76
%83 = OpAll %bool %82
OpBranch %81
%81 = OpLabel
%84 = OpPhi %bool %false %25 %83 %80
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpFOrdEqual %v4bool %66 %66
%88 = OpAll %bool %87
OpBranch %86
%86 = OpLabel
%89 = OpPhi %bool %false %81 %88 %85
OpSelectionMerge %93 None
OpBranchConditional %89 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%96 = OpLoad %v4float %94
OpStore %90 %96
OpBranch %93
%92 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%99 = OpLoad %v4float %97
OpStore %90 %99
OpBranch %93
%93 = OpLabel
%100 = OpLoad %v4float %90
OpReturnValue %100
OpFunctionEnd
