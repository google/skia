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
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_4_d "_4_d"
OpName %_5_n "_5_n"
OpName %_6_d "_6_d"
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
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
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
%117 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_blend_set_color_luminance = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpFunctionParameter %_ptr_Function_v3float
%21 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%_4_d = OpVariable %_ptr_Function_float Function
%_5_n = OpVariable %_ptr_Function_v3float Function
%_6_d = OpVariable %_ptr_Function_float Function
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
%68 = OpLoad %float %lum
%69 = OpLoad %float %minComp
%70 = OpFSub %float %68 %69
OpStore %_4_d %70
%71 = OpLoad %float %lum
%72 = OpLoad %v3float %result
%73 = OpLoad %float %lum
%74 = OpCompositeConstruct %v3float %73 %73 %73
%75 = OpFSub %v3float %72 %74
%76 = OpLoad %float %lum
%77 = OpLoad %float %_4_d
%78 = OpFDiv %float %76 %77
%79 = OpVectorTimesScalar %v3float %75 %78
%80 = OpCompositeConstruct %v3float %71 %71 %71
%81 = OpFAdd %v3float %80 %79
OpStore %result %81
OpBranch %66
%66 = OpLabel
%82 = OpLoad %float %maxComp
%83 = OpLoad %float %19
%84 = OpFOrdGreaterThan %bool %82 %83
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %float %maxComp
%88 = OpLoad %float %lum
%89 = OpFOrdNotEqual %bool %87 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %66 %89 %85
OpSelectionMerge %93 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%95 = OpLoad %v3float %result
%96 = OpLoad %float %lum
%97 = OpCompositeConstruct %v3float %96 %96 %96
%98 = OpFSub %v3float %95 %97
%99 = OpLoad %float %19
%100 = OpLoad %float %lum
%101 = OpFSub %float %99 %100
%102 = OpVectorTimesScalar %v3float %98 %101
OpStore %_5_n %102
%104 = OpLoad %float %maxComp
%105 = OpLoad %float %lum
%106 = OpFSub %float %104 %105
OpStore %_6_d %106
%107 = OpLoad %float %lum
%108 = OpLoad %v3float %_5_n
%109 = OpLoad %float %_6_d
%111 = OpFDiv %float %float_1 %109
%112 = OpVectorTimesScalar %v3float %108 %111
%113 = OpCompositeConstruct %v3float %107 %107 %107
%114 = OpFAdd %v3float %113 %112
OpReturnValue %114
%92 = OpLabel
%115 = OpLoad %v3float %result
OpReturnValue %115
%93 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %117
%118 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%148 = OpVariable %_ptr_Function_v3float Function
%150 = OpVariable %_ptr_Function_float Function
%152 = OpVariable %_ptr_Function_v3float Function
%120 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%124 = OpLoad %v4float %120
%125 = OpCompositeExtract %float %124 3
%126 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%128 = OpLoad %v4float %126
%129 = OpCompositeExtract %float %128 3
%130 = OpFMul %float %125 %129
OpStore %_0_alpha %130
%132 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%133 = OpLoad %v4float %132
%134 = OpVectorShuffle %v3float %133 %133 0 1 2
%135 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%136 = OpLoad %v4float %135
%137 = OpCompositeExtract %float %136 3
%138 = OpVectorTimesScalar %v3float %134 %137
OpStore %_1_sda %138
%140 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%141 = OpLoad %v4float %140
%142 = OpVectorShuffle %v3float %141 %141 0 1 2
%143 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%144 = OpLoad %v4float %143
%145 = OpCompositeExtract %float %144 3
%146 = OpVectorTimesScalar %v3float %142 %145
OpStore %_2_dsa %146
%147 = OpLoad %v3float %_1_sda
OpStore %148 %147
%149 = OpLoad %float %_0_alpha
OpStore %150 %149
%151 = OpLoad %v3float %_2_dsa
OpStore %152 %151
%153 = OpFunctionCall %v3float %_blend_set_color_luminance %148 %150 %152
%154 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%155 = OpLoad %v4float %154
%156 = OpVectorShuffle %v3float %155 %155 0 1 2
%157 = OpFAdd %v3float %153 %156
%158 = OpLoad %v3float %_2_dsa
%159 = OpFSub %v3float %157 %158
%160 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%161 = OpLoad %v4float %160
%162 = OpVectorShuffle %v3float %161 %161 0 1 2
%163 = OpFAdd %v3float %159 %162
%164 = OpLoad %v3float %_1_sda
%165 = OpFSub %v3float %163 %164
%166 = OpCompositeExtract %float %165 0
%167 = OpCompositeExtract %float %165 1
%168 = OpCompositeExtract %float %165 2
%169 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%170 = OpLoad %v4float %169
%171 = OpCompositeExtract %float %170 3
%172 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%173 = OpLoad %v4float %172
%174 = OpCompositeExtract %float %173 3
%175 = OpFAdd %float %171 %174
%176 = OpLoad %float %_0_alpha
%177 = OpFSub %float %175 %176
%178 = OpCompositeConstruct %v4float %166 %167 %168 %177
OpStore %sk_FragColor %178
OpReturn
OpFunctionEnd
