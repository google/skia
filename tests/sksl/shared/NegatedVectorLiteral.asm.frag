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
OpDecorate %86 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
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
%int_0 = OpConstant %int 0
%int_n2 = OpConstant %int -2
%37 = OpConstantComposite %v4int %int_n2 %int_n2 %int_n2 %int_n2
%v4bool = OpTypeVector %bool 4
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%75 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v3float = OpTypeVector %float 3
%v2float = OpTypeVector %float 2
%float_n2 = OpConstant %float -2
%106 = OpConstantComposite %v2float %float_1 %float_n2
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
%30 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %30 %int_1
%32 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %32 %int_1
%34 = OpLoad %int %two
%35 = OpCompositeConstruct %v4int %34 %34 %34 %34
%33 = OpSNegate %v4int %35
%38 = OpIEqual %v4bool %33 %37
%40 = OpAll %bool %38
%41 = OpSelect %int %40 %int_1 %int_0
%42 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %42 %41
%46 = OpLoad %int %one
%45 = OpSNegate %int %46
%47 = OpLoad %int %one
%48 = OpLoad %int %one
%49 = OpIAdd %int %47 %48
%50 = OpCompositeConstruct %v2int %45 %49
%43 = OpSNegate %v2int %50
%52 = OpLoad %int %one
%53 = OpLoad %int %two
%54 = OpISub %int %52 %53
%55 = OpLoad %int %two
%56 = OpCompositeConstruct %v2int %54 %55
%51 = OpSNegate %v2int %56
%57 = OpIEqual %v2bool %43 %51
%59 = OpAll %bool %57
%60 = OpSelect %int %59 %int_1 %int_0
%61 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %61 %60
%63 = OpLoad %v4int %result
%64 = OpCompositeExtract %int %63 0
%65 = OpLoad %v4int %result
%66 = OpCompositeExtract %int %65 1
%67 = OpIMul %int %64 %66
%68 = OpLoad %v4int %result
%69 = OpCompositeExtract %int %68 2
%70 = OpIMul %int %67 %69
%71 = OpLoad %v4int %result
%72 = OpCompositeExtract %int %71 3
%73 = OpIMul %int %70 %72
%74 = OpINotEqual %bool %73 %int_0
OpReturnValue %74
OpFunctionEnd
%main = OpFunction %v4float None %75
%76 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%136 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%84 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %84 %float_1
%85 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %85 %float_1
%87 = OpLoad %float %_1_two
%88 = OpCompositeConstruct %v4float %87 %87 %87 %87
%86 = OpFNegate %v4float %88
%90 = OpLoad %float %_1_two
%89 = OpFNegate %float %90
%92 = OpLoad %float %_1_two
%91 = OpFNegate %float %92
%93 = OpCompositeConstruct %v3float %91 %91 %91
%95 = OpCompositeExtract %float %93 0
%96 = OpCompositeExtract %float %93 1
%97 = OpCompositeExtract %float %93 2
%98 = OpCompositeConstruct %v4float %89 %95 %96 %97
%99 = OpFOrdEqual %v4bool %86 %98
%100 = OpAll %bool %99
%101 = OpSelect %int %100 %int_1 %int_0
%102 = OpConvertSToF %float %101
%103 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %103 %102
%108 = OpLoad %float %_0_one
%109 = OpLoad %float %_1_two
%110 = OpFSub %float %108 %109
%111 = OpLoad %float %_1_two
%112 = OpCompositeConstruct %v2float %110 %111
%107 = OpFNegate %v2float %112
%113 = OpFOrdEqual %v2bool %106 %107
%114 = OpAll %bool %113
%115 = OpSelect %int %114 %int_1 %int_0
%116 = OpConvertSToF %float %115
%117 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %117 %116
%119 = OpLoad %v4float %_2_result
%120 = OpCompositeExtract %float %119 0
%121 = OpLoad %v4float %_2_result
%122 = OpCompositeExtract %float %121 1
%123 = OpFMul %float %120 %122
%124 = OpLoad %v4float %_2_result
%125 = OpCompositeExtract %float %124 2
%126 = OpFMul %float %123 %125
%127 = OpLoad %v4float %_2_result
%128 = OpCompositeExtract %float %127 3
%129 = OpFMul %float %126 %128
%130 = OpFUnordNotEqual %bool %129 %float_0
OpSelectionMerge %133 None
OpBranchConditional %130 %132 %133
%132 = OpLabel
%134 = OpFunctionCall %bool %test_int
OpBranch %133
%133 = OpLabel
%135 = OpPhi %bool %false %76 %134 %132
OpSelectionMerge %139 None
OpBranchConditional %135 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%142 = OpLoad %v4float %140
OpStore %136 %142
OpBranch %139
%138 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%144 = OpLoad %v4float %143
OpStore %136 %144
OpBranch %139
%139 = OpLabel
%145 = OpLoad %v4float %136
OpReturnValue %145
OpFunctionEnd
