OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %h2 %h3 %h4 %f2 %f3 %f4
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %h2 "h2"
OpName %h3 "h3"
OpName %h4 "h4"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %h2 RelaxedPrecision
OpDecorate %h3 RelaxedPrecision
OpDecorate %h4 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%h2 = OpVariable %_ptr_Input_v2float Input
%v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
%h3 = OpVariable %_ptr_Input_v3float Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%h4 = OpVariable %_ptr_Input_v4float Input
%f2 = OpVariable %_ptr_Input_v2float Input
%f3 = OpVariable %_ptr_Input_v3float Input
%f4 = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%22 = OpTypeFunction %void
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%int_2 = OpConstant %int 2
%_ptr_Function_v3float = OpTypePointer Function %v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%int_3 = OpConstant %int 3
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
%main = OpFunction %void None %22
%23 = OpLabel
%24 = OpVariable %_ptr_Function_mat2v2float Function
%36 = OpVariable %_ptr_Function_mat3v3float Function
%47 = OpVariable %_ptr_Function_mat4v4float Function
%57 = OpVariable %_ptr_Function_mat2v3float Function
%66 = OpVariable %_ptr_Function_mat3v2float Function
%75 = OpVariable %_ptr_Function_mat2v4float Function
%83 = OpVariable %_ptr_Function_mat4v2float Function
%92 = OpVariable %_ptr_Function_mat3v4float Function
%100 = OpVariable %_ptr_Function_mat4v3float Function
%109 = OpVariable %_ptr_Function_mat2v2float Function
%116 = OpVariable %_ptr_Function_mat3v3float Function
%123 = OpVariable %_ptr_Function_mat4v4float Function
%129 = OpVariable %_ptr_Function_mat2v3float Function
%136 = OpVariable %_ptr_Function_mat3v2float Function
%143 = OpVariable %_ptr_Function_mat2v4float Function
%149 = OpVariable %_ptr_Function_mat4v2float Function
%156 = OpVariable %_ptr_Function_mat3v4float Function
%162 = OpVariable %_ptr_Function_mat4v3float Function
%28 = OpLoad %v2float %f2
%29 = OpLoad %v2float %f2
%27 = OpOuterProduct %mat2v2float %28 %29
OpStore %24 %27
%32 = OpAccessChain %_ptr_Function_v2float %24 %int_1
%34 = OpLoad %v2float %32
%35 = OpVectorShuffle %v4float %34 %34 0 1 1 1
OpStore %sk_FragColor %35
%40 = OpLoad %v3float %f3
%41 = OpLoad %v3float %f3
%39 = OpOuterProduct %mat3v3float %40 %41
OpStore %36 %39
%43 = OpAccessChain %_ptr_Function_v3float %36 %int_2
%45 = OpLoad %v3float %43
%46 = OpVectorShuffle %v4float %45 %45 0 1 2 2
OpStore %sk_FragColor %46
%51 = OpLoad %v4float %f4
%52 = OpLoad %v4float %f4
%50 = OpOuterProduct %mat4v4float %51 %52
OpStore %47 %50
%54 = OpAccessChain %_ptr_Function_v4float %47 %int_3
%56 = OpLoad %v4float %54
OpStore %sk_FragColor %56
%61 = OpLoad %v3float %f3
%62 = OpLoad %v2float %f2
%60 = OpOuterProduct %mat2v3float %61 %62
OpStore %57 %60
%63 = OpAccessChain %_ptr_Function_v3float %57 %int_1
%64 = OpLoad %v3float %63
%65 = OpVectorShuffle %v4float %64 %64 0 1 2 2
OpStore %sk_FragColor %65
%70 = OpLoad %v2float %f2
%71 = OpLoad %v3float %f3
%69 = OpOuterProduct %mat3v2float %70 %71
OpStore %66 %69
%72 = OpAccessChain %_ptr_Function_v2float %66 %int_2
%73 = OpLoad %v2float %72
%74 = OpVectorShuffle %v4float %73 %73 0 1 1 1
OpStore %sk_FragColor %74
%79 = OpLoad %v4float %f4
%80 = OpLoad %v2float %f2
%78 = OpOuterProduct %mat2v4float %79 %80
OpStore %75 %78
%81 = OpAccessChain %_ptr_Function_v4float %75 %int_1
%82 = OpLoad %v4float %81
OpStore %sk_FragColor %82
%87 = OpLoad %v2float %f2
%88 = OpLoad %v4float %f4
%86 = OpOuterProduct %mat4v2float %87 %88
OpStore %83 %86
%89 = OpAccessChain %_ptr_Function_v2float %83 %int_3
%90 = OpLoad %v2float %89
%91 = OpVectorShuffle %v4float %90 %90 0 1 1 1
OpStore %sk_FragColor %91
%96 = OpLoad %v4float %f4
%97 = OpLoad %v3float %f3
%95 = OpOuterProduct %mat3v4float %96 %97
OpStore %92 %95
%98 = OpAccessChain %_ptr_Function_v4float %92 %int_2
%99 = OpLoad %v4float %98
OpStore %sk_FragColor %99
%104 = OpLoad %v3float %f3
%105 = OpLoad %v4float %f4
%103 = OpOuterProduct %mat4v3float %104 %105
OpStore %100 %103
%106 = OpAccessChain %_ptr_Function_v3float %100 %int_3
%107 = OpLoad %v3float %106
%108 = OpVectorShuffle %v4float %107 %107 0 1 2 2
OpStore %sk_FragColor %108
%111 = OpLoad %v2float %h2
%112 = OpLoad %v2float %h2
%110 = OpOuterProduct %mat2v2float %111 %112
OpStore %109 %110
%113 = OpAccessChain %_ptr_Function_v2float %109 %int_1
%114 = OpLoad %v2float %113
%115 = OpVectorShuffle %v4float %114 %114 0 1 1 1
OpStore %sk_FragColor %115
%118 = OpLoad %v3float %h3
%119 = OpLoad %v3float %h3
%117 = OpOuterProduct %mat3v3float %118 %119
OpStore %116 %117
%120 = OpAccessChain %_ptr_Function_v3float %116 %int_2
%121 = OpLoad %v3float %120
%122 = OpVectorShuffle %v4float %121 %121 0 1 2 2
OpStore %sk_FragColor %122
%125 = OpLoad %v4float %h4
%126 = OpLoad %v4float %h4
%124 = OpOuterProduct %mat4v4float %125 %126
OpStore %123 %124
%127 = OpAccessChain %_ptr_Function_v4float %123 %int_3
%128 = OpLoad %v4float %127
OpStore %sk_FragColor %128
%131 = OpLoad %v3float %h3
%132 = OpLoad %v2float %h2
%130 = OpOuterProduct %mat2v3float %131 %132
OpStore %129 %130
%133 = OpAccessChain %_ptr_Function_v3float %129 %int_1
%134 = OpLoad %v3float %133
%135 = OpVectorShuffle %v4float %134 %134 0 1 2 2
OpStore %sk_FragColor %135
%138 = OpLoad %v2float %h2
%139 = OpLoad %v3float %h3
%137 = OpOuterProduct %mat3v2float %138 %139
OpStore %136 %137
%140 = OpAccessChain %_ptr_Function_v2float %136 %int_2
%141 = OpLoad %v2float %140
%142 = OpVectorShuffle %v4float %141 %141 0 1 1 1
OpStore %sk_FragColor %142
%145 = OpLoad %v4float %h4
%146 = OpLoad %v2float %h2
%144 = OpOuterProduct %mat2v4float %145 %146
OpStore %143 %144
%147 = OpAccessChain %_ptr_Function_v4float %143 %int_1
%148 = OpLoad %v4float %147
OpStore %sk_FragColor %148
%151 = OpLoad %v2float %h2
%152 = OpLoad %v4float %h4
%150 = OpOuterProduct %mat4v2float %151 %152
OpStore %149 %150
%153 = OpAccessChain %_ptr_Function_v2float %149 %int_3
%154 = OpLoad %v2float %153
%155 = OpVectorShuffle %v4float %154 %154 0 1 1 1
OpStore %sk_FragColor %155
%158 = OpLoad %v4float %h4
%159 = OpLoad %v3float %h3
%157 = OpOuterProduct %mat3v4float %158 %159
OpStore %156 %157
%160 = OpAccessChain %_ptr_Function_v4float %156 %int_2
%161 = OpLoad %v4float %160
OpStore %sk_FragColor %161
%164 = OpLoad %v3float %h3
%165 = OpLoad %v4float %h4
%163 = OpOuterProduct %mat4v3float %164 %165
OpStore %162 %163
%166 = OpAccessChain %_ptr_Function_v3float %162 %int_3
%167 = OpLoad %v3float %166
%168 = OpVectorShuffle %v4float %167 %167 0 1 2 2
OpStore %sk_FragColor %168
OpReturn
OpFunctionEnd
