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
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
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
%33 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%v4bool = OpTypeVector %bool 4
%int_0 = OpConstant %int 0
%int_n2 = OpConstant %int -2
%45 = OpConstantComposite %v4int %int_n2 %int_n2 %int_n2 %int_n2
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%82 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%92 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%94 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v3float = OpTypeVector %float 3
%v2float = OpTypeVector %float 2
%121 = OpConstantComposite %v2float %float_n1 %float_2
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
%32 = OpSNegate %v4int %33
%34 = OpIEqual %v4bool %31 %32
%36 = OpAll %bool %34
%37 = OpSelect %int %36 %int_1 %int_0
%39 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %39 %37
%40 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %40 %int_1
%42 = OpLoad %int %two
%43 = OpCompositeConstruct %v4int %42 %42 %42 %42
%41 = OpSNegate %v4int %43
%46 = OpIEqual %v4bool %41 %45
%47 = OpAll %bool %46
%48 = OpSelect %int %47 %int_1 %int_0
%49 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %49 %48
%53 = OpLoad %int %one
%52 = OpSNegate %int %53
%54 = OpLoad %int %one
%55 = OpLoad %int %one
%56 = OpIAdd %int %54 %55
%57 = OpCompositeConstruct %v2int %52 %56
%50 = OpSNegate %v2int %57
%59 = OpLoad %int %one
%60 = OpLoad %int %two
%61 = OpISub %int %59 %60
%62 = OpLoad %int %two
%63 = OpCompositeConstruct %v2int %61 %62
%58 = OpSNegate %v2int %63
%64 = OpIEqual %v2bool %50 %58
%66 = OpAll %bool %64
%67 = OpSelect %int %66 %int_1 %int_0
%68 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %68 %67
%70 = OpLoad %v4int %result
%71 = OpCompositeExtract %int %70 0
%72 = OpLoad %v4int %result
%73 = OpCompositeExtract %int %72 1
%74 = OpIMul %int %71 %73
%75 = OpLoad %v4int %result
%76 = OpCompositeExtract %int %75 2
%77 = OpIMul %int %74 %76
%78 = OpLoad %v4int %result
%79 = OpCompositeExtract %int %78 3
%80 = OpIMul %int %77 %79
%81 = OpINotEqual %bool %80 %int_0
OpReturnValue %81
OpFunctionEnd
%main = OpFunction %v4float None %82
%83 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%151 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%93 = OpFNegate %v4float %94
%95 = OpFOrdEqual %v4bool %92 %93
%96 = OpAll %bool %95
%97 = OpSelect %int %96 %int_1 %int_0
%98 = OpConvertSToF %float %97
%99 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %99 %98
%100 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %100 %float_1
%102 = OpLoad %float %_1_two
%103 = OpCompositeConstruct %v4float %102 %102 %102 %102
%101 = OpFNegate %v4float %103
%105 = OpLoad %float %_1_two
%104 = OpFNegate %float %105
%107 = OpLoad %float %_1_two
%106 = OpFNegate %float %107
%108 = OpCompositeConstruct %v3float %106 %106 %106
%110 = OpCompositeExtract %float %108 0
%111 = OpCompositeExtract %float %108 1
%112 = OpCompositeExtract %float %108 2
%113 = OpCompositeConstruct %v4float %104 %110 %111 %112
%114 = OpFOrdEqual %v4bool %101 %113
%115 = OpAll %bool %114
%116 = OpSelect %int %115 %int_1 %int_0
%117 = OpConvertSToF %float %116
%118 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %118 %117
%119 = OpFNegate %v2float %121
%123 = OpLoad %float %_0_one
%124 = OpLoad %float %_1_two
%125 = OpFSub %float %123 %124
%126 = OpLoad %float %_1_two
%127 = OpCompositeConstruct %v2float %125 %126
%122 = OpFNegate %v2float %127
%128 = OpFOrdEqual %v2bool %119 %122
%129 = OpAll %bool %128
%130 = OpSelect %int %129 %int_1 %int_0
%131 = OpConvertSToF %float %130
%132 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %132 %131
%134 = OpLoad %v4float %_2_result
%135 = OpCompositeExtract %float %134 0
%136 = OpLoad %v4float %_2_result
%137 = OpCompositeExtract %float %136 1
%138 = OpFMul %float %135 %137
%139 = OpLoad %v4float %_2_result
%140 = OpCompositeExtract %float %139 2
%141 = OpFMul %float %138 %140
%142 = OpLoad %v4float %_2_result
%143 = OpCompositeExtract %float %142 3
%144 = OpFMul %float %141 %143
%145 = OpFUnordNotEqual %bool %144 %float_0
OpSelectionMerge %148 None
OpBranchConditional %145 %147 %148
%147 = OpLabel
%149 = OpFunctionCall %bool %test_int_b
OpBranch %148
%148 = OpLabel
%150 = OpPhi %bool %false %83 %149 %147
OpSelectionMerge %154 None
OpBranchConditional %150 %152 %153
%152 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%157 = OpLoad %v4float %155
OpStore %151 %157
OpBranch %154
%153 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%159 = OpLoad %v4float %158
OpStore %151 %159
OpBranch %154
%154 = OpLabel
%160 = OpLoad %v4float %151
OpReturnValue %160
OpFunctionEnd
