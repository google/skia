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
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %94 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
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
%int_0 = OpConstant %int 0
%v3int = OpTypeVector %int 3
%v4bool = OpTypeVector %bool 4
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%83 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v3float = OpTypeVector %float 3
%v2float = OpTypeVector %float 2
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
%37 = OpLoad %int %two
%36 = OpSNegate %int %37
%39 = OpLoad %int %two
%38 = OpSNegate %int %39
%40 = OpCompositeConstruct %v3int %38 %38 %38
%42 = OpCompositeExtract %int %40 0
%43 = OpCompositeExtract %int %40 1
%44 = OpCompositeExtract %int %40 2
%45 = OpCompositeConstruct %v4int %36 %42 %43 %44
%46 = OpIEqual %v4bool %33 %45
%48 = OpAll %bool %46
%49 = OpSelect %int %48 %int_1 %int_0
%50 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %50 %49
%54 = OpLoad %int %one
%53 = OpSNegate %int %54
%55 = OpLoad %int %one
%56 = OpLoad %int %one
%57 = OpIAdd %int %55 %56
%58 = OpCompositeConstruct %v2int %53 %57
%51 = OpSNegate %v2int %58
%60 = OpLoad %int %one
%61 = OpLoad %int %two
%62 = OpISub %int %60 %61
%63 = OpLoad %int %two
%64 = OpCompositeConstruct %v2int %62 %63
%59 = OpSNegate %v2int %64
%65 = OpIEqual %v2bool %51 %59
%67 = OpAll %bool %65
%68 = OpSelect %int %67 %int_1 %int_0
%69 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %69 %68
%71 = OpLoad %v4int %result
%72 = OpCompositeExtract %int %71 0
%73 = OpLoad %v4int %result
%74 = OpCompositeExtract %int %73 1
%75 = OpIMul %int %72 %74
%76 = OpLoad %v4int %result
%77 = OpCompositeExtract %int %76 2
%78 = OpIMul %int %75 %77
%79 = OpLoad %v4int %result
%80 = OpCompositeExtract %int %79 3
%81 = OpIMul %int %78 %80
%82 = OpINotEqual %bool %81 %int_0
OpReturnValue %82
OpFunctionEnd
%main = OpFunction %v4float None %83
%84 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%149 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%92 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %92 %float_1
%93 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %93 %float_1
%95 = OpLoad %float %_1_two
%96 = OpCompositeConstruct %v4float %95 %95 %95 %95
%94 = OpFNegate %v4float %96
%98 = OpLoad %float %_1_two
%97 = OpFNegate %float %98
%100 = OpLoad %float %_1_two
%99 = OpFNegate %float %100
%101 = OpCompositeConstruct %v3float %99 %99 %99
%103 = OpCompositeExtract %float %101 0
%104 = OpCompositeExtract %float %101 1
%105 = OpCompositeExtract %float %101 2
%106 = OpCompositeConstruct %v4float %97 %103 %104 %105
%107 = OpFOrdEqual %v4bool %94 %106
%108 = OpAll %bool %107
%109 = OpSelect %int %108 %int_1 %int_0
%110 = OpConvertSToF %float %109
%111 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %111 %110
%115 = OpLoad %float %_0_one
%114 = OpFNegate %float %115
%116 = OpLoad %float %_0_one
%117 = OpLoad %float %_0_one
%118 = OpFAdd %float %116 %117
%119 = OpCompositeConstruct %v2float %114 %118
%112 = OpFNegate %v2float %119
%121 = OpLoad %float %_0_one
%122 = OpLoad %float %_1_two
%123 = OpFSub %float %121 %122
%124 = OpLoad %float %_1_two
%125 = OpCompositeConstruct %v2float %123 %124
%120 = OpFNegate %v2float %125
%126 = OpFOrdEqual %v2bool %112 %120
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
%147 = OpFunctionCall %bool %test_int
OpBranch %146
%146 = OpLabel
%148 = OpPhi %bool %false %84 %147 %145
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
