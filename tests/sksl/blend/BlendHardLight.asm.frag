OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_overlay_component_Qhh2h2 "blend_overlay_component_Qhh2h2"
OpName %blend_overlay_h4h4h4 "blend_overlay_h4h4h4"
OpName %result "result"
OpName %blend_hard_light_h4h4h4 "blend_hard_light_h4h4h4"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%18 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%60 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%void = OpTypeVoid
%125 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_overlay_component_Qhh2h2 = OpFunction %float None %18
%19 = OpFunctionParameter %_ptr_Function_v2float
%20 = OpFunctionParameter %_ptr_Function_v2float
%21 = OpLabel
%29 = OpVariable %_ptr_Function_float Function
%23 = OpLoad %v2float %20
%24 = OpCompositeExtract %float %23 0
%25 = OpFMul %float %float_2 %24
%26 = OpLoad %v2float %20
%27 = OpCompositeExtract %float %26 1
%28 = OpFOrdLessThanEqual %bool %25 %27
OpSelectionMerge %33 None
OpBranchConditional %28 %31 %32
%31 = OpLabel
%34 = OpLoad %v2float %19
%35 = OpCompositeExtract %float %34 0
%36 = OpFMul %float %float_2 %35
%37 = OpLoad %v2float %20
%38 = OpCompositeExtract %float %37 0
%39 = OpFMul %float %36 %38
OpStore %29 %39
OpBranch %33
%32 = OpLabel
%40 = OpLoad %v2float %19
%41 = OpCompositeExtract %float %40 1
%42 = OpLoad %v2float %20
%43 = OpCompositeExtract %float %42 1
%44 = OpFMul %float %41 %43
%45 = OpLoad %v2float %20
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %20
%48 = OpCompositeExtract %float %47 0
%49 = OpFSub %float %46 %48
%50 = OpFMul %float %float_2 %49
%51 = OpLoad %v2float %19
%52 = OpCompositeExtract %float %51 1
%53 = OpLoad %v2float %19
%54 = OpCompositeExtract %float %53 0
%55 = OpFSub %float %52 %54
%56 = OpFMul %float %50 %55
%57 = OpFSub %float %44 %56
OpStore %29 %57
OpBranch %33
%33 = OpLabel
%58 = OpLoad %float %29
OpReturnValue %58
OpFunctionEnd
%blend_overlay_h4h4h4 = OpFunction %v4float None %60
%61 = OpFunctionParameter %_ptr_Function_v4float
%62 = OpFunctionParameter %_ptr_Function_v4float
%63 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%67 = OpVariable %_ptr_Function_v2float Function
%70 = OpVariable %_ptr_Function_v2float Function
%74 = OpVariable %_ptr_Function_v2float Function
%77 = OpVariable %_ptr_Function_v2float Function
%81 = OpVariable %_ptr_Function_v2float Function
%84 = OpVariable %_ptr_Function_v2float Function
%65 = OpLoad %v4float %61
%66 = OpVectorShuffle %v2float %65 %65 0 3
OpStore %67 %66
%68 = OpLoad %v4float %62
%69 = OpVectorShuffle %v2float %68 %68 0 3
OpStore %70 %69
%71 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %67 %70
%72 = OpLoad %v4float %61
%73 = OpVectorShuffle %v2float %72 %72 1 3
OpStore %74 %73
%75 = OpLoad %v4float %62
%76 = OpVectorShuffle %v2float %75 %75 1 3
OpStore %77 %76
%78 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %74 %77
%79 = OpLoad %v4float %61
%80 = OpVectorShuffle %v2float %79 %79 2 3
OpStore %81 %80
%82 = OpLoad %v4float %62
%83 = OpVectorShuffle %v2float %82 %82 2 3
OpStore %84 %83
%85 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %81 %84
%86 = OpLoad %v4float %61
%87 = OpCompositeExtract %float %86 3
%89 = OpLoad %v4float %61
%90 = OpCompositeExtract %float %89 3
%91 = OpFSub %float %float_1 %90
%92 = OpLoad %v4float %62
%93 = OpCompositeExtract %float %92 3
%94 = OpFMul %float %91 %93
%95 = OpFAdd %float %87 %94
%96 = OpCompositeConstruct %v4float %71 %78 %85 %95
OpStore %result %96
%97 = OpLoad %v4float %result
%98 = OpVectorShuffle %v3float %97 %97 0 1 2
%100 = OpLoad %v4float %62
%101 = OpVectorShuffle %v3float %100 %100 0 1 2
%102 = OpLoad %v4float %61
%103 = OpCompositeExtract %float %102 3
%104 = OpFSub %float %float_1 %103
%105 = OpVectorTimesScalar %v3float %101 %104
%106 = OpLoad %v4float %61
%107 = OpVectorShuffle %v3float %106 %106 0 1 2
%108 = OpLoad %v4float %62
%109 = OpCompositeExtract %float %108 3
%110 = OpFSub %float %float_1 %109
%111 = OpVectorTimesScalar %v3float %107 %110
%112 = OpFAdd %v3float %105 %111
%113 = OpFAdd %v3float %98 %112
%114 = OpLoad %v4float %result
%115 = OpVectorShuffle %v4float %114 %113 4 5 6 3
OpStore %result %115
OpReturnValue %115
OpFunctionEnd
%blend_hard_light_h4h4h4 = OpFunction %v4float None %60
%116 = OpFunctionParameter %_ptr_Function_v4float
%117 = OpFunctionParameter %_ptr_Function_v4float
%118 = OpLabel
%120 = OpVariable %_ptr_Function_v4float Function
%122 = OpVariable %_ptr_Function_v4float Function
%119 = OpLoad %v4float %117
OpStore %120 %119
%121 = OpLoad %v4float %116
OpStore %122 %121
%123 = OpFunctionCall %v4float %blend_overlay_h4h4h4 %120 %122
OpReturnValue %123
OpFunctionEnd
%main = OpFunction %void None %125
%126 = OpLabel
%132 = OpVariable %_ptr_Function_v4float Function
%136 = OpVariable %_ptr_Function_v4float Function
%127 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%131 = OpLoad %v4float %127
OpStore %132 %131
%133 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%135 = OpLoad %v4float %133
OpStore %136 %135
%137 = OpFunctionCall %v4float %blend_hard_light_h4h4h4 %132 %136
OpStore %sk_FragColor %137
OpReturn
OpFunctionEnd
