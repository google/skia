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
OpDecorate %36 RelaxedPrecision
OpDecorate %_2_y RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %_3_z RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %_4_w RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %_9_x RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %_10_y RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %_11_z RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %_12_w RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %_13_v RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %_14_x RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %_15_y RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %_16_z RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %_17_w RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%78 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%102 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
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
%118 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
OpStore %_0_v %32
%35 = OpLoad %v4float %_0_v
%36 = OpCompositeExtract %float %35 0
OpStore %_1_x %36
%38 = OpLoad %v4float %_0_v
%39 = OpCompositeExtract %float %38 1
OpStore %_2_y %39
%41 = OpLoad %v4float %_0_v
%42 = OpCompositeExtract %float %41 2
OpStore %_3_z %42
%44 = OpLoad %v4float %_0_v
%45 = OpCompositeExtract %float %44 3
OpStore %_4_w %45
%47 = OpLoad %float %_1_x
%48 = OpLoad %float %_2_y
%49 = OpLoad %float %_3_z
%50 = OpLoad %float %_4_w
%51 = OpCompositeConstruct %v4float %47 %48 %49 %50
OpStore %a %51
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpCompositeExtract %float %54 0
OpStore %_9_x %55
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpCompositeExtract %float %58 1
OpStore %_10_y %59
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpCompositeExtract %float %62 2
OpStore %_11_z %63
%65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%66 = OpLoad %v4float %65
%67 = OpCompositeExtract %float %66 3
OpStore %_12_w %67
%69 = OpLoad %float %_9_x
%70 = OpLoad %float %_10_y
%71 = OpLoad %float %_11_z
%72 = OpLoad %float %_12_w
%73 = OpCompositeConstruct %v4float %69 %70 %71 %72
OpStore %b %73
OpStore %_13_v %78
%80 = OpLoad %v4float %_13_v
%81 = OpCompositeExtract %float %80 0
OpStore %_14_x %81
%83 = OpLoad %v4float %_13_v
%84 = OpCompositeExtract %float %83 1
OpStore %_15_y %84
%86 = OpLoad %v4float %_13_v
%87 = OpCompositeExtract %float %86 2
OpStore %_16_z %87
%89 = OpLoad %v4float %_13_v
%90 = OpCompositeExtract %float %89 3
OpStore %_17_w %90
%92 = OpLoad %float %_14_x
%93 = OpLoad %float %_15_y
%94 = OpLoad %float %_16_z
%95 = OpLoad %float %_17_w
%96 = OpCompositeConstruct %v4float %92 %93 %94 %95
OpStore %c %96
%98 = OpLoad %v4float %a
%103 = OpFOrdEqual %v4bool %98 %102
%105 = OpAll %bool %103
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %v4float %b
%109 = OpFOrdEqual %v4bool %108 %102
%110 = OpAll %bool %109
OpBranch %107
%107 = OpLabel
%111 = OpPhi %bool %false %25 %110 %106
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpLoad %v4float %c
%115 = OpFOrdEqual %v4bool %114 %78
%116 = OpAll %bool %115
OpBranch %113
%113 = OpLabel
%117 = OpPhi %bool %false %107 %116 %112
OpSelectionMerge %121 None
OpBranchConditional %117 %119 %120
%119 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%124 = OpLoad %v4float %122
OpStore %118 %124
OpBranch %121
%120 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%127 = OpLoad %v4float %125
OpStore %118 %127
OpBranch %121
%121 = OpLabel
%128 = OpLoad %v4float %118
OpReturnValue %128
OpFunctionEnd
