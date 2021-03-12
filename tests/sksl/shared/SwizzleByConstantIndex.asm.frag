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
OpName %_5_ZERO "_5_ZERO"
OpName %_6_ONE "_6_ONE"
OpName %_7_TWO "_7_TWO"
OpName %_8_THREE "_8_THREE"
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
OpDecorate %sk_Clockwise RelaxedPrecision
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
OpDecorate %56 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
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
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%81 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%105 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
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
%_5_ZERO = OpVariable %_ptr_Function_int Function
%_6_ONE = OpVariable %_ptr_Function_int Function
%_7_TWO = OpVariable %_ptr_Function_int Function
%_8_THREE = OpVariable %_ptr_Function_int Function
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
%121 = OpVariable %_ptr_Function_v4float Function
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
OpStore %_5_ZERO %int_0
OpStore %_6_ONE %int_1
OpStore %_7_TWO %int_2
OpStore %_8_THREE %int_3
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 0
OpStore %_9_x %57
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%61 = OpCompositeExtract %float %60 1
OpStore %_10_y %61
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%64 = OpLoad %v4float %63
%65 = OpCompositeExtract %float %64 2
OpStore %_11_z %65
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%68 = OpLoad %v4float %67
%69 = OpCompositeExtract %float %68 3
OpStore %_12_w %69
%71 = OpLoad %float %_9_x
%72 = OpLoad %float %_10_y
%73 = OpLoad %float %_11_z
%74 = OpLoad %float %_12_w
%75 = OpCompositeConstruct %v4float %71 %72 %73 %74
OpStore %b %75
OpStore %_13_v %81
%83 = OpLoad %v4float %_13_v
%84 = OpCompositeExtract %float %83 0
OpStore %_14_x %84
%86 = OpLoad %v4float %_13_v
%87 = OpCompositeExtract %float %86 1
OpStore %_15_y %87
%89 = OpLoad %v4float %_13_v
%90 = OpCompositeExtract %float %89 2
OpStore %_16_z %90
%92 = OpLoad %v4float %_13_v
%93 = OpCompositeExtract %float %92 3
OpStore %_17_w %93
%95 = OpLoad %float %_14_x
%96 = OpLoad %float %_15_y
%97 = OpLoad %float %_16_z
%98 = OpLoad %float %_17_w
%99 = OpCompositeConstruct %v4float %95 %96 %97 %98
OpStore %c %99
%101 = OpLoad %v4float %a
%106 = OpFOrdEqual %v4bool %101 %105
%108 = OpAll %bool %106
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %v4float %b
%112 = OpFOrdEqual %v4bool %111 %105
%113 = OpAll %bool %112
OpBranch %110
%110 = OpLabel
%114 = OpPhi %bool %false %19 %113 %109
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpLoad %v4float %c
%118 = OpFOrdEqual %v4bool %117 %81
%119 = OpAll %bool %118
OpBranch %116
%116 = OpLabel
%120 = OpPhi %bool %false %110 %119 %115
OpSelectionMerge %124 None
OpBranchConditional %120 %122 %123
%122 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%126 = OpLoad %v4float %125
OpStore %121 %126
OpBranch %124
%123 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%128 = OpLoad %v4float %127
OpStore %121 %128
OpBranch %124
%124 = OpLabel
%129 = OpLoad %v4float %121
OpReturnValue %129
OpFunctionEnd
