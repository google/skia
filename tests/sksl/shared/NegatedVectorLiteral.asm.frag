OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %test_int "test_int"
OpName %one "one"
OpName %two "two"
OpName %result "result"
OpName %main "main"
OpName %_0_test_float "_0_test_float"
OpName %_1_one "_1_one"
OpName %_2_two "_2_two"
OpName %_3_result "_3_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %116 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%19 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_n1 = OpConstant %int -1
%31 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n1 %int_n1
%v2int = OpTypeVector %int 2
%35 = OpConstantComposite %v2int %int_n1 %int_n1
%38 = OpConstantComposite %v2int %int_1 %int_1
%v4bool = OpTypeVector %bool 4
%int_0 = OpConstant %int 0
%48 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%v3int = OpTypeVector %int 3
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%102 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%114 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%v2float = OpTypeVector %float 2
%118 = OpConstantComposite %v2float %float_n1 %float_n1
%121 = OpConstantComposite %v2float %float_1 %float_1
%130 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v3float = OpTypeVector %float 3
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_int = OpFunction %bool None %19
%20 = OpLabel
%one = OpVariable %_ptr_Function_int Function
%two = OpVariable %_ptr_Function_int Function
%result = OpVariable %_ptr_Function_v4int Function
OpStore %one %int_1
OpStore %two %int_2
%33 = OpSNegate %v2int %35
%36 = OpCompositeExtract %int %33 0
%37 = OpCompositeExtract %int %33 1
%39 = OpCompositeExtract %int %38 0
%40 = OpCompositeExtract %int %38 1
%41 = OpCompositeConstruct %v4int %36 %37 %39 %40
%32 = OpSNegate %v4int %41
%42 = OpIEqual %v4bool %31 %32
%44 = OpAll %bool %42
%45 = OpSelect %int %44 %int_1 %int_0
%47 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %47 %45
%49 = OpSNegate %v4int %48
%50 = OpINotEqual %v4bool %48 %49
%51 = OpAny %bool %50
%52 = OpSelect %int %51 %int_1 %int_0
%53 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %53 %52
%55 = OpLoad %int %two
%56 = OpCompositeConstruct %v4int %55 %55 %55 %55
%54 = OpSNegate %v4int %56
%58 = OpLoad %int %two
%57 = OpSNegate %int %58
%60 = OpLoad %int %two
%59 = OpSNegate %int %60
%61 = OpCompositeConstruct %v3int %59 %59 %59
%63 = OpCompositeExtract %int %61 0
%64 = OpCompositeExtract %int %61 1
%65 = OpCompositeExtract %int %61 2
%66 = OpCompositeConstruct %v4int %57 %63 %64 %65
%67 = OpIEqual %v4bool %54 %66
%68 = OpAll %bool %67
%69 = OpSelect %int %68 %int_1 %int_0
%70 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %70 %69
%73 = OpLoad %int %one
%72 = OpSNegate %int %73
%74 = OpLoad %int %one
%75 = OpLoad %int %one
%76 = OpIAdd %int %74 %75
%77 = OpCompositeConstruct %v2int %72 %76
%71 = OpSNegate %v2int %77
%79 = OpLoad %int %one
%80 = OpLoad %int %two
%81 = OpISub %int %79 %80
%82 = OpLoad %int %two
%83 = OpCompositeConstruct %v2int %81 %82
%78 = OpSNegate %v2int %83
%84 = OpIEqual %v2bool %71 %78
%86 = OpAll %bool %84
%87 = OpSelect %int %86 %int_1 %int_0
%88 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %88 %87
%90 = OpLoad %v4int %result
%91 = OpCompositeExtract %int %90 0
%92 = OpLoad %v4int %result
%93 = OpCompositeExtract %int %92 1
%94 = OpIMul %int %91 %93
%95 = OpLoad %v4int %result
%96 = OpCompositeExtract %int %95 2
%97 = OpIMul %int %94 %96
%98 = OpLoad %v4int %result
%99 = OpCompositeExtract %int %98 3
%100 = OpIMul %int %97 %99
%101 = OpINotEqual %bool %100 %int_0
OpReturnValue %101
OpFunctionEnd
%main = OpFunction %v4float None %102
%103 = OpLabel
%_0_test_float = OpVariable %_ptr_Function_bool Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_two = OpVariable %_ptr_Function_float Function
%_3_result = OpVariable %_ptr_Function_v4float Function
%191 = OpVariable %_ptr_Function_v4float Function
OpStore %_1_one %float_1
OpStore %_2_two %float_2
%116 = OpFNegate %v2float %118
%119 = OpCompositeExtract %float %116 0
%120 = OpCompositeExtract %float %116 1
%122 = OpCompositeExtract %float %121 0
%123 = OpCompositeExtract %float %121 1
%124 = OpCompositeConstruct %v4float %119 %120 %122 %123
%115 = OpFNegate %v4float %124
%125 = OpFOrdEqual %v4bool %114 %115
%126 = OpAll %bool %125
%127 = OpSelect %int %126 %int_1 %int_0
%128 = OpConvertSToF %float %127
%129 = OpAccessChain %_ptr_Function_float %_3_result %int_0
OpStore %129 %128
%131 = OpFNegate %v4float %130
%132 = OpFOrdNotEqual %v4bool %130 %131
%133 = OpAny %bool %132
%134 = OpSelect %int %133 %int_1 %int_0
%135 = OpConvertSToF %float %134
%136 = OpAccessChain %_ptr_Function_float %_3_result %int_1
OpStore %136 %135
%138 = OpLoad %float %_2_two
%139 = OpCompositeConstruct %v4float %138 %138 %138 %138
%137 = OpFNegate %v4float %139
%141 = OpLoad %float %_2_two
%140 = OpFNegate %float %141
%143 = OpLoad %float %_2_two
%142 = OpFNegate %float %143
%144 = OpCompositeConstruct %v3float %142 %142 %142
%146 = OpCompositeExtract %float %144 0
%147 = OpCompositeExtract %float %144 1
%148 = OpCompositeExtract %float %144 2
%149 = OpCompositeConstruct %v4float %140 %146 %147 %148
%150 = OpFOrdEqual %v4bool %137 %149
%151 = OpAll %bool %150
%152 = OpSelect %int %151 %int_1 %int_0
%153 = OpConvertSToF %float %152
%154 = OpAccessChain %_ptr_Function_float %_3_result %int_2
OpStore %154 %153
%157 = OpLoad %float %_1_one
%156 = OpFNegate %float %157
%158 = OpLoad %float %_1_one
%159 = OpLoad %float %_1_one
%160 = OpFAdd %float %158 %159
%161 = OpCompositeConstruct %v2float %156 %160
%155 = OpFNegate %v2float %161
%163 = OpLoad %float %_1_one
%164 = OpLoad %float %_2_two
%165 = OpFSub %float %163 %164
%166 = OpLoad %float %_2_two
%167 = OpCompositeConstruct %v2float %165 %166
%162 = OpFNegate %v2float %167
%168 = OpFOrdEqual %v2bool %155 %162
%169 = OpAll %bool %168
%170 = OpSelect %int %169 %int_1 %int_0
%171 = OpConvertSToF %float %170
%172 = OpAccessChain %_ptr_Function_float %_3_result %int_3
OpStore %172 %171
%174 = OpLoad %v4float %_3_result
%175 = OpCompositeExtract %float %174 0
%176 = OpLoad %v4float %_3_result
%177 = OpCompositeExtract %float %176 1
%178 = OpFMul %float %175 %177
%179 = OpLoad %v4float %_3_result
%180 = OpCompositeExtract %float %179 2
%181 = OpFMul %float %178 %180
%182 = OpLoad %v4float %_3_result
%183 = OpCompositeExtract %float %182 3
%184 = OpFMul %float %181 %183
%185 = OpFUnordNotEqual %bool %184 %float_0
OpSelectionMerge %188 None
OpBranchConditional %185 %187 %188
%187 = OpLabel
%189 = OpFunctionCall %bool %test_int
OpBranch %188
%188 = OpLabel
%190 = OpPhi %bool %false %103 %189 %187
OpSelectionMerge %194 None
OpBranchConditional %190 %192 %193
%192 = OpLabel
%195 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%197 = OpLoad %v4float %195
OpStore %191 %197
OpBranch %194
%193 = OpLabel
%198 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%199 = OpLoad %v4float %198
OpStore %191 %199
OpBranch %194
%194 = OpLabel
%200 = OpLoad %v4float %191
OpReturnValue %200
OpFunctionEnd
