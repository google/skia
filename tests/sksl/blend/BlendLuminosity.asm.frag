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
OpDecorate %37 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
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
OpDecorate %82 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
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
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %_0_alpha RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %_1_sda RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %_2_dsa RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
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
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%112 = OpTypeFunction %void
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
%27 = OpCompositeConstruct %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%28 = OpLoad %v3float %20
%23 = OpDot %float %27 %28
OpStore %lum %23
%30 = OpLoad %float %lum
%32 = OpCompositeConstruct %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%33 = OpLoad %v3float %18
%31 = OpDot %float %32 %33
%34 = OpFSub %float %30 %31
%35 = OpLoad %v3float %18
%36 = OpCompositeConstruct %v3float %34 %34 %34
%37 = OpFAdd %v3float %36 %35
OpStore %result %37
%41 = OpLoad %v3float %result
%42 = OpCompositeExtract %float %41 0
%43 = OpLoad %v3float %result
%44 = OpCompositeExtract %float %43 1
%40 = OpExtInst %float %1 FMin %42 %44
%45 = OpLoad %v3float %result
%46 = OpCompositeExtract %float %45 2
%39 = OpExtInst %float %1 FMin %40 %46
OpStore %minComp %39
%50 = OpLoad %v3float %result
%51 = OpCompositeExtract %float %50 0
%52 = OpLoad %v3float %result
%53 = OpCompositeExtract %float %52 1
%49 = OpExtInst %float %1 FMax %51 %53
%54 = OpLoad %v3float %result
%55 = OpCompositeExtract %float %54 2
%48 = OpExtInst %float %1 FMax %49 %55
OpStore %maxComp %48
%57 = OpLoad %float %minComp
%59 = OpFOrdLessThan %bool %57 %float_0
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %float %lum
%63 = OpLoad %float %minComp
%64 = OpFOrdNotEqual %bool %62 %63
OpBranch %61
%61 = OpLabel
%65 = OpPhi %bool %false %21 %64 %60
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpLoad %float %lum
%69 = OpLoad %v3float %result
%70 = OpLoad %float %lum
%71 = OpCompositeConstruct %v3float %70 %70 %70
%72 = OpFSub %v3float %69 %71
%73 = OpLoad %float %lum
%74 = OpLoad %float %lum
%75 = OpLoad %float %minComp
%76 = OpFSub %float %74 %75
%77 = OpFDiv %float %73 %76
%78 = OpVectorTimesScalar %v3float %72 %77
%79 = OpCompositeConstruct %v3float %68 %68 %68
%80 = OpFAdd %v3float %79 %78
OpStore %result %80
OpBranch %67
%67 = OpLabel
%81 = OpLoad %float %maxComp
%82 = OpLoad %float %19
%83 = OpFOrdGreaterThan %bool %81 %82
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %float %maxComp
%87 = OpLoad %float %lum
%88 = OpFOrdNotEqual %bool %86 %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %67 %88 %84
OpSelectionMerge %92 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpLoad %float %lum
%94 = OpLoad %v3float %result
%95 = OpLoad %float %lum
%96 = OpCompositeConstruct %v3float %95 %95 %95
%97 = OpFSub %v3float %94 %96
%98 = OpLoad %float %19
%99 = OpLoad %float %lum
%100 = OpFSub %float %98 %99
%101 = OpVectorTimesScalar %v3float %97 %100
%102 = OpLoad %float %maxComp
%103 = OpLoad %float %lum
%104 = OpFSub %float %102 %103
%106 = OpFDiv %float %float_1 %104
%107 = OpVectorTimesScalar %v3float %101 %106
%108 = OpCompositeConstruct %v3float %93 %93 %93
%109 = OpFAdd %v3float %108 %107
OpReturnValue %109
%91 = OpLabel
%110 = OpLoad %v3float %result
OpReturnValue %110
%92 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %112
%113 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%143 = OpVariable %_ptr_Function_v3float Function
%145 = OpVariable %_ptr_Function_float Function
%147 = OpVariable %_ptr_Function_v3float Function
%115 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%119 = OpLoad %v4float %115
%120 = OpCompositeExtract %float %119 3
%121 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%123 = OpLoad %v4float %121
%124 = OpCompositeExtract %float %123 3
%125 = OpFMul %float %120 %124
OpStore %_0_alpha %125
%127 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%128 = OpLoad %v4float %127
%129 = OpVectorShuffle %v3float %128 %128 0 1 2
%130 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 3
%133 = OpVectorTimesScalar %v3float %129 %132
OpStore %_1_sda %133
%135 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%136 = OpLoad %v4float %135
%137 = OpVectorShuffle %v3float %136 %136 0 1 2
%138 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 3
%141 = OpVectorTimesScalar %v3float %137 %140
OpStore %_2_dsa %141
%142 = OpLoad %v3float %_2_dsa
OpStore %143 %142
%144 = OpLoad %float %_0_alpha
OpStore %145 %144
%146 = OpLoad %v3float %_1_sda
OpStore %147 %146
%148 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %143 %145 %147
%149 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%150 = OpLoad %v4float %149
%151 = OpVectorShuffle %v3float %150 %150 0 1 2
%152 = OpFAdd %v3float %148 %151
%153 = OpLoad %v3float %_2_dsa
%154 = OpFSub %v3float %152 %153
%155 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%156 = OpLoad %v4float %155
%157 = OpVectorShuffle %v3float %156 %156 0 1 2
%158 = OpFAdd %v3float %154 %157
%159 = OpLoad %v3float %_1_sda
%160 = OpFSub %v3float %158 %159
%161 = OpCompositeExtract %float %160 0
%162 = OpCompositeExtract %float %160 1
%163 = OpCompositeExtract %float %160 2
%164 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%165 = OpLoad %v4float %164
%166 = OpCompositeExtract %float %165 3
%167 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%168 = OpLoad %v4float %167
%169 = OpCompositeExtract %float %168 3
%170 = OpFAdd %float %166 %169
%171 = OpLoad %float %_0_alpha
%172 = OpFSub %float %170 %171
%173 = OpCompositeConstruct %v4float %161 %162 %163 %172
OpStore %sk_FragColor %173
OpReturn
OpFunctionEnd
