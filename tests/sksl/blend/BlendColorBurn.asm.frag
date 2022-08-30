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
OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
OpName %delta "delta"
OpName %blend_color_burn_h4h4h4 "blend_color_burn_h4h4h4"
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
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
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
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
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
%114 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%150 = OpTypeFunction %void
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
%color_burn_component_Qhh2h2 = OpFunction %float None %34
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpFunctionParameter %_ptr_Function_v2float
%37 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%90 = OpVariable %_ptr_Function_float Function
%93 = OpVariable %_ptr_Function_float Function
%38 = OpLoad %v2float %36
%39 = OpCompositeExtract %float %38 1
%40 = OpLoad %v2float %36
%41 = OpCompositeExtract %float %40 0
%42 = OpFOrdEqual %bool %39 %41
OpSelectionMerge %45 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpLoad %v2float %35
%47 = OpCompositeExtract %float %46 1
%48 = OpLoad %v2float %36
%49 = OpCompositeExtract %float %48 1
%50 = OpFMul %float %47 %49
%51 = OpLoad %v2float %35
%52 = OpCompositeExtract %float %51 0
%54 = OpLoad %v2float %36
%55 = OpCompositeExtract %float %54 1
%56 = OpFSub %float %float_1 %55
%57 = OpFMul %float %52 %56
%58 = OpFAdd %float %50 %57
%59 = OpLoad %v2float %36
%60 = OpCompositeExtract %float %59 0
%61 = OpLoad %v2float %35
%62 = OpCompositeExtract %float %61 1
%63 = OpFSub %float %float_1 %62
%64 = OpFMul %float %60 %63
%65 = OpFAdd %float %58 %64
OpReturnValue %65
%44 = OpLabel
%66 = OpLoad %v2float %35
%67 = OpCompositeExtract %float %66 0
%68 = OpFOrdEqual %bool %67 %float_0
OpSelectionMerge %71 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpLoad %v2float %36
%73 = OpCompositeExtract %float %72 0
%74 = OpLoad %v2float %35
%75 = OpCompositeExtract %float %74 1
%76 = OpFSub %float %float_1 %75
%77 = OpFMul %float %73 %76
OpReturnValue %77
%70 = OpLabel
%80 = OpLoad %v2float %36
%81 = OpCompositeExtract %float %80 1
%82 = OpLoad %v2float %36
%83 = OpCompositeExtract %float %82 1
%84 = OpLoad %v2float %36
%85 = OpCompositeExtract %float %84 0
%86 = OpFSub %float %83 %85
%87 = OpLoad %v2float %35
%88 = OpCompositeExtract %float %87 1
%89 = OpFMul %float %86 %88
OpStore %90 %89
%91 = OpLoad %v2float %35
%92 = OpCompositeExtract %float %91 0
OpStore %93 %92
%94 = OpFunctionCall %float %guarded_divide_Qhhh %90 %93
%95 = OpFSub %float %81 %94
%79 = OpExtInst %float %1 FMax %float_0 %95
OpStore %delta %79
%96 = OpLoad %v2float %35
%97 = OpCompositeExtract %float %96 1
%98 = OpFMul %float %79 %97
%99 = OpLoad %v2float %35
%100 = OpCompositeExtract %float %99 0
%101 = OpLoad %v2float %36
%102 = OpCompositeExtract %float %101 1
%103 = OpFSub %float %float_1 %102
%104 = OpFMul %float %100 %103
%105 = OpFAdd %float %98 %104
%106 = OpLoad %v2float %36
%107 = OpCompositeExtract %float %106 0
%108 = OpLoad %v2float %35
%109 = OpCompositeExtract %float %108 1
%110 = OpFSub %float %float_1 %109
%111 = OpFMul %float %107 %110
%112 = OpFAdd %float %105 %111
OpReturnValue %112
%71 = OpLabel
OpBranch %45
%45 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color_burn_h4h4h4 = OpFunction %v4float None %114
%115 = OpFunctionParameter %_ptr_Function_v4float
%116 = OpFunctionParameter %_ptr_Function_v4float
%117 = OpLabel
%120 = OpVariable %_ptr_Function_v2float Function
%123 = OpVariable %_ptr_Function_v2float Function
%127 = OpVariable %_ptr_Function_v2float Function
%130 = OpVariable %_ptr_Function_v2float Function
%134 = OpVariable %_ptr_Function_v2float Function
%137 = OpVariable %_ptr_Function_v2float Function
%118 = OpLoad %v4float %115
%119 = OpVectorShuffle %v2float %118 %118 0 3
OpStore %120 %119
%121 = OpLoad %v4float %116
%122 = OpVectorShuffle %v2float %121 %121 0 3
OpStore %123 %122
%124 = OpFunctionCall %float %color_burn_component_Qhh2h2 %120 %123
%125 = OpLoad %v4float %115
%126 = OpVectorShuffle %v2float %125 %125 1 3
OpStore %127 %126
%128 = OpLoad %v4float %116
%129 = OpVectorShuffle %v2float %128 %128 1 3
OpStore %130 %129
%131 = OpFunctionCall %float %color_burn_component_Qhh2h2 %127 %130
%132 = OpLoad %v4float %115
%133 = OpVectorShuffle %v2float %132 %132 2 3
OpStore %134 %133
%135 = OpLoad %v4float %116
%136 = OpVectorShuffle %v2float %135 %135 2 3
OpStore %137 %136
%138 = OpFunctionCall %float %color_burn_component_Qhh2h2 %134 %137
%139 = OpLoad %v4float %115
%140 = OpCompositeExtract %float %139 3
%141 = OpLoad %v4float %115
%142 = OpCompositeExtract %float %141 3
%143 = OpFSub %float %float_1 %142
%144 = OpLoad %v4float %116
%145 = OpCompositeExtract %float %144 3
%146 = OpFMul %float %143 %145
%147 = OpFAdd %float %140 %146
%148 = OpCompositeConstruct %v4float %124 %131 %138 %147
OpReturnValue %148
OpFunctionEnd
%main = OpFunction %void None %150
%151 = OpLabel
%157 = OpVariable %_ptr_Function_v4float Function
%161 = OpVariable %_ptr_Function_v4float Function
%11 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_GuardedDivideEpsilon %11
%152 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%156 = OpLoad %v4float %152
OpStore %157 %156
%158 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%160 = OpLoad %v4float %158
OpStore %161 %160
%162 = OpFunctionCall %v4float %blend_color_burn_h4h4h4 %157 %161
OpStore %sk_FragColor %162
OpReturn
OpFunctionEnd
