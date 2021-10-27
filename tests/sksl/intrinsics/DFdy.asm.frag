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
OpMemberName %_UniformBuffer 3 "u_skRTFlip"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
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
OpMemberDecorate %_UniformBuffer 3 Offset 16384
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_3 = OpConstant %int 3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%123 = OpConstantComposite %v2float %float_1 %float_1
%137 = OpConstantComposite %v2float %float_0 %float_1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%expected = OpVariable %_ptr_Function_v4float Function
%141 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %29
%32 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpDPdy %float %37
%39 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%41 = OpLoad %v2float %39
%42 = OpCompositeExtract %float %41 1
%43 = OpFMul %float %31 %42
%44 = OpLoad %v4float %expected
%45 = OpCompositeExtract %float %44 0
%46 = OpFOrdEqual %bool %43 %45
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v2float %51 %51 0 1
%49 = OpDPdy %v2float %52
%53 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%54 = OpLoad %v2float %53
%55 = OpCompositeExtract %float %54 1
%56 = OpCompositeConstruct %v2float %55 %55
%57 = OpFMul %v2float %49 %56
%58 = OpLoad %v4float %expected
%59 = OpVectorShuffle %v2float %58 %58 0 1
%60 = OpFOrdEqual %v2bool %57 %59
%62 = OpAll %bool %60
OpBranch %48
%48 = OpLabel
%63 = OpPhi %bool %false %26 %62 %47
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%66 = OpDPdy %v3float %69
%71 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%72 = OpLoad %v2float %71
%73 = OpCompositeExtract %float %72 1
%74 = OpCompositeConstruct %v3float %73 %73 %73
%75 = OpFMul %v3float %66 %74
%76 = OpLoad %v4float %expected
%77 = OpVectorShuffle %v3float %76 %76 0 1 2
%78 = OpFOrdEqual %v3bool %75 %77
%80 = OpAll %bool %78
OpBranch %65
%65 = OpLabel
%81 = OpPhi %bool %false %48 %80 %64
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%86 = OpLoad %v4float %85
%84 = OpDPdy %v4float %86
%87 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%88 = OpLoad %v2float %87
%89 = OpCompositeExtract %float %88 1
%90 = OpCompositeConstruct %v4float %89 %89 %89 %89
%91 = OpFMul %v4float %84 %90
%92 = OpLoad %v4float %expected
%93 = OpFOrdEqual %v4bool %91 %92
%95 = OpAll %bool %93
OpBranch %83
%83 = OpLabel
%96 = OpPhi %bool %false %65 %95 %82
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%101 = OpLoad %v2float %25
%102 = OpVectorShuffle %v2float %101 %101 0 0
%100 = OpDPdy %v2float %102
%103 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%104 = OpLoad %v2float %103
%105 = OpCompositeExtract %float %104 1
%106 = OpCompositeConstruct %v2float %105 %105
%107 = OpFMul %v2float %100 %106
%99 = OpExtInst %v2float %1 FSign %107
%108 = OpFOrdEqual %v2bool %99 %20
%109 = OpAll %bool %108
OpBranch %98
%98 = OpLabel
%110 = OpPhi %bool %false %83 %109 %97
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%115 = OpLoad %v2float %25
%116 = OpVectorShuffle %v2float %115 %115 1 1
%114 = OpDPdy %v2float %116
%117 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%118 = OpLoad %v2float %117
%119 = OpCompositeExtract %float %118 1
%120 = OpCompositeConstruct %v2float %119 %119
%121 = OpFMul %v2float %114 %120
%113 = OpExtInst %v2float %1 FSign %121
%124 = OpFOrdEqual %v2bool %113 %123
%125 = OpAll %bool %124
OpBranch %112
%112 = OpLabel
%126 = OpPhi %bool %false %98 %125 %111
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%131 = OpLoad %v2float %25
%130 = OpDPdy %v2float %131
%132 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%133 = OpLoad %v2float %132
%134 = OpCompositeExtract %float %133 1
%135 = OpCompositeConstruct %v2float %134 %134
%136 = OpFMul %v2float %130 %135
%129 = OpExtInst %v2float %1 FSign %136
%138 = OpFOrdEqual %v2bool %129 %137
%139 = OpAll %bool %138
OpBranch %128
%128 = OpLabel
%140 = OpPhi %bool %false %112 %139 %127
OpSelectionMerge %144 None
OpBranchConditional %140 %142 %143
%142 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%147 = OpLoad %v4float %145
OpStore %141 %147
OpBranch %144
%143 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%150 = OpLoad %v4float %148
OpStore %141 %150
OpBranch %144
%144 = OpLabel
%151 = OpLoad %v4float %141
OpReturnValue %151
OpFunctionEnd
