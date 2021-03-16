OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
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
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%73 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%97 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
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
%113 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
OpStore %_0_v %26
%29 = OpLoad %v4float %_0_v
%30 = OpCompositeExtract %float %29 0
OpStore %_1_x %30
%32 = OpLoad %v4float %_0_v
%33 = OpCompositeExtract %float %32 1
OpStore %_2_y %33
%35 = OpLoad %v4float %_0_v
%36 = OpCompositeExtract %float %35 2
OpStore %_3_z %36
%38 = OpLoad %v4float %_0_v
%39 = OpCompositeExtract %float %38 3
OpStore %_4_w %39
%41 = OpLoad %float %_1_x
%42 = OpLoad %float %_2_y
%43 = OpLoad %float %_3_z
%44 = OpLoad %float %_4_w
%45 = OpCompositeConstruct %v4float %41 %42 %43 %44
OpStore %a %45
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%48 = OpLoad %v4float %47
%49 = OpCompositeExtract %float %48 0
OpStore %_9_x %49
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%52 = OpLoad %v4float %51
%53 = OpCompositeExtract %float %52 1
OpStore %_10_y %53
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 2
OpStore %_11_z %57
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%61 = OpCompositeExtract %float %60 3
OpStore %_12_w %61
%63 = OpLoad %float %_9_x
%64 = OpLoad %float %_10_y
%65 = OpLoad %float %_11_z
%66 = OpLoad %float %_12_w
%67 = OpCompositeConstruct %v4float %63 %64 %65 %66
OpStore %b %67
OpStore %_13_v %73
%75 = OpLoad %v4float %_13_v
%76 = OpCompositeExtract %float %75 0
OpStore %_14_x %76
%78 = OpLoad %v4float %_13_v
%79 = OpCompositeExtract %float %78 1
OpStore %_15_y %79
%81 = OpLoad %v4float %_13_v
%82 = OpCompositeExtract %float %81 2
OpStore %_16_z %82
%84 = OpLoad %v4float %_13_v
%85 = OpCompositeExtract %float %84 3
OpStore %_17_w %85
%87 = OpLoad %float %_14_x
%88 = OpLoad %float %_15_y
%89 = OpLoad %float %_16_z
%90 = OpLoad %float %_17_w
%91 = OpCompositeConstruct %v4float %87 %88 %89 %90
OpStore %c %91
%93 = OpLoad %v4float %a
%98 = OpFOrdEqual %v4bool %93 %97
%100 = OpAll %bool %98
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpLoad %v4float %b
%104 = OpFOrdEqual %v4bool %103 %97
%105 = OpAll %bool %104
OpBranch %102
%102 = OpLabel
%106 = OpPhi %bool %false %19 %105 %101
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4float %c
%110 = OpFOrdEqual %v4bool %109 %73
%111 = OpAll %bool %110
OpBranch %108
%108 = OpLabel
%112 = OpPhi %bool %false %102 %111 %107
OpSelectionMerge %116 None
OpBranchConditional %112 %114 %115
%114 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%119 = OpLoad %v4float %117
OpStore %113 %119
OpBranch %116
%115 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%122 = OpLoad %v4float %120
OpStore %113 %122
OpBranch %116
%116 = OpLabel
%123 = OpLoad %v4float %113
OpReturnValue %123
OpFunctionEnd
