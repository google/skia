OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %constVal "constVal"
OpName %expected "expected"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %constVal RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%39 = OpConstantComposite %v4int %int_1 %int_0 %int_0 %int_2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%116 = OpConstantComposite %v2int %int_1 %int_0
%124 = OpConstantComposite %v3int %int_1 %int_0 %int_0
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%constVal = OpVariable %_ptr_Function_v4float Function
%expected = OpVariable %_ptr_Function_v4int Function
%147 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal %31
OpStore %expected %39
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %42
%45 = OpCompositeExtract %float %44 0
%46 = OpConvertFToS %int %45
%41 = OpExtInst %int %1 SAbs %46
%47 = OpLoad %v4int %expected
%48 = OpCompositeExtract %int %47 0
%49 = OpIEqual %bool %41 %48
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpVectorShuffle %v2float %54 %54 0 1
%56 = OpCompositeExtract %float %55 0
%57 = OpConvertFToS %int %56
%58 = OpCompositeExtract %float %55 1
%59 = OpConvertFToS %int %58
%60 = OpCompositeConstruct %v2int %57 %59
%52 = OpExtInst %v2int %1 SAbs %60
%62 = OpLoad %v4int %expected
%63 = OpVectorShuffle %v2int %62 %62 0 1
%64 = OpIEqual %v2bool %52 %63
%66 = OpAll %bool %64
OpBranch %51
%51 = OpLabel
%67 = OpPhi %bool %false %25 %66 %50
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%75 = OpCompositeExtract %float %73 0
%76 = OpConvertFToS %int %75
%77 = OpCompositeExtract %float %73 1
%78 = OpConvertFToS %int %77
%79 = OpCompositeExtract %float %73 2
%80 = OpConvertFToS %int %79
%81 = OpCompositeConstruct %v3int %76 %78 %80
%70 = OpExtInst %v3int %1 SAbs %81
%83 = OpLoad %v4int %expected
%84 = OpVectorShuffle %v3int %83 %83 0 1 2
%85 = OpIEqual %v3bool %70 %84
%87 = OpAll %bool %85
OpBranch %69
%69 = OpLabel
%88 = OpPhi %bool %false %51 %87 %68
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%93 = OpLoad %v4float %92
%94 = OpCompositeExtract %float %93 0
%95 = OpConvertFToS %int %94
%96 = OpCompositeExtract %float %93 1
%97 = OpConvertFToS %int %96
%98 = OpCompositeExtract %float %93 2
%99 = OpConvertFToS %int %98
%100 = OpCompositeExtract %float %93 3
%101 = OpConvertFToS %int %100
%102 = OpCompositeConstruct %v4int %95 %97 %99 %101
%91 = OpExtInst %v4int %1 SAbs %102
%103 = OpLoad %v4int %expected
%104 = OpIEqual %v4bool %91 %103
%106 = OpAll %bool %104
OpBranch %90
%90 = OpLabel
%107 = OpPhi %bool %false %69 %106 %89
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpLoad %v4int %expected
%111 = OpCompositeExtract %int %110 0
%112 = OpIEqual %bool %int_1 %111
OpBranch %109
%109 = OpLabel
%113 = OpPhi %bool %false %90 %112 %108
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpLoad %v4int %expected
%118 = OpVectorShuffle %v2int %117 %117 0 1
%119 = OpIEqual %v2bool %116 %118
%120 = OpAll %bool %119
OpBranch %115
%115 = OpLabel
%121 = OpPhi %bool %false %109 %120 %114
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%125 = OpLoad %v4int %expected
%126 = OpVectorShuffle %v3int %125 %125 0 1 2
%127 = OpIEqual %v3bool %124 %126
%128 = OpAll %bool %127
OpBranch %123
%123 = OpLabel
%129 = OpPhi %bool %false %115 %128 %122
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%133 = OpLoad %v4float %constVal
%134 = OpCompositeExtract %float %133 0
%135 = OpConvertFToS %int %134
%136 = OpCompositeExtract %float %133 1
%137 = OpConvertFToS %int %136
%138 = OpCompositeExtract %float %133 2
%139 = OpConvertFToS %int %138
%140 = OpCompositeExtract %float %133 3
%141 = OpConvertFToS %int %140
%142 = OpCompositeConstruct %v4int %135 %137 %139 %141
%132 = OpExtInst %v4int %1 SAbs %142
%143 = OpLoad %v4int %expected
%144 = OpIEqual %v4bool %132 %143
%145 = OpAll %bool %144
OpBranch %131
%131 = OpLabel
%146 = OpPhi %bool %false %123 %145 %130
OpSelectionMerge %150 None
OpBranchConditional %146 %148 %149
%148 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%152 = OpLoad %v4float %151
OpStore %147 %152
OpBranch %150
%149 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%154 = OpLoad %v4float %153
OpStore %147 %154
OpBranch %150
%150 = OpLabel
%155 = OpLoad %v4float %147
OpReturnValue %155
OpFunctionEnd
