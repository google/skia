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
OpName %test_int_b "test_int_b"
OpName %one "one"
OpName %two "two"
OpName %result "result"
OpName %main "main"
OpName %_0_one "_0_one"
OpName %_1_two "_1_two"
OpName %_2_result "_2_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
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
%34 = OpConstantComposite %v2int %int_1 %int_1
%v4bool = OpTypeVector %bool 4
%int_0 = OpConstant %int 0
%int_n2 = OpConstant %int -2
%v3int = OpTypeVector %int 3
%52 = OpConstantComposite %v3int %int_n2 %int_n2 %int_n2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%92 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%102 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%v2float = OpTypeVector %float 2
%105 = OpConstantComposite %v2float %float_1 %float_1
%v3float = OpTypeVector %float 3
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %19
%20 = OpLabel
%one = OpVariable %_ptr_Function_int Function
%two = OpVariable %_ptr_Function_int Function
%result = OpVariable %_ptr_Function_v4int Function
OpStore %one %int_1
OpStore %two %int_2
%35 = OpCompositeExtract %int %34 0
%36 = OpCompositeExtract %int %34 1
%37 = OpCompositeExtract %int %34 0
%38 = OpCompositeExtract %int %34 1
%39 = OpCompositeConstruct %v4int %35 %36 %37 %38
%32 = OpSNegate %v4int %39
%40 = OpIEqual %v4bool %31 %32
%42 = OpAll %bool %40
%43 = OpSelect %int %42 %int_1 %int_0
%45 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %45 %43
%46 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %46 %int_1
%48 = OpLoad %int %two
%49 = OpCompositeConstruct %v4int %48 %48 %48 %48
%47 = OpSNegate %v4int %49
%53 = OpCompositeExtract %int %52 0
%54 = OpCompositeExtract %int %52 1
%55 = OpCompositeExtract %int %52 2
%56 = OpCompositeConstruct %v4int %int_n2 %53 %54 %55
%57 = OpIEqual %v4bool %47 %56
%58 = OpAll %bool %57
%59 = OpSelect %int %58 %int_1 %int_0
%60 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %60 %59
%63 = OpLoad %int %one
%62 = OpSNegate %int %63
%64 = OpLoad %int %one
%65 = OpLoad %int %one
%66 = OpIAdd %int %64 %65
%67 = OpCompositeConstruct %v2int %62 %66
%61 = OpSNegate %v2int %67
%69 = OpLoad %int %one
%70 = OpLoad %int %two
%71 = OpISub %int %69 %70
%72 = OpLoad %int %two
%73 = OpCompositeConstruct %v2int %71 %72
%68 = OpSNegate %v2int %73
%74 = OpIEqual %v2bool %61 %68
%76 = OpAll %bool %74
%77 = OpSelect %int %76 %int_1 %int_0
%78 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %78 %77
%80 = OpLoad %v4int %result
%81 = OpCompositeExtract %int %80 0
%82 = OpLoad %v4int %result
%83 = OpCompositeExtract %int %82 1
%84 = OpIMul %int %81 %83
%85 = OpLoad %v4int %result
%86 = OpCompositeExtract %int %85 2
%87 = OpIMul %int %84 %86
%88 = OpLoad %v4int %result
%89 = OpCompositeExtract %int %88 3
%90 = OpIMul %int %87 %89
%91 = OpINotEqual %bool %90 %int_0
OpReturnValue %91
OpFunctionEnd
%main = OpFunction %v4float None %92
%93 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%166 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%106 = OpCompositeExtract %float %105 0
%107 = OpCompositeExtract %float %105 1
%108 = OpCompositeExtract %float %105 0
%109 = OpCompositeExtract %float %105 1
%110 = OpCompositeConstruct %v4float %106 %107 %108 %109
%103 = OpFNegate %v4float %110
%111 = OpFOrdEqual %v4bool %102 %103
%112 = OpAll %bool %111
%113 = OpSelect %int %112 %int_1 %int_0
%114 = OpConvertSToF %float %113
%115 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %115 %114
%116 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %116 %float_1
%118 = OpLoad %float %_1_two
%119 = OpCompositeConstruct %v4float %118 %118 %118 %118
%117 = OpFNegate %v4float %119
%121 = OpLoad %float %_1_two
%120 = OpFNegate %float %121
%123 = OpLoad %float %_1_two
%122 = OpFNegate %float %123
%124 = OpCompositeConstruct %v3float %122 %122 %122
%126 = OpCompositeExtract %float %124 0
%127 = OpCompositeExtract %float %124 1
%128 = OpCompositeExtract %float %124 2
%129 = OpCompositeConstruct %v4float %120 %126 %127 %128
%130 = OpFOrdEqual %v4bool %117 %129
%131 = OpAll %bool %130
%132 = OpSelect %int %131 %int_1 %int_0
%133 = OpConvertSToF %float %132
%134 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %134 %133
%136 = OpCompositeConstruct %v2float %float_n1 %float_2
%135 = OpFNegate %v2float %136
%138 = OpLoad %float %_0_one
%139 = OpLoad %float %_1_two
%140 = OpFSub %float %138 %139
%141 = OpLoad %float %_1_two
%142 = OpCompositeConstruct %v2float %140 %141
%137 = OpFNegate %v2float %142
%143 = OpFOrdEqual %v2bool %135 %137
%144 = OpAll %bool %143
%145 = OpSelect %int %144 %int_1 %int_0
%146 = OpConvertSToF %float %145
%147 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %147 %146
%149 = OpLoad %v4float %_2_result
%150 = OpCompositeExtract %float %149 0
%151 = OpLoad %v4float %_2_result
%152 = OpCompositeExtract %float %151 1
%153 = OpFMul %float %150 %152
%154 = OpLoad %v4float %_2_result
%155 = OpCompositeExtract %float %154 2
%156 = OpFMul %float %153 %155
%157 = OpLoad %v4float %_2_result
%158 = OpCompositeExtract %float %157 3
%159 = OpFMul %float %156 %158
%160 = OpFUnordNotEqual %bool %159 %float_0
OpSelectionMerge %163 None
OpBranchConditional %160 %162 %163
%162 = OpLabel
%164 = OpFunctionCall %bool %test_int_b
OpBranch %163
%163 = OpLabel
%165 = OpPhi %bool %false %93 %164 %162
OpSelectionMerge %169 None
OpBranchConditional %165 %167 %168
%167 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%172 = OpLoad %v4float %170
OpStore %166 %172
OpBranch %169
%168 = OpLabel
%173 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%174 = OpLoad %v4float %173
OpStore %166 %174
OpBranch %169
%169 = OpLabel
%175 = OpLoad %v4float %166
OpReturnValue %175
OpFunctionEnd
