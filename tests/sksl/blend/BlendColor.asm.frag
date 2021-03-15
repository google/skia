### Compilation failed:

error: SPIR-V validation error: Variable must be decorated with a location
  %src = OpVariable %_ptr_Input_v4float Input

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_guarded_divide "_guarded_divide"
OpName %_guarded_divide_0 "_guarded_divide"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %blend_color "blend_color"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_float = OpTypePointer Function %float
%17 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%26 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_1 = OpConstant %float 1
%36 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%46 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%131 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%183 = OpTypeFunction %void
%_guarded_divide = OpFunction %float None %17
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpFunctionParameter %_ptr_Function_float
%21 = OpLabel
%22 = OpLoad %float %19
%23 = OpLoad %float %20
%24 = OpFDiv %float %22 %23
OpReturnValue %24
OpFunctionEnd
%_guarded_divide_0 = OpFunction %v3float None %26
%28 = OpFunctionParameter %_ptr_Function_v3float
%29 = OpFunctionParameter %_ptr_Function_float
%30 = OpLabel
%31 = OpLoad %v3float %28
%32 = OpLoad %float %29
%34 = OpFDiv %float %float_1 %32
%35 = OpVectorTimesScalar %v3float %31 %34
OpReturnValue %35
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %36
%37 = OpFunctionParameter %_ptr_Function_v3float
%38 = OpFunctionParameter %_ptr_Function_float
%39 = OpFunctionParameter %_ptr_Function_v3float
%40 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%92 = OpVariable %_ptr_Function_float Function
%96 = OpVariable %_ptr_Function_float Function
%122 = OpVariable %_ptr_Function_v3float Function
%126 = OpVariable %_ptr_Function_float Function
%47 = OpLoad %v3float %39
%42 = OpDot %float %46 %47
OpStore %lum %42
%49 = OpLoad %float %lum
%51 = OpLoad %v3float %37
%50 = OpDot %float %46 %51
%52 = OpFSub %float %49 %50
%53 = OpLoad %v3float %37
%54 = OpCompositeConstruct %v3float %52 %52 %52
%55 = OpFAdd %v3float %54 %53
OpStore %result %55
%59 = OpLoad %v3float %result
%60 = OpCompositeExtract %float %59 0
%61 = OpLoad %v3float %result
%62 = OpCompositeExtract %float %61 1
%58 = OpExtInst %float %1 FMin %60 %62
%63 = OpLoad %v3float %result
%64 = OpCompositeExtract %float %63 2
%57 = OpExtInst %float %1 FMin %58 %64
OpStore %minComp %57
%68 = OpLoad %v3float %result
%69 = OpCompositeExtract %float %68 0
%70 = OpLoad %v3float %result
%71 = OpCompositeExtract %float %70 1
%67 = OpExtInst %float %1 FMax %69 %71
%72 = OpLoad %v3float %result
%73 = OpCompositeExtract %float %72 2
%66 = OpExtInst %float %1 FMax %67 %73
OpStore %maxComp %66
%75 = OpLoad %float %minComp
%77 = OpFOrdLessThan %bool %75 %float_0
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %float %lum
%81 = OpLoad %float %minComp
%82 = OpFOrdNotEqual %bool %80 %81
OpBranch %79
%79 = OpLabel
%83 = OpPhi %bool %false %40 %82 %78
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %float %lum
%87 = OpLoad %v3float %result
%88 = OpLoad %float %lum
%89 = OpCompositeConstruct %v3float %88 %88 %88
%90 = OpFSub %v3float %87 %89
%91 = OpLoad %float %lum
OpStore %92 %91
%93 = OpLoad %float %lum
%94 = OpLoad %float %minComp
%95 = OpFSub %float %93 %94
OpStore %96 %95
%97 = OpFunctionCall %float %_guarded_divide %92 %96
%98 = OpVectorTimesScalar %v3float %90 %97
%99 = OpCompositeConstruct %v3float %86 %86 %86
%100 = OpFAdd %v3float %99 %98
OpStore %result %100
OpBranch %85
%85 = OpLabel
%101 = OpLoad %float %maxComp
%102 = OpLoad %float %38
%103 = OpFOrdGreaterThan %bool %101 %102
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %float %maxComp
%107 = OpLoad %float %lum
%108 = OpFOrdNotEqual %bool %106 %107
OpBranch %105
%105 = OpLabel
%109 = OpPhi %bool %false %85 %108 %104
OpSelectionMerge %112 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpLoad %float %lum
%114 = OpLoad %v3float %result
%115 = OpLoad %float %lum
%116 = OpCompositeConstruct %v3float %115 %115 %115
%117 = OpFSub %v3float %114 %116
%118 = OpLoad %float %38
%119 = OpLoad %float %lum
%120 = OpFSub %float %118 %119
%121 = OpVectorTimesScalar %v3float %117 %120
OpStore %122 %121
%123 = OpLoad %float %maxComp
%124 = OpLoad %float %lum
%125 = OpFSub %float %123 %124
OpStore %126 %125
%127 = OpFunctionCall %v3float %_guarded_divide_0 %122 %126
%128 = OpCompositeConstruct %v3float %113 %113 %113
%129 = OpFAdd %v3float %128 %127
OpReturnValue %129
%111 = OpLabel
%130 = OpLoad %v3float %result
OpReturnValue %130
%112 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color = OpFunction %v4float None %131
%133 = OpFunctionParameter %_ptr_Function_v4float
%134 = OpFunctionParameter %_ptr_Function_v4float
%135 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%155 = OpVariable %_ptr_Function_v3float Function
%157 = OpVariable %_ptr_Function_float Function
%159 = OpVariable %_ptr_Function_v3float Function
%137 = OpLoad %v4float %134
%138 = OpCompositeExtract %float %137 3
%139 = OpLoad %v4float %133
%140 = OpCompositeExtract %float %139 3
%141 = OpFMul %float %138 %140
OpStore %alpha %141
%143 = OpLoad %v4float %133
%144 = OpVectorShuffle %v3float %143 %143 0 1 2
%145 = OpLoad %v4float %134
%146 = OpCompositeExtract %float %145 3
%147 = OpVectorTimesScalar %v3float %144 %146
OpStore %sda %147
%149 = OpLoad %v4float %134
%150 = OpVectorShuffle %v3float %149 %149 0 1 2
%151 = OpLoad %v4float %133
%152 = OpCompositeExtract %float %151 3
%153 = OpVectorTimesScalar %v3float %150 %152
OpStore %dsa %153
%154 = OpLoad %v3float %sda
OpStore %155 %154
%156 = OpLoad %float %alpha
OpStore %157 %156
%158 = OpLoad %v3float %dsa
OpStore %159 %158
%160 = OpFunctionCall %v3float %_blend_set_color_luminance %155 %157 %159
%161 = OpLoad %v4float %134
%162 = OpVectorShuffle %v3float %161 %161 0 1 2
%163 = OpFAdd %v3float %160 %162
%164 = OpLoad %v3float %dsa
%165 = OpFSub %v3float %163 %164
%166 = OpLoad %v4float %133
%167 = OpVectorShuffle %v3float %166 %166 0 1 2
%168 = OpFAdd %v3float %165 %167
%169 = OpLoad %v3float %sda
%170 = OpFSub %v3float %168 %169
%171 = OpCompositeExtract %float %170 0
%172 = OpCompositeExtract %float %170 1
%173 = OpCompositeExtract %float %170 2
%174 = OpLoad %v4float %133
%175 = OpCompositeExtract %float %174 3
%176 = OpLoad %v4float %134
%177 = OpCompositeExtract %float %176 3
%178 = OpFAdd %float %175 %177
%179 = OpLoad %float %alpha
%180 = OpFSub %float %178 %179
%181 = OpCompositeConstruct %v4float %171 %172 %173 %180
OpReturnValue %181
OpFunctionEnd
%main = OpFunction %void None %183
%184 = OpLabel
%186 = OpVariable %_ptr_Function_v4float Function
%188 = OpVariable %_ptr_Function_v4float Function
%185 = OpLoad %v4float %src
OpStore %186 %185
%187 = OpLoad %v4float %dst
OpStore %188 %187
%189 = OpFunctionCall %v4float %blend_color %186 %188
OpStore %sk_FragColor %189
OpReturn
OpFunctionEnd

1 error
