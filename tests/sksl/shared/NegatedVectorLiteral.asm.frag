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
OpName %test_float "test_float"
OpName %one "one"
OpName %two "two"
OpName %result "result"
OpName %test_int "test_int"
OpName %one_0 "one"
OpName %two_0 "two"
OpName %result_0 "result"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %34 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
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
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%20 = OpTypeFunction %bool
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%v2float = OpTypeVector %float 2
%float_n2 = OpConstant %float -2
%56 = OpConstantComposite %v2float %float_1 %float_n2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%float_0 = OpConstant %float 0
%_ptr_Function_int = OpTypePointer Function %int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_n2 = OpConstant %int -2
%96 = OpConstantComposite %v4int %int_n2 %int_n2 %int_n2 %int_n2
%v2int = OpTypeVector %int 2
%131 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%test_float = OpFunction %bool None %20
%21 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%two = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v4float Function
OpStore %one %float_1
OpStore %two %float_2
%29 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %29 %float_1
%32 = OpAccessChain %_ptr_Function_float %result %int_1
OpStore %32 %float_1
%35 = OpLoad %float %two
%36 = OpCompositeConstruct %v4float %35 %35 %35 %35
%34 = OpFNegate %v4float %36
%38 = OpLoad %float %two
%37 = OpFNegate %float %38
%40 = OpLoad %float %two
%39 = OpFNegate %float %40
%41 = OpCompositeConstruct %v3float %39 %39 %39
%43 = OpCompositeExtract %float %41 0
%44 = OpCompositeExtract %float %41 1
%45 = OpCompositeExtract %float %41 2
%46 = OpCompositeConstruct %v4float %37 %43 %44 %45
%47 = OpFOrdEqual %v4bool %34 %46
%49 = OpAll %bool %47
%50 = OpSelect %int %49 %int_1 %int_0
%51 = OpConvertSToF %float %50
%52 = OpAccessChain %_ptr_Function_float %result %int_2
OpStore %52 %51
%58 = OpLoad %float %one
%59 = OpLoad %float %two
%60 = OpFSub %float %58 %59
%61 = OpLoad %float %two
%62 = OpCompositeConstruct %v2float %60 %61
%57 = OpFNegate %v2float %62
%63 = OpFOrdEqual %v2bool %56 %57
%65 = OpAll %bool %63
%66 = OpSelect %int %65 %int_1 %int_0
%67 = OpConvertSToF %float %66
%68 = OpAccessChain %_ptr_Function_float %result %int_3
OpStore %68 %67
%70 = OpLoad %v4float %result
%71 = OpCompositeExtract %float %70 0
%72 = OpLoad %v4float %result
%73 = OpCompositeExtract %float %72 1
%74 = OpFMul %float %71 %73
%75 = OpLoad %v4float %result
%76 = OpCompositeExtract %float %75 2
%77 = OpFMul %float %74 %76
%78 = OpLoad %v4float %result
%79 = OpCompositeExtract %float %78 3
%80 = OpFMul %float %77 %79
%81 = OpFUnordNotEqual %bool %80 %float_0
OpReturnValue %81
OpFunctionEnd
%test_int = OpFunction %bool None %20
%83 = OpLabel
%one_0 = OpVariable %_ptr_Function_int Function
%two_0 = OpVariable %_ptr_Function_int Function
%result_0 = OpVariable %_ptr_Function_v4int Function
OpStore %one_0 %int_1
OpStore %two_0 %int_2
%90 = OpAccessChain %_ptr_Function_int %result_0 %int_0
OpStore %90 %int_1
%91 = OpAccessChain %_ptr_Function_int %result_0 %int_1
OpStore %91 %int_1
%93 = OpLoad %int %two_0
%94 = OpCompositeConstruct %v4int %93 %93 %93 %93
%92 = OpSNegate %v4int %94
%97 = OpIEqual %v4bool %92 %96
%98 = OpAll %bool %97
%99 = OpSelect %int %98 %int_1 %int_0
%100 = OpAccessChain %_ptr_Function_int %result_0 %int_2
OpStore %100 %99
%104 = OpLoad %int %one_0
%103 = OpSNegate %int %104
%105 = OpLoad %int %one_0
%106 = OpLoad %int %one_0
%107 = OpIAdd %int %105 %106
%108 = OpCompositeConstruct %v2int %103 %107
%101 = OpSNegate %v2int %108
%110 = OpLoad %int %one_0
%111 = OpLoad %int %two_0
%112 = OpISub %int %110 %111
%113 = OpLoad %int %two_0
%114 = OpCompositeConstruct %v2int %112 %113
%109 = OpSNegate %v2int %114
%115 = OpIEqual %v2bool %101 %109
%116 = OpAll %bool %115
%117 = OpSelect %int %116 %int_1 %int_0
%118 = OpAccessChain %_ptr_Function_int %result_0 %int_3
OpStore %118 %117
%119 = OpLoad %v4int %result_0
%120 = OpCompositeExtract %int %119 0
%121 = OpLoad %v4int %result_0
%122 = OpCompositeExtract %int %121 1
%123 = OpIMul %int %120 %122
%124 = OpLoad %v4int %result_0
%125 = OpCompositeExtract %int %124 2
%126 = OpIMul %int %123 %125
%127 = OpLoad %v4int %result_0
%128 = OpCompositeExtract %int %127 3
%129 = OpIMul %int %126 %128
%130 = OpINotEqual %bool %129 %int_0
OpReturnValue %130
OpFunctionEnd
%main = OpFunction %v4float None %131
%132 = OpLabel
%139 = OpVariable %_ptr_Function_v4float Function
%134 = OpFunctionCall %bool %test_float
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpFunctionCall %bool %test_int
OpBranch %136
%136 = OpLabel
%138 = OpPhi %bool %false %132 %137 %135
OpSelectionMerge %142 None
OpBranchConditional %138 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%145 = OpLoad %v4float %143
OpStore %139 %145
OpBranch %142
%141 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%147 = OpLoad %v4float %146
OpStore %139 %147
OpBranch %142
%142 = OpLabel
%148 = OpLoad %v4float %139
OpReturnValue %148
OpFunctionEnd
