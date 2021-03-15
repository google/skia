OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "h2"
OpMemberName %_UniformBuffer 1 "h3"
OpMemberName %_UniformBuffer 2 "h4"
OpMemberName %_UniformBuffer 3 "f2"
OpMemberName %_UniformBuffer 4 "f3"
OpMemberName %_UniformBuffer 5 "f4"
OpName %main "main"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 5 Offset 80
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%v3float = OpTypeVector %float 3
%_UniformBuffer = OpTypeStruct %v2float %v3float %v4float %v2float %v3float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%int_1 = OpConstant %int 1
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%int_4 = OpConstant %int 4
%int_2 = OpConstant %int 2
%_ptr_Function_v3float = OpTypePointer Function %v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_5 = OpConstant %int 5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %16
%17 = OpLabel
%18 = OpVariable %_ptr_Function_mat2v2float Function
%34 = OpVariable %_ptr_Function_mat3v3float Function
%49 = OpVariable %_ptr_Function_mat4v4float Function
%62 = OpVariable %_ptr_Function_mat2v3float Function
%73 = OpVariable %_ptr_Function_mat3v2float Function
%84 = OpVariable %_ptr_Function_mat2v4float Function
%94 = OpVariable %_ptr_Function_mat4v2float Function
%105 = OpVariable %_ptr_Function_mat3v4float Function
%115 = OpVariable %_ptr_Function_mat4v3float Function
%126 = OpVariable %_ptr_Function_mat2v2float Function
%136 = OpVariable %_ptr_Function_mat3v3float Function
%145 = OpVariable %_ptr_Function_mat4v4float Function
%153 = OpVariable %_ptr_Function_mat2v3float Function
%162 = OpVariable %_ptr_Function_mat3v2float Function
%171 = OpVariable %_ptr_Function_mat2v4float Function
%179 = OpVariable %_ptr_Function_mat4v2float Function
%188 = OpVariable %_ptr_Function_mat3v4float Function
%196 = OpVariable %_ptr_Function_mat4v3float Function
%22 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%26 = OpLoad %v2float %22
%27 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%28 = OpLoad %v2float %27
%21 = OpOuterProduct %mat2v2float %26 %28
OpStore %18 %21
%30 = OpAccessChain %_ptr_Function_v2float %18 %int_1
%32 = OpLoad %v2float %30
%33 = OpVectorShuffle %v4float %32 %32 0 1 1 1
OpStore %sk_FragColor %33
%38 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%41 = OpLoad %v3float %38
%42 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%43 = OpLoad %v3float %42
%37 = OpOuterProduct %mat3v3float %41 %43
OpStore %34 %37
%45 = OpAccessChain %_ptr_Function_v3float %34 %int_2
%47 = OpLoad %v3float %45
%48 = OpVectorShuffle %v4float %47 %47 0 1 2 2
OpStore %sk_FragColor %48
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%56 = OpLoad %v4float %53
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%58 = OpLoad %v4float %57
%52 = OpOuterProduct %mat4v4float %56 %58
OpStore %49 %52
%59 = OpAccessChain %_ptr_Function_v4float %49 %int_3
%61 = OpLoad %v4float %59
OpStore %sk_FragColor %61
%66 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%67 = OpLoad %v3float %66
%68 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%69 = OpLoad %v2float %68
%65 = OpOuterProduct %mat2v3float %67 %69
OpStore %62 %65
%70 = OpAccessChain %_ptr_Function_v3float %62 %int_1
%71 = OpLoad %v3float %70
%72 = OpVectorShuffle %v4float %71 %71 0 1 2 2
OpStore %sk_FragColor %72
%77 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%78 = OpLoad %v2float %77
%79 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%80 = OpLoad %v3float %79
%76 = OpOuterProduct %mat3v2float %78 %80
OpStore %73 %76
%81 = OpAccessChain %_ptr_Function_v2float %73 %int_2
%82 = OpLoad %v2float %81
%83 = OpVectorShuffle %v4float %82 %82 0 1 1 1
OpStore %sk_FragColor %83
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%89 = OpLoad %v4float %88
%90 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%91 = OpLoad %v2float %90
%87 = OpOuterProduct %mat2v4float %89 %91
OpStore %84 %87
%92 = OpAccessChain %_ptr_Function_v4float %84 %int_1
%93 = OpLoad %v4float %92
OpStore %sk_FragColor %93
%98 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%99 = OpLoad %v2float %98
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%101 = OpLoad %v4float %100
%97 = OpOuterProduct %mat4v2float %99 %101
OpStore %94 %97
%102 = OpAccessChain %_ptr_Function_v2float %94 %int_3
%103 = OpLoad %v2float %102
%104 = OpVectorShuffle %v4float %103 %103 0 1 1 1
OpStore %sk_FragColor %104
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%110 = OpLoad %v4float %109
%111 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%112 = OpLoad %v3float %111
%108 = OpOuterProduct %mat3v4float %110 %112
OpStore %105 %108
%113 = OpAccessChain %_ptr_Function_v4float %105 %int_2
%114 = OpLoad %v4float %113
OpStore %sk_FragColor %114
%119 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%120 = OpLoad %v3float %119
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%122 = OpLoad %v4float %121
%118 = OpOuterProduct %mat4v3float %120 %122
OpStore %115 %118
%123 = OpAccessChain %_ptr_Function_v3float %115 %int_3
%124 = OpLoad %v3float %123
%125 = OpVectorShuffle %v4float %124 %124 0 1 2 2
OpStore %sk_FragColor %125
%128 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%130 = OpLoad %v2float %128
%131 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%132 = OpLoad %v2float %131
%127 = OpOuterProduct %mat2v2float %130 %132
OpStore %126 %127
%133 = OpAccessChain %_ptr_Function_v2float %126 %int_1
%134 = OpLoad %v2float %133
%135 = OpVectorShuffle %v4float %134 %134 0 1 1 1
OpStore %sk_FragColor %135
%138 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%139 = OpLoad %v3float %138
%140 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%141 = OpLoad %v3float %140
%137 = OpOuterProduct %mat3v3float %139 %141
OpStore %136 %137
%142 = OpAccessChain %_ptr_Function_v3float %136 %int_2
%143 = OpLoad %v3float %142
%144 = OpVectorShuffle %v4float %143 %143 0 1 2 2
OpStore %sk_FragColor %144
%147 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%148 = OpLoad %v4float %147
%149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%150 = OpLoad %v4float %149
%146 = OpOuterProduct %mat4v4float %148 %150
OpStore %145 %146
%151 = OpAccessChain %_ptr_Function_v4float %145 %int_3
%152 = OpLoad %v4float %151
OpStore %sk_FragColor %152
%155 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%156 = OpLoad %v3float %155
%157 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%158 = OpLoad %v2float %157
%154 = OpOuterProduct %mat2v3float %156 %158
OpStore %153 %154
%159 = OpAccessChain %_ptr_Function_v3float %153 %int_1
%160 = OpLoad %v3float %159
%161 = OpVectorShuffle %v4float %160 %160 0 1 2 2
OpStore %sk_FragColor %161
%164 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%165 = OpLoad %v2float %164
%166 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%167 = OpLoad %v3float %166
%163 = OpOuterProduct %mat3v2float %165 %167
OpStore %162 %163
%168 = OpAccessChain %_ptr_Function_v2float %162 %int_2
%169 = OpLoad %v2float %168
%170 = OpVectorShuffle %v4float %169 %169 0 1 1 1
OpStore %sk_FragColor %170
%173 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%174 = OpLoad %v4float %173
%175 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%176 = OpLoad %v2float %175
%172 = OpOuterProduct %mat2v4float %174 %176
OpStore %171 %172
%177 = OpAccessChain %_ptr_Function_v4float %171 %int_1
%178 = OpLoad %v4float %177
OpStore %sk_FragColor %178
%181 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%182 = OpLoad %v2float %181
%183 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%184 = OpLoad %v4float %183
%180 = OpOuterProduct %mat4v2float %182 %184
OpStore %179 %180
%185 = OpAccessChain %_ptr_Function_v2float %179 %int_3
%186 = OpLoad %v2float %185
%187 = OpVectorShuffle %v4float %186 %186 0 1 1 1
OpStore %sk_FragColor %187
%190 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%191 = OpLoad %v4float %190
%192 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%193 = OpLoad %v3float %192
%189 = OpOuterProduct %mat3v4float %191 %193
OpStore %188 %189
%194 = OpAccessChain %_ptr_Function_v4float %188 %int_2
%195 = OpLoad %v4float %194
OpStore %sk_FragColor %195
%198 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%199 = OpLoad %v3float %198
%200 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%201 = OpLoad %v4float %200
%197 = OpOuterProduct %mat4v3float %199 %201
OpStore %196 %197
%202 = OpAccessChain %_ptr_Function_v3float %196 %int_3
%203 = OpLoad %v3float %202
%204 = OpVectorShuffle %v4float %203 %203 0 1 2 2
OpStore %sk_FragColor %204
OpReturn
OpFunctionEnd
