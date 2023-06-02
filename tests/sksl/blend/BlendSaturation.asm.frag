OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
OpName %blend_hslc_h4h2h4h4 "blend_hslc_h4h2h4h4"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_mn "_2_mn"
OpName %_3_mx "_3_mx"
OpName %_4_lum "_4_lum"
OpName %_5_result "_5_result"
OpName %_6_minComp "_6_minComp"
OpName %_7_maxComp "_7_maxComp"
OpName %main "main"
OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %18 Binding 0
OpDecorate %18 DescriptorSet 0
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %_2_mn RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %_3_mx RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
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
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%23 = OpTypeFunction %float %_ptr_Function_v3float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%46 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Function_float = OpTypePointer Function %float
%v3bool = OpTypeVector %bool 3
%float_1 = OpConstant %float 1
%111 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%118 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%void = OpTypeVoid
%189 = OpTypeFunction %void
%191 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_color_saturation_Qhh3 = OpFunction %float None %23
%24 = OpFunctionParameter %_ptr_Function_v3float
%25 = OpLabel
%28 = OpLoad %v3float %24
%29 = OpCompositeExtract %float %28 0
%30 = OpLoad %v3float %24
%31 = OpCompositeExtract %float %30 1
%27 = OpExtInst %float %1 FMax %29 %31
%32 = OpLoad %v3float %24
%33 = OpCompositeExtract %float %32 2
%26 = OpExtInst %float %1 FMax %27 %33
%36 = OpLoad %v3float %24
%37 = OpCompositeExtract %float %36 0
%38 = OpLoad %v3float %24
%39 = OpCompositeExtract %float %38 1
%35 = OpExtInst %float %1 FMin %37 %39
%40 = OpLoad %v3float %24
%41 = OpCompositeExtract %float %40 2
%34 = OpExtInst %float %1 FMin %35 %41
%42 = OpFSub %float %26 %34
OpReturnValue %42
OpFunctionEnd
%blend_hslc_h4h2h4h4 = OpFunction %v4float None %46
%47 = OpFunctionParameter %_ptr_Function_v2float
%48 = OpFunctionParameter %_ptr_Function_v4float
%49 = OpFunctionParameter %_ptr_Function_v4float
%50 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%_2_mn = OpVariable %_ptr_Function_float Function
%_3_mx = OpVariable %_ptr_Function_float Function
%98 = OpVariable %_ptr_Function_v3float Function
%104 = OpVariable %_ptr_Function_v3float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
%53 = OpLoad %v4float %49
%54 = OpCompositeExtract %float %53 3
%55 = OpLoad %v4float %48
%56 = OpCompositeExtract %float %55 3
%57 = OpFMul %float %54 %56
OpStore %alpha %57
%59 = OpLoad %v4float %48
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%61 = OpLoad %v4float %49
%62 = OpCompositeExtract %float %61 3
%63 = OpVectorTimesScalar %v3float %60 %62
OpStore %sda %63
%65 = OpLoad %v4float %49
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%67 = OpLoad %v4float %48
%68 = OpCompositeExtract %float %67 3
%69 = OpVectorTimesScalar %v3float %66 %68
OpStore %dsa %69
%71 = OpLoad %v2float %47
%72 = OpCompositeExtract %float %71 0
%73 = OpFUnordNotEqual %bool %72 %float_0
%75 = OpCompositeConstruct %v3bool %73 %73 %73
%76 = OpSelect %v3float %75 %69 %63
OpStore %l %76
%78 = OpLoad %v2float %47
%79 = OpCompositeExtract %float %78 0
%80 = OpFUnordNotEqual %bool %79 %float_0
%81 = OpCompositeConstruct %v3bool %80 %80 %80
%82 = OpSelect %v3float %81 %63 %69
OpStore %r %82
%83 = OpLoad %v2float %47
%84 = OpCompositeExtract %float %83 1
%85 = OpFUnordNotEqual %bool %84 %float_0
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%91 = OpCompositeExtract %float %76 0
%92 = OpCompositeExtract %float %76 1
%90 = OpExtInst %float %1 FMin %91 %92
%93 = OpCompositeExtract %float %76 2
%89 = OpExtInst %float %1 FMin %90 %93
OpStore %_2_mn %89
%96 = OpExtInst %float %1 FMax %91 %92
%95 = OpExtInst %float %1 FMax %96 %93
OpStore %_3_mx %95
%97 = OpFOrdGreaterThan %bool %95 %89
OpSelectionMerge %101 None
OpBranchConditional %97 %99 %100
%99 = OpLabel
%102 = OpCompositeConstruct %v3float %89 %89 %89
%103 = OpFSub %v3float %76 %102
OpStore %104 %82
%105 = OpFunctionCall %float %blend_color_saturation_Qhh3 %104
%106 = OpVectorTimesScalar %v3float %103 %105
%107 = OpFSub %float %95 %89
%109 = OpFDiv %float %float_1 %107
%110 = OpVectorTimesScalar %v3float %106 %109
OpStore %98 %110
OpBranch %101
%100 = OpLabel
OpStore %98 %111
OpBranch %101
%101 = OpLabel
%112 = OpLoad %v3float %98
OpStore %l %112
OpStore %r %69
OpBranch %87
%87 = OpLabel
%119 = OpLoad %v3float %r
%114 = OpDot %float %118 %119
OpStore %_4_lum %114
%122 = OpLoad %v3float %l
%121 = OpDot %float %118 %122
%123 = OpFSub %float %114 %121
%124 = OpLoad %v3float %l
%125 = OpCompositeConstruct %v3float %123 %123 %123
%126 = OpFAdd %v3float %125 %124
OpStore %_5_result %126
%130 = OpCompositeExtract %float %126 0
%131 = OpCompositeExtract %float %126 1
%129 = OpExtInst %float %1 FMin %130 %131
%132 = OpCompositeExtract %float %126 2
%128 = OpExtInst %float %1 FMin %129 %132
OpStore %_6_minComp %128
%135 = OpExtInst %float %1 FMax %130 %131
%134 = OpExtInst %float %1 FMax %135 %132
OpStore %_7_maxComp %134
%136 = OpFOrdLessThan %bool %128 %float_0
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%139 = OpFUnordNotEqual %bool %114 %128
OpBranch %138
%138 = OpLabel
%140 = OpPhi %bool %false %87 %139 %137
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpCompositeConstruct %v3float %114 %114 %114
%144 = OpFSub %v3float %126 %143
%145 = OpFSub %float %114 %128
%146 = OpLoad %float %_kGuardedDivideEpsilon
%147 = OpFAdd %float %145 %146
%148 = OpFDiv %float %114 %147
%149 = OpVectorTimesScalar %v3float %144 %148
%150 = OpFAdd %v3float %143 %149
OpStore %_5_result %150
OpBranch %142
%142 = OpLabel
%151 = OpFOrdGreaterThan %bool %134 %57
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpFUnordNotEqual %bool %134 %114
OpBranch %153
%153 = OpLabel
%155 = OpPhi %bool %false %142 %154 %152
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpLoad %v3float %_5_result
%159 = OpCompositeConstruct %v3float %114 %114 %114
%160 = OpFSub %v3float %158 %159
%161 = OpFSub %float %57 %114
%162 = OpVectorTimesScalar %v3float %160 %161
%163 = OpFSub %float %134 %114
%164 = OpLoad %float %_kGuardedDivideEpsilon
%165 = OpFAdd %float %163 %164
%166 = OpFDiv %float %float_1 %165
%167 = OpVectorTimesScalar %v3float %162 %166
%168 = OpFAdd %v3float %159 %167
OpStore %_5_result %168
OpBranch %157
%157 = OpLabel
%169 = OpLoad %v3float %_5_result
%170 = OpLoad %v4float %49
%171 = OpVectorShuffle %v3float %170 %170 0 1 2
%172 = OpFAdd %v3float %169 %171
%173 = OpFSub %v3float %172 %69
%174 = OpLoad %v4float %48
%175 = OpVectorShuffle %v3float %174 %174 0 1 2
%176 = OpFAdd %v3float %173 %175
%177 = OpFSub %v3float %176 %63
%178 = OpCompositeExtract %float %177 0
%179 = OpCompositeExtract %float %177 1
%180 = OpCompositeExtract %float %177 2
%181 = OpLoad %v4float %48
%182 = OpCompositeExtract %float %181 3
%183 = OpLoad %v4float %49
%184 = OpCompositeExtract %float %183 3
%185 = OpFAdd %float %182 %184
%186 = OpFSub %float %185 %57
%187 = OpCompositeConstruct %v4float %178 %179 %180 %186
OpReturnValue %187
OpFunctionEnd
%main = OpFunction %void None %189
%190 = OpLabel
%192 = OpVariable %_ptr_Function_v2float Function
%198 = OpVariable %_ptr_Function_v4float Function
%202 = OpVariable %_ptr_Function_v4float Function
%10 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_kGuardedDivideEpsilon %10
OpStore %192 %191
%193 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%197 = OpLoad %v4float %193
OpStore %198 %197
%199 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%201 = OpLoad %v4float %199
OpStore %202 %201
%203 = OpFunctionCall %v4float %blend_hslc_h4h2h4h4 %192 %198 %202
OpStore %sk_FragColor %203
OpReturn
OpFunctionEnd
