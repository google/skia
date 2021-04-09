OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %returns_a_struct_S "returns_a_struct_S"
OpName %s "s"
OpName %constructs_a_struct_S "constructs_a_struct_S"
OpName %accepts_a_struct_fS "accepts_a_struct_fS"
OpName %modifies_a_struct_vS "modifies_a_struct_vS"
OpName %main "main"
OpName %s_0 "s"
OpName %x "x"
OpName %expected "expected"
OpName %Nested "Nested"
OpMemberName %Nested 0 "a"
OpMemberName %Nested 1 "b"
OpName %n1 "n1"
OpName %n2 "n2"
OpName %n3 "n3"
OpName %valid "valid"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %36 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpMemberDecorate %Nested 0 Offset 0
OpMemberDecorate %Nested 0 RelaxedPrecision
OpMemberDecorate %Nested 1 Offset 16
OpMemberDecorate %Nested 1 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%int = OpTypeInt 32 1
%S = OpTypeStruct %float %int
%24 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%41 = OpTypeFunction %float %_ptr_Function_S
%50 = OpTypeFunction %void %_ptr_Function_S
%59 = OpTypeFunction %v4float
%Nested = OpTypeStruct %S %S
%_ptr_Function_Nested = OpTypePointer Function %Nested
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%21 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%returns_a_struct_S = OpFunction %S None %24
%25 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%30 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %30 %float_1
%34 = OpAccessChain %_ptr_Function_int %s %int_1
OpStore %34 %int_2
%36 = OpLoad %S %s
OpReturnValue %36
OpFunctionEnd
%constructs_a_struct_S = OpFunction %S None %24
%37 = OpLabel
%40 = OpCompositeConstruct %S %float_2 %int_3
OpReturnValue %40
OpFunctionEnd
%accepts_a_struct_fS = OpFunction %float None %41
%42 = OpFunctionParameter %_ptr_Function_S
%43 = OpLabel
%44 = OpAccessChain %_ptr_Function_float %42 %int_0
%45 = OpLoad %float %44
%46 = OpAccessChain %_ptr_Function_int %42 %int_1
%47 = OpLoad %int %46
%48 = OpConvertSToF %float %47
%49 = OpFAdd %float %45 %48
OpReturnValue %49
OpFunctionEnd
%modifies_a_struct_vS = OpFunction %void None %50
%51 = OpFunctionParameter %_ptr_Function_S
%52 = OpLabel
%53 = OpAccessChain %_ptr_Function_float %51 %int_0
%54 = OpLoad %float %53
%55 = OpFAdd %float %54 %float_1
OpStore %53 %55
%56 = OpAccessChain %_ptr_Function_int %51 %int_1
%57 = OpLoad %int %56
%58 = OpIAdd %int %57 %int_1
OpStore %56 %58
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %59
%60 = OpLabel
%s_0 = OpVariable %_ptr_Function_S Function
%x = OpVariable %_ptr_Function_float Function
%65 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%81 = OpVariable %_ptr_Function_S Function
%valid = OpVariable %_ptr_Function_bool Function
%213 = OpVariable %_ptr_Function_v4float Function
%62 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %62
%64 = OpLoad %S %s_0
OpStore %65 %64
%66 = OpFunctionCall %float %accepts_a_struct_fS %65
OpStore %x %66
%67 = OpFunctionCall %void %modifies_a_struct_vS %s_0
%69 = OpFunctionCall %S %constructs_a_struct_S
OpStore %expected %69
%75 = OpFunctionCall %S %returns_a_struct_S
%76 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %76 %75
%77 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %77 %75
%78 = OpLoad %Nested %n1
OpStore %n2 %78
OpStore %n3 %78
%79 = OpAccessChain %_ptr_Function_S %n3 %int_1
%80 = OpLoad %S %79
OpStore %81 %80
%82 = OpFunctionCall %void %modifies_a_struct_vS %81
%83 = OpLoad %S %81
OpStore %79 %83
%87 = OpLoad %float %x
%89 = OpFOrdEqual %bool %87 %float_3
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%93 = OpLoad %float %92
%94 = OpFOrdEqual %bool %93 %float_2
OpBranch %91
%91 = OpLabel
%95 = OpPhi %bool %false %60 %94 %90
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%99 = OpLoad %int %98
%100 = OpIEqual %bool %99 %int_3
OpBranch %97
%97 = OpLabel
%101 = OpPhi %bool %false %91 %100 %96
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpLoad %S %s_0
%105 = OpLoad %S %expected
%106 = OpCompositeExtract %float %104 0
%107 = OpCompositeExtract %float %105 0
%108 = OpFOrdEqual %bool %106 %107
%109 = OpCompositeExtract %int %104 1
%110 = OpCompositeExtract %int %105 1
%111 = OpIEqual %bool %109 %110
%112 = OpLogicalAnd %bool %111 %108
OpBranch %103
%103 = OpLabel
%113 = OpPhi %bool %false %97 %112 %102
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpLoad %S %s_0
%117 = OpCompositeConstruct %S %float_2 %int_3
%118 = OpCompositeExtract %float %116 0
%119 = OpCompositeExtract %float %117 0
%120 = OpFOrdEqual %bool %118 %119
%121 = OpCompositeExtract %int %116 1
%122 = OpCompositeExtract %int %117 1
%123 = OpIEqual %bool %121 %122
%124 = OpLogicalAnd %bool %123 %120
OpBranch %115
%115 = OpLabel
%125 = OpPhi %bool %false %103 %124 %114
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpLoad %S %s_0
%129 = OpFunctionCall %S %returns_a_struct_S
%130 = OpCompositeExtract %float %128 0
%131 = OpCompositeExtract %float %129 0
%132 = OpFOrdNotEqual %bool %130 %131
%133 = OpCompositeExtract %int %128 1
%134 = OpCompositeExtract %int %129 1
%135 = OpINotEqual %bool %133 %134
%136 = OpLogicalOr %bool %135 %132
OpBranch %127
%127 = OpLabel
%137 = OpPhi %bool %false %115 %136 %126
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%140 = OpLoad %Nested %n1
%141 = OpLoad %Nested %n2
%142 = OpCompositeExtract %S %140 0
%143 = OpCompositeExtract %S %141 0
%144 = OpCompositeExtract %float %142 0
%145 = OpCompositeExtract %float %143 0
%146 = OpFOrdEqual %bool %144 %145
%147 = OpCompositeExtract %int %142 1
%148 = OpCompositeExtract %int %143 1
%149 = OpIEqual %bool %147 %148
%150 = OpLogicalAnd %bool %149 %146
%151 = OpCompositeExtract %S %140 1
%152 = OpCompositeExtract %S %141 1
%153 = OpCompositeExtract %float %151 0
%154 = OpCompositeExtract %float %152 0
%155 = OpFOrdEqual %bool %153 %154
%156 = OpCompositeExtract %int %151 1
%157 = OpCompositeExtract %int %152 1
%158 = OpIEqual %bool %156 %157
%159 = OpLogicalAnd %bool %158 %155
%160 = OpLogicalAnd %bool %159 %150
OpBranch %139
%139 = OpLabel
%161 = OpPhi %bool %false %127 %160 %138
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %Nested %n1
%165 = OpLoad %Nested %n3
%166 = OpCompositeExtract %S %164 0
%167 = OpCompositeExtract %S %165 0
%168 = OpCompositeExtract %float %166 0
%169 = OpCompositeExtract %float %167 0
%170 = OpFOrdNotEqual %bool %168 %169
%171 = OpCompositeExtract %int %166 1
%172 = OpCompositeExtract %int %167 1
%173 = OpINotEqual %bool %171 %172
%174 = OpLogicalOr %bool %173 %170
%175 = OpCompositeExtract %S %164 1
%176 = OpCompositeExtract %S %165 1
%177 = OpCompositeExtract %float %175 0
%178 = OpCompositeExtract %float %176 0
%179 = OpFOrdNotEqual %bool %177 %178
%180 = OpCompositeExtract %int %175 1
%181 = OpCompositeExtract %int %176 1
%182 = OpINotEqual %bool %180 %181
%183 = OpLogicalOr %bool %182 %179
%184 = OpLogicalOr %bool %183 %174
OpBranch %163
%163 = OpLabel
%185 = OpPhi %bool %false %139 %184 %162
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpLoad %Nested %n3
%189 = OpCompositeConstruct %S %float_1 %int_2
%190 = OpCompositeConstruct %S %float_2 %int_3
%191 = OpCompositeConstruct %Nested %189 %190
%192 = OpCompositeExtract %S %188 0
%193 = OpCompositeExtract %S %191 0
%194 = OpCompositeExtract %float %192 0
%195 = OpCompositeExtract %float %193 0
%196 = OpFOrdEqual %bool %194 %195
%197 = OpCompositeExtract %int %192 1
%198 = OpCompositeExtract %int %193 1
%199 = OpIEqual %bool %197 %198
%200 = OpLogicalAnd %bool %199 %196
%201 = OpCompositeExtract %S %188 1
%202 = OpCompositeExtract %S %191 1
%203 = OpCompositeExtract %float %201 0
%204 = OpCompositeExtract %float %202 0
%205 = OpFOrdEqual %bool %203 %204
%206 = OpCompositeExtract %int %201 1
%207 = OpCompositeExtract %int %202 1
%208 = OpIEqual %bool %206 %207
%209 = OpLogicalAnd %bool %208 %205
%210 = OpLogicalAnd %bool %209 %200
OpBranch %187
%187 = OpLabel
%211 = OpPhi %bool %false %163 %210 %186
OpStore %valid %211
%212 = OpLoad %bool %valid
OpSelectionMerge %217 None
OpBranchConditional %212 %215 %216
%215 = OpLabel
%218 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%220 = OpLoad %v4float %218
OpStore %213 %220
OpBranch %217
%216 = OpLabel
%221 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%222 = OpLoad %v4float %221
OpStore %213 %222
OpBranch %217
%217 = OpLabel
%223 = OpLoad %v4float %213
OpReturnValue %223
OpFunctionEnd
