OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %_GuardedDivideEpsilon "$GuardedDivideEpsilon"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
OpName %delta "delta"
OpName %blend_color_dodge_h4h4h4 "blend_color_dodge_h4h4h4"
OpName %main "main"
OpDecorate %_GuardedDivideEpsilon RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_GuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
%bool = OpTypeBool
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%23 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%34 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%109 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%145 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%guarded_divide_Qhhh = OpFunction %float None %23
%24 = OpFunctionParameter %_ptr_Function_float
%25 = OpFunctionParameter %_ptr_Function_float
%26 = OpLabel
%27 = OpLoad %float %24
%28 = OpLoad %float %25
%29 = OpLoad %float %_GuardedDivideEpsilon
%30 = OpFAdd %float %28 %29
%31 = OpFDiv %float %27 %30
OpReturnValue %31
OpFunctionEnd
%color_dodge_component_Qhh2h2 = OpFunction %float None %34
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpFunctionParameter %_ptr_Function_v2float
%37 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%88 = OpVariable %_ptr_Function_float Function
%89 = OpVariable %_ptr_Function_float Function
%38 = OpLoad %v2float %36
%39 = OpCompositeExtract %float %38 0
%40 = OpFOrdEqual %bool %39 %float_0
OpSelectionMerge %43 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%44 = OpLoad %v2float %35
%45 = OpCompositeExtract %float %44 0
%47 = OpLoad %v2float %36
%48 = OpCompositeExtract %float %47 1
%49 = OpFSub %float %float_1 %48
%50 = OpFMul %float %45 %49
OpReturnValue %50
%42 = OpLabel
%52 = OpLoad %v2float %35
%53 = OpCompositeExtract %float %52 1
%54 = OpLoad %v2float %35
%55 = OpCompositeExtract %float %54 0
%56 = OpFSub %float %53 %55
OpStore %delta %56
%57 = OpFOrdEqual %bool %56 %float_0
OpSelectionMerge %60 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpLoad %v2float %35
%62 = OpCompositeExtract %float %61 1
%63 = OpLoad %v2float %36
%64 = OpCompositeExtract %float %63 1
%65 = OpFMul %float %62 %64
%66 = OpLoad %v2float %35
%67 = OpCompositeExtract %float %66 0
%68 = OpLoad %v2float %36
%69 = OpCompositeExtract %float %68 1
%70 = OpFSub %float %float_1 %69
%71 = OpFMul %float %67 %70
%72 = OpFAdd %float %65 %71
%73 = OpLoad %v2float %36
%74 = OpCompositeExtract %float %73 0
%75 = OpLoad %v2float %35
%76 = OpCompositeExtract %float %75 1
%77 = OpFSub %float %float_1 %76
%78 = OpFMul %float %74 %77
%79 = OpFAdd %float %72 %78
OpReturnValue %79
%59 = OpLabel
%81 = OpLoad %v2float %36
%82 = OpCompositeExtract %float %81 1
%83 = OpLoad %v2float %36
%84 = OpCompositeExtract %float %83 0
%85 = OpLoad %v2float %35
%86 = OpCompositeExtract %float %85 1
%87 = OpFMul %float %84 %86
OpStore %88 %87
OpStore %89 %56
%90 = OpFunctionCall %float %guarded_divide_Qhhh %88 %89
%80 = OpExtInst %float %1 FMin %82 %90
OpStore %delta %80
%91 = OpLoad %v2float %35
%92 = OpCompositeExtract %float %91 1
%93 = OpFMul %float %80 %92
%94 = OpLoad %v2float %35
%95 = OpCompositeExtract %float %94 0
%96 = OpLoad %v2float %36
%97 = OpCompositeExtract %float %96 1
%98 = OpFSub %float %float_1 %97
%99 = OpFMul %float %95 %98
%100 = OpFAdd %float %93 %99
%101 = OpLoad %v2float %36
%102 = OpCompositeExtract %float %101 0
%103 = OpLoad %v2float %35
%104 = OpCompositeExtract %float %103 1
%105 = OpFSub %float %float_1 %104
%106 = OpFMul %float %102 %105
%107 = OpFAdd %float %100 %106
OpReturnValue %107
%60 = OpLabel
OpBranch %43
%43 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color_dodge_h4h4h4 = OpFunction %v4float None %109
%110 = OpFunctionParameter %_ptr_Function_v4float
%111 = OpFunctionParameter %_ptr_Function_v4float
%112 = OpLabel
%115 = OpVariable %_ptr_Function_v2float Function
%118 = OpVariable %_ptr_Function_v2float Function
%122 = OpVariable %_ptr_Function_v2float Function
%125 = OpVariable %_ptr_Function_v2float Function
%129 = OpVariable %_ptr_Function_v2float Function
%132 = OpVariable %_ptr_Function_v2float Function
%113 = OpLoad %v4float %110
%114 = OpVectorShuffle %v2float %113 %113 0 3
OpStore %115 %114
%116 = OpLoad %v4float %111
%117 = OpVectorShuffle %v2float %116 %116 0 3
OpStore %118 %117
%119 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %115 %118
%120 = OpLoad %v4float %110
%121 = OpVectorShuffle %v2float %120 %120 1 3
OpStore %122 %121
%123 = OpLoad %v4float %111
%124 = OpVectorShuffle %v2float %123 %123 1 3
OpStore %125 %124
%126 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %122 %125
%127 = OpLoad %v4float %110
%128 = OpVectorShuffle %v2float %127 %127 2 3
OpStore %129 %128
%130 = OpLoad %v4float %111
%131 = OpVectorShuffle %v2float %130 %130 2 3
OpStore %132 %131
%133 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %129 %132
%134 = OpLoad %v4float %110
%135 = OpCompositeExtract %float %134 3
%136 = OpLoad %v4float %110
%137 = OpCompositeExtract %float %136 3
%138 = OpFSub %float %float_1 %137
%139 = OpLoad %v4float %111
%140 = OpCompositeExtract %float %139 3
%141 = OpFMul %float %138 %140
%142 = OpFAdd %float %135 %141
%143 = OpCompositeConstruct %v4float %119 %126 %133 %142
OpReturnValue %143
OpFunctionEnd
%main = OpFunction %void None %145
%146 = OpLabel
%152 = OpVariable %_ptr_Function_v4float Function
%156 = OpVariable %_ptr_Function_v4float Function
%11 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_GuardedDivideEpsilon %11
%147 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%151 = OpLoad %v4float %147
OpStore %152 %151
%153 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%155 = OpLoad %v4float %153
OpStore %156 %155
%157 = OpFunctionCall %v4float %blend_color_dodge_h4h4h4 %152 %156
OpStore %sk_FragColor %157
OpReturn
OpFunctionEnd
