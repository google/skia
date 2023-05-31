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
OpName %val "val"
OpName %mask "mask"
OpName %imask "imask"
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
OpDecorate %95 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%uint_0 = OpConstant %uint 0
%130 = OpConstantComposite %v2uint %uint_0 %uint_0
%v2bool = OpTypeVector %bool 2
%float_n1 = OpConstant %float -1
%146 = OpConstantComposite %v4float %float_0 %float_n1 %float_0 %float_n1
%v4bool = OpTypeVector %bool 4
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%159 = OpConstantComposite %v2float %float_n1 %float_n2
%160 = OpConstantComposite %v2float %float_n3 %float_n4
%161 = OpConstantComposite %mat2v2float %159 %160
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_2 = OpConstant %int 2
%int_n5 = OpConstant %int -5
%189 = OpConstantComposite %v2int %int_n5 %int_5
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
%val = OpVariable %_ptr_Function_uint Function
%mask = OpVariable %_ptr_Function_v2uint Function
%imask = OpVariable %_ptr_Function_v2int Function
%iv = OpVariable %_ptr_Function_v2int Function
%193 = OpVariable %_ptr_Function_v4float Function
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
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%94 = OpLoad %v4float %91
%95 = OpCompositeExtract %float %94 0
%96 = OpFOrdEqual %bool %95 %float_1
%90 = OpLogicalNot %bool %96
OpBranch %89
%89 = OpLabel
%97 = OpPhi %bool %false %85 %90 %88
OpStore %ok %97
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%102 = OpLoad %v4float %101
%103 = OpCompositeExtract %float %102 0
%104 = OpConvertFToU %uint %103
OpStore %val %104
%108 = OpNot %uint %104
%109 = OpCompositeConstruct %v2uint %104 %108
OpStore %mask %109
%113 = OpNot %v2uint %109
%114 = OpCompositeExtract %uint %113 0
%115 = OpBitcast %int %114
%116 = OpCompositeExtract %uint %113 1
%117 = OpBitcast %int %116
%118 = OpCompositeConstruct %v2int %115 %117
OpStore %imask %118
%119 = OpNot %v2uint %109
%120 = OpNot %v2int %118
%121 = OpCompositeExtract %int %120 0
%122 = OpBitcast %uint %121
%123 = OpCompositeExtract %int %120 1
%124 = OpBitcast %uint %123
%125 = OpCompositeConstruct %v2uint %122 %124
%126 = OpBitwiseAnd %v2uint %119 %125
OpStore %mask %126
OpSelectionMerge %128 None
OpBranchConditional %97 %127 %128
%127 = OpLabel
%131 = OpIEqual %v2bool %126 %130
%133 = OpAll %bool %131
OpBranch %128
%128 = OpLabel
%134 = OpPhi %bool %false %89 %133 %127
OpStore %ok %134
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 1
%141 = OpFNegate %float %140
%142 = OpFOrdEqual %bool %float_n1 %141
OpBranch %136
%136 = OpLabel
%143 = OpPhi %bool %false %128 %142 %135
OpStore %ok %143
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%147 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%148 = OpLoad %v4float %147
%149 = OpFNegate %v4float %148
%150 = OpFOrdEqual %v4bool %146 %149
%152 = OpAll %bool %150
OpBranch %145
%145 = OpLabel
%153 = OpPhi %bool %false %136 %152 %144
OpStore %ok %153
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%165 = OpLoad %mat2v2float %162
%166 = OpCompositeExtract %v2float %165 0
%167 = OpFNegate %v2float %166
%168 = OpCompositeExtract %v2float %165 1
%169 = OpFNegate %v2float %168
%170 = OpCompositeConstruct %mat2v2float %167 %169
%171 = OpFOrdEqual %v2bool %159 %167
%172 = OpAll %bool %171
%173 = OpFOrdEqual %v2bool %160 %169
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %172 %174
OpBranch %155
%155 = OpLabel
%176 = OpPhi %bool %false %145 %175 %154
OpStore %ok %176
%178 = OpSNegate %int %55
%179 = OpCompositeConstruct %v2int %55 %178
OpStore %iv %179
OpSelectionMerge %181 None
OpBranchConditional %176 %180 %181
%180 = OpLabel
%182 = OpSNegate %int %55
%184 = OpIEqual %bool %182 %int_n5
OpBranch %181
%181 = OpLabel
%185 = OpPhi %bool %false %155 %184 %180
OpStore %ok %185
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpSNegate %v2int %179
%190 = OpIEqual %v2bool %188 %189
%191 = OpAll %bool %190
OpBranch %187
%187 = OpLabel
%192 = OpPhi %bool %false %181 %191 %186
OpStore %ok %192
OpSelectionMerge %197 None
OpBranchConditional %192 %195 %196
%195 = OpLabel
%198 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%199 = OpLoad %v4float %198
OpStore %193 %199
OpBranch %197
%196 = OpLabel
%200 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%201 = OpLoad %v4float %200
OpStore %193 %201
OpBranch %197
%197 = OpLabel
%202 = OpLoad %v4float %193
OpReturnValue %202
OpFunctionEnd
