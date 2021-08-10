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
OpDecorate %39 RelaxedPrecision
OpDecorate %_arr_v3int_int_3 ArrayStride 16
OpDecorate %55 RelaxedPrecision
OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%_ptr_Function__arr_float_int_4_0 = OpTypePointer Function %_arr_float_int_4
%v3int = OpTypeVector %int 3
%int_3 = OpConstant %int 3
%_arr_v3int_int_3 = OpTypeArray %v3int %int_3
%_ptr_Function__arr_v3int_int_3 = OpTypePointer Function %_arr_v3int_int_3
%int_1 = OpConstant %int 1
%47 = OpConstantComposite %v3int %int_1 %int_1 %int_1
%int_2 = OpConstant %int 2
%49 = OpConstantComposite %v3int %int_2 %int_2 %int_2
%50 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%_ptr_Function__arr_v3int_int_3_0 = OpTypePointer Function %_arr_v3int_int_3
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2
%_ptr_Function__arr_mat2v2float_int_2 = OpTypePointer Function %_arr_mat2v2float_int_2
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%_ptr_Function__arr_mat2v2float_int_2_0 = OpTypePointer Function %_arr_mat2v2float_int_2
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
%h = OpVariable %_ptr_Function__arr_float_int_4_0 Function
%i3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_v3int_int_3_0 Function
%h2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
%f2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2_0 Function
%144 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f %35
%38 = OpLoad %_arr_float_int_4 %f
OpStore %h %38
%39 = OpLoad %_arr_float_int_4 %h
OpStore %f %39
%40 = OpLoad %_arr_float_int_4 %f
OpStore %h %40
%51 = OpCompositeConstruct %_arr_v3int_int_3 %47 %49 %50
OpStore %i3 %51
%54 = OpLoad %_arr_v3int_int_3 %i3
OpStore %s3 %54
%55 = OpLoad %_arr_v3int_int_3 %s3
OpStore %i3 %55
%56 = OpLoad %_arr_v3int_int_3 %i3
OpStore %s3 %56
%61 = OpCompositeConstruct %v2float %float_1 %float_2
%62 = OpCompositeConstruct %v2float %float_3 %float_4
%63 = OpCompositeConstruct %mat2v2float %61 %62
%68 = OpCompositeConstruct %v2float %float_5 %float_6
%69 = OpCompositeConstruct %v2float %float_7 %float_8
%70 = OpCompositeConstruct %mat2v2float %68 %69
%71 = OpCompositeConstruct %_arr_mat2v2float_int_2 %63 %70
OpStore %h2x2 %71
%74 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %74
%75 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %75
%76 = OpLoad %_arr_mat2v2float_int_2 %f2x2
OpStore %h2x2 %76
%78 = OpLoad %_arr_float_int_4 %f
%79 = OpLoad %_arr_float_int_4 %h
%80 = OpCompositeExtract %float %78 0
%81 = OpCompositeExtract %float %79 0
%82 = OpFOrdEqual %bool %80 %81
%83 = OpCompositeExtract %float %78 1
%84 = OpCompositeExtract %float %79 1
%85 = OpFOrdEqual %bool %83 %84
%86 = OpLogicalAnd %bool %85 %82
%87 = OpCompositeExtract %float %78 2
%88 = OpCompositeExtract %float %79 2
%89 = OpFOrdEqual %bool %87 %88
%90 = OpLogicalAnd %bool %89 %86
%91 = OpCompositeExtract %float %78 3
%92 = OpCompositeExtract %float %79 3
%93 = OpFOrdEqual %bool %91 %92
%94 = OpLogicalAnd %bool %93 %90
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %_arr_v3int_int_3 %i3
%98 = OpLoad %_arr_v3int_int_3 %s3
%99 = OpCompositeExtract %v3int %97 0
%100 = OpCompositeExtract %v3int %98 0
%101 = OpIEqual %v3bool %99 %100
%103 = OpAll %bool %101
%104 = OpCompositeExtract %v3int %97 1
%105 = OpCompositeExtract %v3int %98 1
%106 = OpIEqual %v3bool %104 %105
%107 = OpAll %bool %106
%108 = OpLogicalAnd %bool %107 %103
%109 = OpCompositeExtract %v3int %97 2
%110 = OpCompositeExtract %v3int %98 2
%111 = OpIEqual %v3bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %112 %108
OpBranch %96
%96 = OpLabel
%114 = OpPhi %bool %false %25 %113 %95
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpLoad %_arr_mat2v2float_int_2 %f2x2
%118 = OpLoad %_arr_mat2v2float_int_2 %h2x2
%119 = OpCompositeExtract %mat2v2float %117 0
%120 = OpCompositeExtract %mat2v2float %118 0
%122 = OpCompositeExtract %v2float %119 0
%123 = OpCompositeExtract %v2float %120 0
%124 = OpFOrdEqual %v2bool %122 %123
%125 = OpAll %bool %124
%126 = OpCompositeExtract %v2float %119 1
%127 = OpCompositeExtract %v2float %120 1
%128 = OpFOrdEqual %v2bool %126 %127
%129 = OpAll %bool %128
%130 = OpLogicalAnd %bool %125 %129
%131 = OpCompositeExtract %mat2v2float %117 1
%132 = OpCompositeExtract %mat2v2float %118 1
%133 = OpCompositeExtract %v2float %131 0
%134 = OpCompositeExtract %v2float %132 0
%135 = OpFOrdEqual %v2bool %133 %134
%136 = OpAll %bool %135
%137 = OpCompositeExtract %v2float %131 1
%138 = OpCompositeExtract %v2float %132 1
%139 = OpFOrdEqual %v2bool %137 %138
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %136 %140
%142 = OpLogicalAnd %bool %141 %130
OpBranch %116
%116 = OpLabel
%143 = OpPhi %bool %false %96 %142 %115
OpSelectionMerge %148 None
OpBranchConditional %143 %146 %147
%146 = OpLabel
%149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%152 = OpLoad %v4float %149
OpStore %144 %152
OpBranch %148
%147 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%154 = OpLoad %v4float %153
OpStore %144 %154
OpBranch %148
%148 = OpLabel
%155 = OpLoad %v4float %144
OpReturnValue %155
OpFunctionEnd
