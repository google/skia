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
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
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
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%70 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v3float = OpTypeVector %float 3
%v4bool = OpTypeVector %bool 4
%float_n2 = OpConstant %float -2
%102 = OpConstantComposite %v2float %float_1 %float_n2
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
%38 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %38 %int_1
%42 = OpLoad %int %one
%41 = OpSNegate %int %42
%43 = OpLoad %int %one
%44 = OpLoad %int %one
%45 = OpIAdd %int %43 %44
%46 = OpCompositeConstruct %v2int %41 %45
%39 = OpSNegate %v2int %46
%48 = OpLoad %int %one
%49 = OpLoad %int %two
%50 = OpISub %int %48 %49
%51 = OpCompositeConstruct %v2int %50 %int_2
%47 = OpSNegate %v2int %51
%52 = OpIEqual %v2bool %39 %47
%54 = OpAll %bool %52
%55 = OpSelect %int %54 %int_1 %int_0
%56 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %56 %55
%58 = OpLoad %v4int %result
%59 = OpCompositeExtract %int %58 0
%60 = OpLoad %v4int %result
%61 = OpCompositeExtract %int %60 1
%62 = OpIMul %int %59 %61
%63 = OpLoad %v4int %result
%64 = OpCompositeExtract %int %63 2
%65 = OpIMul %int %62 %64
%66 = OpLoad %v4int %result
%67 = OpCompositeExtract %int %66 3
%68 = OpIMul %int %65 %67
%69 = OpINotEqual %bool %68 %int_0
OpReturnValue %69
OpFunctionEnd
%main = OpFunction %v4float None %70
%71 = OpFunctionParameter %_ptr_Function_v2float
%72 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%131 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%80 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %80 %float_1
%81 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %81 %float_1
%83 = OpLoad %float %_1_two
%84 = OpCompositeConstruct %v4float %83 %83 %83 %83
%82 = OpFNegate %v4float %84
%86 = OpLoad %float %_1_two
%85 = OpFNegate %float %86
%88 = OpLoad %float %_1_two
%87 = OpFNegate %float %88
%89 = OpCompositeConstruct %v3float %87 %87 %87
%91 = OpCompositeExtract %float %89 0
%92 = OpCompositeExtract %float %89 1
%93 = OpCompositeExtract %float %89 2
%94 = OpCompositeConstruct %v4float %85 %91 %92 %93
%95 = OpFOrdEqual %v4bool %82 %94
%97 = OpAll %bool %95
%98 = OpSelect %int %97 %int_1 %int_0
%99 = OpConvertSToF %float %98
%100 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %100 %99
%104 = OpLoad %float %_0_one
%105 = OpLoad %float %_1_two
%106 = OpFSub %float %104 %105
%107 = OpLoad %float %_1_two
%108 = OpCompositeConstruct %v2float %106 %107
%103 = OpFNegate %v2float %108
%109 = OpFOrdEqual %v2bool %102 %103
%110 = OpAll %bool %109
%111 = OpSelect %int %110 %int_1 %int_0
%112 = OpConvertSToF %float %111
%113 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %113 %112
%115 = OpLoad %v4float %_2_result
%116 = OpCompositeExtract %float %115 0
%117 = OpLoad %v4float %_2_result
%118 = OpCompositeExtract %float %117 1
%119 = OpFMul %float %116 %118
%120 = OpLoad %v4float %_2_result
%121 = OpCompositeExtract %float %120 2
%122 = OpFMul %float %119 %121
%123 = OpLoad %v4float %_2_result
%124 = OpCompositeExtract %float %123 3
%125 = OpFMul %float %122 %124
%126 = OpFUnordNotEqual %bool %125 %float_0
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%129 = OpFunctionCall %bool %test_int_b
OpBranch %128
%128 = OpLabel
%130 = OpPhi %bool %false %72 %129 %127
OpSelectionMerge %134 None
OpBranchConditional %130 %132 %133
%132 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%137 = OpLoad %v4float %135
OpStore %131 %137
OpBranch %134
%133 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%139 = OpLoad %v4float %138
OpStore %131 %139
OpBranch %134
%134 = OpLabel
%140 = OpLoad %v4float %131
OpReturnValue %140
OpFunctionEnd
