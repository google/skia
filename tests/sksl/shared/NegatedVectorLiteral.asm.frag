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
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_0 = OpConstant %int 0
%int_n2 = OpConstant %int -2
%42 = OpConstantComposite %v4int %int_n2 %int_n2 %int_n2 %int_n2
%v4bool = OpTypeVector %bool 4
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%79 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v3float = OpTypeVector %float 3
%float_n2 = OpConstant %float -2
%110 = OpConstantComposite %v2float %float_1 %float_n2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %24
%25 = OpLabel
%one = OpVariable %_ptr_Function_int Function
%two = OpVariable %_ptr_Function_int Function
%result = OpVariable %_ptr_Function_v4int Function
OpStore %one %int_1
OpStore %two %int_2
%35 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %35 %int_1
%37 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %37 %int_1
%39 = OpLoad %int %two
%40 = OpCompositeConstruct %v4int %39 %39 %39 %39
%38 = OpSNegate %v4int %40
%43 = OpIEqual %v4bool %38 %42
%45 = OpAll %bool %43
%46 = OpSelect %int %45 %int_1 %int_0
%47 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %47 %46
%51 = OpLoad %int %one
%50 = OpSNegate %int %51
%52 = OpLoad %int %one
%53 = OpLoad %int %one
%54 = OpIAdd %int %52 %53
%55 = OpCompositeConstruct %v2int %50 %54
%48 = OpSNegate %v2int %55
%57 = OpLoad %int %one
%58 = OpLoad %int %two
%59 = OpISub %int %57 %58
%60 = OpCompositeConstruct %v2int %59 %int_2
%56 = OpSNegate %v2int %60
%61 = OpIEqual %v2bool %48 %56
%63 = OpAll %bool %61
%64 = OpSelect %int %63 %int_1 %int_0
%65 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %65 %64
%67 = OpLoad %v4int %result
%68 = OpCompositeExtract %int %67 0
%69 = OpLoad %v4int %result
%70 = OpCompositeExtract %int %69 1
%71 = OpIMul %int %68 %70
%72 = OpLoad %v4int %result
%73 = OpCompositeExtract %int %72 2
%74 = OpIMul %int %71 %73
%75 = OpLoad %v4int %result
%76 = OpCompositeExtract %int %75 3
%77 = OpIMul %int %74 %76
%78 = OpINotEqual %bool %77 %int_0
OpReturnValue %78
OpFunctionEnd
%main = OpFunction %v4float None %79
%80 = OpFunctionParameter %_ptr_Function_v2float
%81 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%139 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%89 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %89 %float_1
%90 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %90 %float_1
%92 = OpLoad %float %_1_two
%93 = OpCompositeConstruct %v4float %92 %92 %92 %92
%91 = OpFNegate %v4float %93
%95 = OpLoad %float %_1_two
%94 = OpFNegate %float %95
%97 = OpLoad %float %_1_two
%96 = OpFNegate %float %97
%98 = OpCompositeConstruct %v3float %96 %96 %96
%100 = OpCompositeExtract %float %98 0
%101 = OpCompositeExtract %float %98 1
%102 = OpCompositeExtract %float %98 2
%103 = OpCompositeConstruct %v4float %94 %100 %101 %102
%104 = OpFOrdEqual %v4bool %91 %103
%105 = OpAll %bool %104
%106 = OpSelect %int %105 %int_1 %int_0
%107 = OpConvertSToF %float %106
%108 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %108 %107
%112 = OpLoad %float %_0_one
%113 = OpLoad %float %_1_two
%114 = OpFSub %float %112 %113
%115 = OpLoad %float %_1_two
%116 = OpCompositeConstruct %v2float %114 %115
%111 = OpFNegate %v2float %116
%117 = OpFOrdEqual %v2bool %110 %111
%118 = OpAll %bool %117
%119 = OpSelect %int %118 %int_1 %int_0
%120 = OpConvertSToF %float %119
%121 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %121 %120
%123 = OpLoad %v4float %_2_result
%124 = OpCompositeExtract %float %123 0
%125 = OpLoad %v4float %_2_result
%126 = OpCompositeExtract %float %125 1
%127 = OpFMul %float %124 %126
%128 = OpLoad %v4float %_2_result
%129 = OpCompositeExtract %float %128 2
%130 = OpFMul %float %127 %129
%131 = OpLoad %v4float %_2_result
%132 = OpCompositeExtract %float %131 3
%133 = OpFMul %float %130 %132
%134 = OpFUnordNotEqual %bool %133 %float_0
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpFunctionCall %bool %test_int_b
OpBranch %136
%136 = OpLabel
%138 = OpPhi %bool %false %81 %137 %135
OpSelectionMerge %142 None
OpBranchConditional %138 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%145 = OpLoad %v4float %143
OpStore %139 %145
OpBranch %142
%141 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%147 = OpLoad %v4float %146
OpStore %139 %147
OpBranch %142
%142 = OpLabel
%148 = OpLoad %v4float %139
OpReturnValue %148
OpFunctionEnd
