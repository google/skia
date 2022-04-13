OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f "f"
OpName %h "h"
OpName %i3 "i3"
OpName %s3 "s3"
OpName %h2x2 "h2x2"
OpName %f2x2 "f2x2"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %h RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %_arr_v3int_int_3 ArrayStride 16
OpDecorate %53 RelaxedPrecision
OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v3int = OpTypeVector %int 3
%int_3 = OpConstant %int 3
%_arr_v3int_int_3 = OpTypeArray %v3int %int_3
%_ptr_Function__arr_v3int_int_3 = OpTypePointer Function %_arr_v3int_int_3
%int_1 = OpConstant %int 1
%46 = OpConstantComposite %v3int %int_1 %int_1 %int_1
%int_2 = OpConstant %int 2
%48 = OpConstantComposite %v3int %int_2 %int_2 %int_2
%49 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2
%_ptr_Function__arr_mat2v2float_int_2 = OpTypePointer Function %_arr_mat2v2float_int_2
%59 = OpConstantComposite %v2float %float_1 %float_2
%60 = OpConstantComposite %v2float %float_3 %float_4
%61 = OpConstantComposite %mat2v2float %59 %60
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%66 = OpConstantComposite %v2float %float_5 %float_6
%67 = OpConstantComposite %v2float %float_7 %float_8
%68 = OpConstantComposite %mat2v2float %66 %67
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%f = OpVariable %_ptr_Function__arr_float_int_4 Function
%h = OpVariable %_ptr_Function__arr_float_int_4 Function
%i3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
%h2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
%f2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
%141 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f %35
%37 = OpLoad %_arr_float_int_4 %f
OpStore %h %37
%38 = OpLoad %_arr_float_int_4 %h
OpStore %f %38
%39 = OpLoad %_arr_float_int_4 %f
OpStore %h %39
%50 = OpCompositeConstruct %_arr_v3int_int_3 %46 %48 %49
OpStore %i3 %50
%52 = OpLoad %_arr_v3int_int_3 %i3
OpStore %s3 %52
%53 = OpLoad %_arr_v3int_int_3 %s3
OpStore %i3 %53
%54 = OpLoad %_arr_v3int_int_3 %i3
OpStore %s3 %54
%69 = OpCompositeConstruct %_arr_mat2v2float_int_2 %61 %68
OpStore %h2x2 %69
%71 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %71
%72 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %72
%73 = OpLoad %_arr_mat2v2float_int_2 %f2x2
OpStore %h2x2 %73
%75 = OpLoad %_arr_float_int_4 %f
%76 = OpLoad %_arr_float_int_4 %h
%77 = OpCompositeExtract %float %75 0
%78 = OpCompositeExtract %float %76 0
%79 = OpFOrdEqual %bool %77 %78
%80 = OpCompositeExtract %float %75 1
%81 = OpCompositeExtract %float %76 1
%82 = OpFOrdEqual %bool %80 %81
%83 = OpLogicalAnd %bool %82 %79
%84 = OpCompositeExtract %float %75 2
%85 = OpCompositeExtract %float %76 2
%86 = OpFOrdEqual %bool %84 %85
%87 = OpLogicalAnd %bool %86 %83
%88 = OpCompositeExtract %float %75 3
%89 = OpCompositeExtract %float %76 3
%90 = OpFOrdEqual %bool %88 %89
%91 = OpLogicalAnd %bool %90 %87
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %_arr_v3int_int_3 %i3
%95 = OpLoad %_arr_v3int_int_3 %s3
%96 = OpCompositeExtract %v3int %94 0
%97 = OpCompositeExtract %v3int %95 0
%98 = OpIEqual %v3bool %96 %97
%100 = OpAll %bool %98
%101 = OpCompositeExtract %v3int %94 1
%102 = OpCompositeExtract %v3int %95 1
%103 = OpIEqual %v3bool %101 %102
%104 = OpAll %bool %103
%105 = OpLogicalAnd %bool %104 %100
%106 = OpCompositeExtract %v3int %94 2
%107 = OpCompositeExtract %v3int %95 2
%108 = OpIEqual %v3bool %106 %107
%109 = OpAll %bool %108
%110 = OpLogicalAnd %bool %109 %105
OpBranch %93
%93 = OpLabel
%111 = OpPhi %bool %false %25 %110 %92
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpLoad %_arr_mat2v2float_int_2 %f2x2
%115 = OpLoad %_arr_mat2v2float_int_2 %h2x2
%116 = OpCompositeExtract %mat2v2float %114 0
%117 = OpCompositeExtract %mat2v2float %115 0
%119 = OpCompositeExtract %v2float %116 0
%120 = OpCompositeExtract %v2float %117 0
%121 = OpFOrdEqual %v2bool %119 %120
%122 = OpAll %bool %121
%123 = OpCompositeExtract %v2float %116 1
%124 = OpCompositeExtract %v2float %117 1
%125 = OpFOrdEqual %v2bool %123 %124
%126 = OpAll %bool %125
%127 = OpLogicalAnd %bool %122 %126
%128 = OpCompositeExtract %mat2v2float %114 1
%129 = OpCompositeExtract %mat2v2float %115 1
%130 = OpCompositeExtract %v2float %128 0
%131 = OpCompositeExtract %v2float %129 0
%132 = OpFOrdEqual %v2bool %130 %131
%133 = OpAll %bool %132
%134 = OpCompositeExtract %v2float %128 1
%135 = OpCompositeExtract %v2float %129 1
%136 = OpFOrdEqual %v2bool %134 %135
%137 = OpAll %bool %136
%138 = OpLogicalAnd %bool %133 %137
%139 = OpLogicalAnd %bool %138 %127
OpBranch %113
%113 = OpLabel
%140 = OpPhi %bool %false %93 %139 %112
OpSelectionMerge %145 None
OpBranchConditional %140 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%149 = OpLoad %v4float %146
OpStore %141 %149
OpBranch %145
%144 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%151 = OpLoad %v4float %150
OpStore %141 %151
OpBranch %145
%145 = OpLabel
%152 = OpLoad %v4float %141
OpReturnValue %152
OpFunctionEnd
