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
OpName %unpremul "unpremul"
OpName %unpremul_float "unpremul_float"
OpName %dead_fn "dead_fn"
OpName %live_fn "live_fn"
OpName %main "main"
OpName %TRUE "TRUE"
OpName %FALSE "FALSE"
OpName %a "a"
OpName %b "b"
OpName %unused "unused"
OpName %unused_0 "unused"
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
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%22 = OpTypeFunction %v4float %_ptr_Function_v4float
%v3float = OpTypeVector %float 3
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
%float_1 = OpConstant %float 1
%57 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%70 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_0_5 = OpConstant %float 0.5
%85 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%float_2 = OpConstant %float 2
%88 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_3 = OpConstant %float 3
%92 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%float_n5 = OpConstant %float -5
%95 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
%102 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_n1 = OpConstant %float -1
%107 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%float_0 = OpConstant %float 0
%112 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %19
%20 = OpLabel
%21 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%unpremul = OpFunction %v4float None %22
%24 = OpFunctionParameter %_ptr_Function_v4float
%25 = OpLabel
%26 = OpLoad %v4float %24
%27 = OpVectorShuffle %v3float %26 %26 0 1 2
%30 = OpLoad %v4float %24
%31 = OpCompositeExtract %float %30 3
%29 = OpExtInst %float %1 FMax %31 %float_9_99999975en05
%34 = OpFDiv %float %float_1 %29
%35 = OpVectorTimesScalar %v3float %27 %34
%36 = OpCompositeExtract %float %35 0
%37 = OpCompositeExtract %float %35 1
%38 = OpCompositeExtract %float %35 2
%39 = OpLoad %v4float %24
%40 = OpCompositeExtract %float %39 3
%41 = OpCompositeConstruct %v4float %36 %37 %38 %40
OpReturnValue %41
OpFunctionEnd
%unpremul_float = OpFunction %v4float None %22
%42 = OpFunctionParameter %_ptr_Function_v4float
%43 = OpLabel
%44 = OpLoad %v4float %42
%45 = OpVectorShuffle %v3float %44 %44 0 1 2
%47 = OpLoad %v4float %42
%48 = OpCompositeExtract %float %47 3
%46 = OpExtInst %float %1 FMax %48 %float_9_99999975en05
%49 = OpFDiv %float %float_1 %46
%50 = OpVectorTimesScalar %v3float %45 %49
%51 = OpCompositeExtract %float %50 0
%52 = OpCompositeExtract %float %50 1
%53 = OpCompositeExtract %float %50 2
%54 = OpLoad %v4float %42
%55 = OpCompositeExtract %float %54 3
%56 = OpCompositeConstruct %v4float %51 %52 %53 %55
OpReturnValue %56
OpFunctionEnd
%dead_fn = OpFunction %v4float None %57
%58 = OpFunctionParameter %_ptr_Function_v4float
%59 = OpFunctionParameter %_ptr_Function_v4float
%60 = OpLabel
%61 = OpLoad %v4float %58
%62 = OpLoad %v4float %59
%63 = OpFMul %v4float %61 %62
OpReturnValue %63
OpFunctionEnd
%live_fn = OpFunction %v4float None %57
%64 = OpFunctionParameter %_ptr_Function_v4float
%65 = OpFunctionParameter %_ptr_Function_v4float
%66 = OpLabel
%67 = OpLoad %v4float %64
%68 = OpLoad %v4float %65
%69 = OpFAdd %v4float %67 %68
OpReturnValue %69
OpFunctionEnd
%main = OpFunction %v4float None %70
%71 = OpLabel
%TRUE = OpVariable %_ptr_Function_bool Function
%FALSE = OpVariable %_ptr_Function_bool Function
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%unused = OpVariable %_ptr_Function_v4float Function
%86 = OpVariable %_ptr_Function_v4float Function
%89 = OpVariable %_ptr_Function_v4float Function
%93 = OpVariable %_ptr_Function_v4float Function
%96 = OpVariable %_ptr_Function_v4float Function
%103 = OpVariable %_ptr_Function_v4float Function
%unused_0 = OpVariable %_ptr_Function_v4float Function
%108 = OpVariable %_ptr_Function_v4float Function
%122 = OpVariable %_ptr_Function_v4float Function
OpStore %TRUE %true
OpStore %FALSE %false
%79 = OpLoad %bool %FALSE
OpSelectionMerge %82 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
OpStore %86 %85
OpStore %89 %88
%90 = OpFunctionCall %v4float %dead_fn %86 %89
OpStore %unused %90
OpBranch %82
%81 = OpLabel
OpStore %93 %92
OpStore %96 %95
%97 = OpFunctionCall %v4float %live_fn %93 %96
OpStore %a %97
OpBranch %82
%82 = OpLabel
%98 = OpLoad %bool %TRUE
OpSelectionMerge %101 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
OpStore %103 %102
%104 = OpFunctionCall %v4float %unpremul %103
OpStore %b %104
OpBranch %101
%100 = OpLabel
OpStore %108 %107
%109 = OpFunctionCall %v4float %unpremul_float %108
OpStore %unused_0 %109
OpBranch %101
%101 = OpLabel
%110 = OpLoad %v4float %a
%113 = OpFOrdNotEqual %v4bool %110 %112
%115 = OpAny %bool %113
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpLoad %v4float %b
%119 = OpFOrdNotEqual %v4bool %118 %112
%120 = OpAny %bool %119
OpBranch %117
%117 = OpLabel
%121 = OpPhi %bool %false %101 %120 %116
OpSelectionMerge %125 None
OpBranchConditional %121 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%130 = OpLoad %v4float %126
OpStore %122 %130
OpBranch %125
%124 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%133 = OpLoad %v4float %131
OpStore %122 %133
OpBranch %125
%125 = OpLabel
%134 = OpLoad %v4float %122
OpReturnValue %134
OpFunctionEnd
