OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %_blend_set_color_luminance_h3h3hh3 "_blend_set_color_luminance_h3h3hh3"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %main "main"
OpName %_0_alpha "_0_alpha"
OpName %_1_sda "_1_sda"
OpName %_2_dsa "_2_dsa"
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
OpDecorate %lum RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
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
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
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
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %_0_alpha RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %_1_sda RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %_2_dsa RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_float = OpTypePointer Function %float
%15 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%27 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%111 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_blend_set_color_luminance_h3h3hh3 = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpFunctionParameter %_ptr_Function_v3float
%21 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%28 = OpLoad %v3float %20
%23 = OpDot %float %27 %28
OpStore %lum %23
%30 = OpLoad %float %lum
%32 = OpLoad %v3float %18
%31 = OpDot %float %27 %32
%33 = OpFSub %float %30 %31
%34 = OpLoad %v3float %18
%35 = OpCompositeConstruct %v3float %33 %33 %33
%36 = OpFAdd %v3float %35 %34
OpStore %result %36
%40 = OpLoad %v3float %result
%41 = OpCompositeExtract %float %40 0
%42 = OpLoad %v3float %result
%43 = OpCompositeExtract %float %42 1
%39 = OpExtInst %float %1 FMin %41 %43
%44 = OpLoad %v3float %result
%45 = OpCompositeExtract %float %44 2
%38 = OpExtInst %float %1 FMin %39 %45
OpStore %minComp %38
%49 = OpLoad %v3float %result
%50 = OpCompositeExtract %float %49 0
%51 = OpLoad %v3float %result
%52 = OpCompositeExtract %float %51 1
%48 = OpExtInst %float %1 FMax %50 %52
%53 = OpLoad %v3float %result
%54 = OpCompositeExtract %float %53 2
%47 = OpExtInst %float %1 FMax %48 %54
OpStore %maxComp %47
%56 = OpLoad %float %minComp
%58 = OpFOrdLessThan %bool %56 %float_0
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %float %lum
%62 = OpLoad %float %minComp
%63 = OpFOrdNotEqual %bool %61 %62
OpBranch %60
%60 = OpLabel
%64 = OpPhi %bool %false %21 %63 %59
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpLoad %float %lum
%68 = OpLoad %v3float %result
%69 = OpLoad %float %lum
%70 = OpCompositeConstruct %v3float %69 %69 %69
%71 = OpFSub %v3float %68 %70
%72 = OpLoad %float %lum
%73 = OpLoad %float %lum
%74 = OpLoad %float %minComp
%75 = OpFSub %float %73 %74
%76 = OpFDiv %float %72 %75
%77 = OpVectorTimesScalar %v3float %71 %76
%78 = OpCompositeConstruct %v3float %67 %67 %67
%79 = OpFAdd %v3float %78 %77
OpStore %result %79
OpBranch %66
%66 = OpLabel
%80 = OpLoad %float %maxComp
%81 = OpLoad %float %19
%82 = OpFOrdGreaterThan %bool %80 %81
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %float %maxComp
%86 = OpLoad %float %lum
%87 = OpFOrdNotEqual %bool %85 %86
OpBranch %84
%84 = OpLabel
%88 = OpPhi %bool %false %66 %87 %83
OpSelectionMerge %91 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpLoad %float %lum
%93 = OpLoad %v3float %result
%94 = OpLoad %float %lum
%95 = OpCompositeConstruct %v3float %94 %94 %94
%96 = OpFSub %v3float %93 %95
%97 = OpLoad %float %19
%98 = OpLoad %float %lum
%99 = OpFSub %float %97 %98
%100 = OpVectorTimesScalar %v3float %96 %99
%101 = OpLoad %float %maxComp
%102 = OpLoad %float %lum
%103 = OpFSub %float %101 %102
%105 = OpFDiv %float %float_1 %103
%106 = OpVectorTimesScalar %v3float %100 %105
%107 = OpCompositeConstruct %v3float %92 %92 %92
%108 = OpFAdd %v3float %107 %106
OpReturnValue %108
%90 = OpLabel
%109 = OpLoad %v3float %result
OpReturnValue %109
%91 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %111
%112 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%142 = OpVariable %_ptr_Function_v3float Function
%144 = OpVariable %_ptr_Function_float Function
%146 = OpVariable %_ptr_Function_v3float Function
%114 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%118 = OpLoad %v4float %114
%119 = OpCompositeExtract %float %118 3
%120 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%122 = OpLoad %v4float %120
%123 = OpCompositeExtract %float %122 3
%124 = OpFMul %float %119 %123
OpStore %_0_alpha %124
%126 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%127 = OpLoad %v4float %126
%128 = OpVectorShuffle %v3float %127 %127 0 1 2
%129 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%130 = OpLoad %v4float %129
%131 = OpCompositeExtract %float %130 3
%132 = OpVectorTimesScalar %v3float %128 %131
OpStore %_1_sda %132
%134 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%135 = OpLoad %v4float %134
%136 = OpVectorShuffle %v3float %135 %135 0 1 2
%137 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 3
%140 = OpVectorTimesScalar %v3float %136 %139
OpStore %_2_dsa %140
%141 = OpLoad %v3float %_2_dsa
OpStore %142 %141
%143 = OpLoad %float %_0_alpha
OpStore %144 %143
%145 = OpLoad %v3float %_1_sda
OpStore %146 %145
%147 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %142 %144 %146
%148 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%149 = OpLoad %v4float %148
%150 = OpVectorShuffle %v3float %149 %149 0 1 2
%151 = OpFAdd %v3float %147 %150
%152 = OpLoad %v3float %_2_dsa
%153 = OpFSub %v3float %151 %152
%154 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%155 = OpLoad %v4float %154
%156 = OpVectorShuffle %v3float %155 %155 0 1 2
%157 = OpFAdd %v3float %153 %156
%158 = OpLoad %v3float %_1_sda
%159 = OpFSub %v3float %157 %158
%160 = OpCompositeExtract %float %159 0
%161 = OpCompositeExtract %float %159 1
%162 = OpCompositeExtract %float %159 2
%163 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%164 = OpLoad %v4float %163
%165 = OpCompositeExtract %float %164 3
%166 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%167 = OpLoad %v4float %166
%168 = OpCompositeExtract %float %167 3
%169 = OpFAdd %float %165 %168
%170 = OpLoad %float %_0_alpha
%171 = OpFSub %float %169 %170
%172 = OpCompositeConstruct %v4float %160 %161 %162 %171
OpStore %sk_FragColor %172
OpReturn
OpFunctionEnd
