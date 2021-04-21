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
OpDecorate %94 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
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
%80 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v3float = OpTypeVector %float 3
%float_n2 = OpConstant %float -2
%111 = OpConstantComposite %v2float %float_1 %float_n2
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
%60 = OpLoad %int %two
%61 = OpCompositeConstruct %v2int %59 %60
%56 = OpSNegate %v2int %61
%62 = OpIEqual %v2bool %48 %56
%64 = OpAll %bool %62
%65 = OpSelect %int %64 %int_1 %int_0
%66 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %66 %65
%68 = OpLoad %v4int %result
%69 = OpCompositeExtract %int %68 0
%70 = OpLoad %v4int %result
%71 = OpCompositeExtract %int %70 1
%72 = OpIMul %int %69 %71
%73 = OpLoad %v4int %result
%74 = OpCompositeExtract %int %73 2
%75 = OpIMul %int %72 %74
%76 = OpLoad %v4int %result
%77 = OpCompositeExtract %int %76 3
%78 = OpIMul %int %75 %77
%79 = OpINotEqual %bool %78 %int_0
OpReturnValue %79
OpFunctionEnd
%main = OpFunction %v4float None %80
%81 = OpFunctionParameter %_ptr_Function_v2float
%82 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%140 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%90 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %90 %float_1
%91 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %91 %float_1
%93 = OpLoad %float %_1_two
%94 = OpCompositeConstruct %v4float %93 %93 %93 %93
%92 = OpFNegate %v4float %94
%96 = OpLoad %float %_1_two
%95 = OpFNegate %float %96
%98 = OpLoad %float %_1_two
%97 = OpFNegate %float %98
%99 = OpCompositeConstruct %v3float %97 %97 %97
%101 = OpCompositeExtract %float %99 0
%102 = OpCompositeExtract %float %99 1
%103 = OpCompositeExtract %float %99 2
%104 = OpCompositeConstruct %v4float %95 %101 %102 %103
%105 = OpFOrdEqual %v4bool %92 %104
%106 = OpAll %bool %105
%107 = OpSelect %int %106 %int_1 %int_0
%108 = OpConvertSToF %float %107
%109 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %109 %108
%113 = OpLoad %float %_0_one
%114 = OpLoad %float %_1_two
%115 = OpFSub %float %113 %114
%116 = OpLoad %float %_1_two
%117 = OpCompositeConstruct %v2float %115 %116
%112 = OpFNegate %v2float %117
%118 = OpFOrdEqual %v2bool %111 %112
%119 = OpAll %bool %118
%120 = OpSelect %int %119 %int_1 %int_0
%121 = OpConvertSToF %float %120
%122 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %122 %121
%124 = OpLoad %v4float %_2_result
%125 = OpCompositeExtract %float %124 0
%126 = OpLoad %v4float %_2_result
%127 = OpCompositeExtract %float %126 1
%128 = OpFMul %float %125 %127
%129 = OpLoad %v4float %_2_result
%130 = OpCompositeExtract %float %129 2
%131 = OpFMul %float %128 %130
%132 = OpLoad %v4float %_2_result
%133 = OpCompositeExtract %float %132 3
%134 = OpFMul %float %131 %133
%135 = OpFUnordNotEqual %bool %134 %float_0
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpFunctionCall %bool %test_int_b
OpBranch %137
%137 = OpLabel
%139 = OpPhi %bool %false %82 %138 %136
OpSelectionMerge %143 None
OpBranchConditional %139 %141 %142
%141 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%146 = OpLoad %v4float %144
OpStore %140 %146
OpBranch %143
%142 = OpLabel
%147 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%148 = OpLoad %v4float %147
OpStore %140 %148
OpBranch %143
%143 = OpLabel
%149 = OpLoad %v4float %140
OpReturnValue %149
OpFunctionEnd
