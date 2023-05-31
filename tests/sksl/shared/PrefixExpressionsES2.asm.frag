OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpName %i "i"
OpName %f "f"
OpName %f2 "f2"
OpName %iv "iv"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %94 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_5 = OpConstant %int 5
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%_ptr_Function_float = OpTypePointer Function %float
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_2_5 = OpConstant %float 2.5
%89 = OpConstantComposite %v2float %float_0_5 %float_0_5
%int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_n1 = OpConstant %float -1
%145 = OpConstantComposite %v4float %float_0 %float_n1 %float_0 %float_n1
%v4bool = OpTypeVector %bool 4
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%158 = OpConstantComposite %v2float %float_n1 %float_n2
%159 = OpConstantComposite %v2float %float_n3 %float_n4
%160 = OpConstantComposite %mat2v2float %158 %159
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_2 = OpConstant %int 2
%v2bool = OpTypeVector %bool 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%int_n5 = OpConstant %int -5
%194 = OpConstantComposite %v2int %int_n5 %int_5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%i = OpVariable %_ptr_Function_int Function
%f = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_v2float Function
%iv = OpVariable %_ptr_Function_v2int Function
%198 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
OpStore %i %int_5
%35 = OpIAdd %int %int_5 %int_1
OpStore %i %35
OpSelectionMerge %38 None
OpBranchConditional %true %37 %38
%37 = OpLabel
%40 = OpIEqual %bool %35 %int_6
OpBranch %38
%38 = OpLabel
%41 = OpPhi %bool %false %26 %40 %37
OpStore %ok %41
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpIAdd %int %35 %int_1
OpStore %i %44
%46 = OpIEqual %bool %44 %int_7
OpBranch %43
%43 = OpLabel
%47 = OpPhi %bool %false %38 %46 %42
OpStore %ok %47
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%50 = OpLoad %int %i
%51 = OpISub %int %50 %int_1
OpStore %i %51
%52 = OpIEqual %bool %51 %int_6
OpBranch %49
%49 = OpLabel
%53 = OpPhi %bool %false %43 %52 %48
OpStore %ok %53
%54 = OpLoad %int %i
%55 = OpISub %int %54 %int_1
OpStore %i %55
OpSelectionMerge %57 None
OpBranchConditional %53 %56 %57
%56 = OpLabel
%58 = OpIEqual %bool %55 %int_5
OpBranch %57
%57 = OpLabel
%59 = OpPhi %bool %false %49 %58 %56
OpStore %ok %59
OpStore %f %float_0_5
%64 = OpFAdd %float %float_0_5 %float_1
OpStore %f %64
OpSelectionMerge %66 None
OpBranchConditional %59 %65 %66
%65 = OpLabel
%68 = OpFOrdEqual %bool %64 %float_1_5
OpBranch %66
%66 = OpLabel
%69 = OpPhi %bool %false %57 %68 %65
OpStore %ok %69
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpFAdd %float %64 %float_1
OpStore %f %72
%74 = OpFOrdEqual %bool %72 %float_2_5
OpBranch %71
%71 = OpLabel
%75 = OpPhi %bool %false %66 %74 %70
OpStore %ok %75
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpLoad %float %f
%79 = OpFSub %float %78 %float_1
OpStore %f %79
%80 = OpFOrdEqual %bool %79 %float_1_5
OpBranch %77
%77 = OpLabel
%81 = OpPhi %bool %false %71 %80 %76
OpStore %ok %81
%82 = OpLoad %float %f
%83 = OpFSub %float %82 %float_1
OpStore %f %83
OpSelectionMerge %85 None
OpBranchConditional %81 %84 %85
%84 = OpLabel
%86 = OpFOrdEqual %bool %83 %float_0_5
OpBranch %85
%85 = OpLabel
%87 = OpPhi %bool %false %77 %86 %84
OpStore %ok %87
OpStore %f2 %89
%90 = OpAccessChain %_ptr_Function_float %f2 %int_0
%92 = OpLoad %float %90
%93 = OpFAdd %float %92 %float_1
OpStore %90 %93
%94 = OpLoad %bool %ok
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %v2float %f2
%98 = OpCompositeExtract %float %97 0
%99 = OpFOrdEqual %bool %98 %float_1_5
OpBranch %96
%96 = OpLabel
%100 = OpPhi %bool %false %85 %99 %95
OpStore %ok %100
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpAccessChain %_ptr_Function_float %f2 %int_0
%104 = OpLoad %float %103
%105 = OpFAdd %float %104 %float_1
OpStore %103 %105
%106 = OpFOrdEqual %bool %105 %float_2_5
OpBranch %102
%102 = OpLabel
%107 = OpPhi %bool %false %96 %106 %101
OpStore %ok %107
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpAccessChain %_ptr_Function_float %f2 %int_0
%111 = OpLoad %float %110
%112 = OpFSub %float %111 %float_1
OpStore %110 %112
%113 = OpFOrdEqual %bool %112 %float_1_5
OpBranch %109
%109 = OpLabel
%114 = OpPhi %bool %false %102 %113 %108
OpStore %ok %114
%115 = OpAccessChain %_ptr_Function_float %f2 %int_0
%116 = OpLoad %float %115
%117 = OpFSub %float %116 %float_1
OpStore %115 %117
%118 = OpLoad %bool %ok
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%121 = OpLoad %v2float %f2
%122 = OpCompositeExtract %float %121 0
%123 = OpFOrdEqual %bool %122 %float_0_5
OpBranch %120
%120 = OpLabel
%124 = OpPhi %bool %false %109 %123 %119
OpStore %ok %124
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%130 = OpLoad %v4float %128
%131 = OpCompositeExtract %float %130 0
%132 = OpFOrdEqual %bool %131 %float_1
%127 = OpLogicalNot %bool %132
OpBranch %126
%126 = OpLabel
%133 = OpPhi %bool %false %120 %127 %125
OpStore %ok %133
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 1
%140 = OpFNegate %float %139
%141 = OpFOrdEqual %bool %float_n1 %140
OpBranch %135
%135 = OpLabel
%142 = OpPhi %bool %false %126 %141 %134
OpStore %ok %142
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%147 = OpLoad %v4float %146
%148 = OpFNegate %v4float %147
%149 = OpFOrdEqual %v4bool %145 %148
%151 = OpAll %bool %149
OpBranch %144
%144 = OpLabel
%152 = OpPhi %bool %false %135 %151 %143
OpStore %ok %152
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%161 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%164 = OpLoad %mat2v2float %161
%165 = OpCompositeExtract %v2float %164 0
%166 = OpFNegate %v2float %165
%167 = OpCompositeExtract %v2float %164 1
%168 = OpFNegate %v2float %167
%169 = OpCompositeConstruct %mat2v2float %166 %168
%171 = OpFOrdEqual %v2bool %158 %166
%172 = OpAll %bool %171
%173 = OpFOrdEqual %v2bool %159 %168
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %172 %174
OpBranch %154
%154 = OpLabel
%176 = OpPhi %bool %false %144 %175 %153
OpStore %ok %176
%180 = OpLoad %int %i
%181 = OpLoad %int %i
%182 = OpSNegate %int %181
%183 = OpCompositeConstruct %v2int %180 %182
OpStore %iv %183
OpSelectionMerge %185 None
OpBranchConditional %176 %184 %185
%184 = OpLabel
%186 = OpLoad %int %i
%187 = OpSNegate %int %186
%189 = OpIEqual %bool %187 %int_n5
OpBranch %185
%185 = OpLabel
%190 = OpPhi %bool %false %154 %189 %184
OpStore %ok %190
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpSNegate %v2int %183
%195 = OpIEqual %v2bool %193 %194
%196 = OpAll %bool %195
OpBranch %192
%192 = OpLabel
%197 = OpPhi %bool %false %185 %196 %191
OpStore %ok %197
OpSelectionMerge %202 None
OpBranchConditional %197 %200 %201
%200 = OpLabel
%203 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%204 = OpLoad %v4float %203
OpStore %198 %204
OpBranch %202
%201 = OpLabel
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%206 = OpLoad %v4float %205
OpStore %198 %206
OpBranch %202
%202 = OpLabel
%207 = OpLoad %v4float %198
OpReturnValue %207
OpFunctionEnd
