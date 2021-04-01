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
OpDecorate %92 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
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
%32 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n1 %int_n1
%v4bool = OpTypeVector %bool 4
%int_0 = OpConstant %int 0
%int_n2 = OpConstant %int -2
%44 = OpConstantComposite %v4int %int_n2 %int_n2 %int_n2 %int_n2
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%81 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%92 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%v3float = OpTypeVector %float 3
%v2float = OpTypeVector %float 2
%float_n2 = OpConstant %float -2
%119 = OpConstantComposite %v2float %float_1 %float_n2
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
%31 = OpCompositeConstruct %v4int %int_n1 %int_n1 %int_n1 %int_n1
%33 = OpIEqual %v4bool %31 %32
%35 = OpAll %bool %33
%36 = OpSelect %int %35 %int_1 %int_0
%38 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %38 %36
%39 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %39 %int_1
%41 = OpLoad %int %two
%42 = OpCompositeConstruct %v4int %41 %41 %41 %41
%40 = OpSNegate %v4int %42
%45 = OpIEqual %v4bool %40 %44
%46 = OpAll %bool %45
%47 = OpSelect %int %46 %int_1 %int_0
%48 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %48 %47
%52 = OpLoad %int %one
%51 = OpSNegate %int %52
%53 = OpLoad %int %one
%54 = OpLoad %int %one
%55 = OpIAdd %int %53 %54
%56 = OpCompositeConstruct %v2int %51 %55
%49 = OpSNegate %v2int %56
%58 = OpLoad %int %one
%59 = OpLoad %int %two
%60 = OpISub %int %58 %59
%61 = OpLoad %int %two
%62 = OpCompositeConstruct %v2int %60 %61
%57 = OpSNegate %v2int %62
%63 = OpIEqual %v2bool %49 %57
%65 = OpAll %bool %63
%66 = OpSelect %int %65 %int_1 %int_0
%67 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %67 %66
%69 = OpLoad %v4int %result
%70 = OpCompositeExtract %int %69 0
%71 = OpLoad %v4int %result
%72 = OpCompositeExtract %int %71 1
%73 = OpIMul %int %70 %72
%74 = OpLoad %v4int %result
%75 = OpCompositeExtract %int %74 2
%76 = OpIMul %int %73 %75
%77 = OpLoad %v4int %result
%78 = OpCompositeExtract %int %77 3
%79 = OpIMul %int %76 %78
%80 = OpINotEqual %bool %79 %int_0
OpReturnValue %80
OpFunctionEnd
%main = OpFunction %v4float None %81
%82 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%149 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%91 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n1 %float_n1
%93 = OpFOrdEqual %v4bool %91 %92
%94 = OpAll %bool %93
%95 = OpSelect %int %94 %int_1 %int_0
%96 = OpConvertSToF %float %95
%97 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %97 %96
%98 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %98 %float_1
%100 = OpLoad %float %_1_two
%101 = OpCompositeConstruct %v4float %100 %100 %100 %100
%99 = OpFNegate %v4float %101
%103 = OpLoad %float %_1_two
%102 = OpFNegate %float %103
%105 = OpLoad %float %_1_two
%104 = OpFNegate %float %105
%106 = OpCompositeConstruct %v3float %104 %104 %104
%108 = OpCompositeExtract %float %106 0
%109 = OpCompositeExtract %float %106 1
%110 = OpCompositeExtract %float %106 2
%111 = OpCompositeConstruct %v4float %102 %108 %109 %110
%112 = OpFOrdEqual %v4bool %99 %111
%113 = OpAll %bool %112
%114 = OpSelect %int %113 %int_1 %int_0
%115 = OpConvertSToF %float %114
%116 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %116 %115
%121 = OpLoad %float %_0_one
%122 = OpLoad %float %_1_two
%123 = OpFSub %float %121 %122
%124 = OpLoad %float %_1_two
%125 = OpCompositeConstruct %v2float %123 %124
%120 = OpFNegate %v2float %125
%126 = OpFOrdEqual %v2bool %119 %120
%127 = OpAll %bool %126
%128 = OpSelect %int %127 %int_1 %int_0
%129 = OpConvertSToF %float %128
%130 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %130 %129
%132 = OpLoad %v4float %_2_result
%133 = OpCompositeExtract %float %132 0
%134 = OpLoad %v4float %_2_result
%135 = OpCompositeExtract %float %134 1
%136 = OpFMul %float %133 %135
%137 = OpLoad %v4float %_2_result
%138 = OpCompositeExtract %float %137 2
%139 = OpFMul %float %136 %138
%140 = OpLoad %v4float %_2_result
%141 = OpCompositeExtract %float %140 3
%142 = OpFMul %float %139 %141
%143 = OpFUnordNotEqual %bool %142 %float_0
OpSelectionMerge %146 None
OpBranchConditional %143 %145 %146
%145 = OpLabel
%147 = OpFunctionCall %bool %test_int_b
OpBranch %146
%146 = OpLabel
%148 = OpPhi %bool %false %82 %147 %145
OpSelectionMerge %152 None
OpBranchConditional %148 %150 %151
%150 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%155 = OpLoad %v4float %153
OpStore %149 %155
OpBranch %152
%151 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%157 = OpLoad %v4float %156
OpStore %149 %157
OpBranch %152
%152 = OpLabel
%158 = OpLoad %v4float %149
OpReturnValue %158
OpFunctionEnd
