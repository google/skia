OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %h3 "h3"
OpName %h4 "h4"
OpName %h2x2 "h2x2"
OpName %h3x3 "h3x3"
OpName %h4x4 "h4x4"
OpName %i2 "i2"
OpName %i4 "i4"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f2x2 "f2x2"
OpName %b3 "b3"
OpName %b4 "b4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %31 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_3 = OpConstant %float 3
%16 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_4 = OpConstant %float 4
%20 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%27 = OpConstantComposite %v2float %float_2 %float_2
%32 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%62 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%int_2 = OpConstant %int 2
%84 = OpConstantComposite %v2int %int_2 %int_2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_4 = OpConstant %int 4
%89 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%v3int = OpTypeVector %int 3
%91 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%_ptr_Function_int = OpTypePointer Function %int
%float_2_0 = OpConstant %float 2
%105 = OpConstantComposite %v2float %float_2_0 %float_2_0
%float_3_0 = OpConstant %float 3
%108 = OpConstantComposite %v3float %float_3_0 %float_3_0 %float_3_0
%110 = OpConstantComposite %v2float %float_2_0 %float_2_0
%float_1_0 = OpConstant %float 1
%float_4_0 = OpConstant %float 4
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%true = OpConstantTrue %bool
%151 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%false = OpConstantFalse %bool
%156 = OpConstantComposite %v4bool %false %false %false %false
%v2bool = OpTypeVector %bool 2
%158 = OpConstantComposite %v2bool %false %false
%_ptr_Function_bool = OpTypePointer Function %bool
%float_0_0 = OpConstant %float 0
%main = OpFunction %void None %11
%12 = OpLabel
%h3 = OpVariable %_ptr_Function_v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
%h2x2 = OpVariable %_ptr_Function_mat2v2float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%i2 = OpVariable %_ptr_Function_v2int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f2 = OpVariable %_ptr_Function_v2float Function
%f3 = OpVariable %_ptr_Function_v3float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%130 = OpVariable %_ptr_Function_mat3v3float Function
%138 = OpVariable %_ptr_Function_mat4v4float Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
OpStore %h3 %16
OpStore %h4 %20
%23 = OpAccessChain %_ptr_Function_float %h3 %int_1
OpStore %23 %float_1
%30 = OpLoad %v3float %h3
%31 = OpVectorShuffle %v3float %30 %27 3 1 4
OpStore %h3 %31
%33 = OpLoad %v4float %h4
%34 = OpVectorShuffle %v4float %33 %32 6 7 4 5
OpStore %h4 %34
%35 = OpLoad %v3float %h3
%36 = OpCompositeExtract %float %35 0
%37 = OpLoad %v4float %h4
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeConstruct %v4float %float_1 %float_2 %36 %38
OpStore %sk_FragColor %39
%45 = OpCompositeConstruct %v2float %float_2 %float_0
%46 = OpCompositeConstruct %v2float %float_0 %float_2
%43 = OpCompositeConstruct %mat2v2float %45 %46
OpStore %h2x2 %43
%51 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%52 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%53 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%50 = OpCompositeConstruct %mat3v3float %51 %52 %53
OpStore %h3x3 %50
%58 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%59 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%60 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%61 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%57 = OpCompositeConstruct %mat4v4float %58 %59 %60 %61
OpStore %h4x4 %57
%63 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
OpStore %63 %62
%65 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%66 = OpAccessChain %_ptr_Function_float %65 %int_3
OpStore %66 %float_1
%68 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%70 = OpAccessChain %_ptr_Function_float %68 %int_0
OpStore %70 %float_1
%71 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%72 = OpLoad %v2float %71
%73 = OpCompositeExtract %float %72 0
%74 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%75 = OpLoad %v3float %74
%76 = OpCompositeExtract %float %75 0
%77 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%78 = OpLoad %v4float %77
%79 = OpCompositeExtract %float %78 0
%80 = OpCompositeConstruct %v4float %73 %76 %79 %float_1
OpStore %sk_FragColor %80
OpStore %i2 %84
OpStore %i4 %89
%93 = OpLoad %v4int %i4
%94 = OpVectorShuffle %v4int %93 %91 4 5 6 3
OpStore %i4 %94
%95 = OpAccessChain %_ptr_Function_int %i2 %int_1
OpStore %95 %int_1
%98 = OpLoad %v2int %i2
%99 = OpCompositeExtract %int %98 0
%97 = OpConvertSToF %float %99
%101 = OpLoad %v4int %i4
%102 = OpCompositeExtract %int %101 0
%100 = OpConvertSToF %float %102
%103 = OpCompositeConstruct %v4float %float_1 %97 %float_3 %100
OpStore %sk_FragColor %103
OpStore %f2 %105
OpStore %f3 %108
%111 = OpLoad %v3float %f3
%112 = OpVectorShuffle %v3float %111 %110 3 4 2
OpStore %f3 %112
%114 = OpAccessChain %_ptr_Function_float %f2 %int_0
OpStore %114 %float_1_0
%115 = OpLoad %v2float %f2
%116 = OpCompositeExtract %float %115 0
%117 = OpLoad %v3float %f3
%118 = OpCompositeExtract %float %117 0
%120 = OpCompositeConstruct %v4float %float_1_0 %116 %118 %float_4_0
OpStore %sk_FragColor %120
%123 = OpCompositeConstruct %v2float %float_2_0 %float_0
%124 = OpCompositeConstruct %v2float %float_0 %float_2_0
%122 = OpCompositeConstruct %mat2v2float %123 %124
OpStore %f2x2 %122
%125 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%126 = OpAccessChain %_ptr_Function_float %125 %int_0
OpStore %126 %float_1_0
%127 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%128 = OpLoad %v2float %127
%129 = OpCompositeExtract %float %128 0
%132 = OpCompositeConstruct %v3float %float_3_0 %float_0 %float_0
%133 = OpCompositeConstruct %v3float %float_0 %float_3_0 %float_0
%134 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3_0
%131 = OpCompositeConstruct %mat3v3float %132 %133 %134
OpStore %130 %131
%135 = OpAccessChain %_ptr_Function_v3float %130 %int_0
%136 = OpLoad %v3float %135
%137 = OpCompositeExtract %float %136 0
%140 = OpCompositeConstruct %v4float %float_4_0 %float_0 %float_0 %float_0
%141 = OpCompositeConstruct %v4float %float_0 %float_4_0 %float_0 %float_0
%142 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4_0 %float_0
%143 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4_0
%139 = OpCompositeConstruct %mat4v4float %140 %141 %142 %143
OpStore %138 %139
%144 = OpAccessChain %_ptr_Function_v4float %138 %int_0
%145 = OpLoad %v4float %144
%146 = OpCompositeExtract %float %145 0
%147 = OpCompositeConstruct %v4float %129 %137 %146 %float_1
OpStore %sk_FragColor %147
OpStore %b3 %151
OpStore %b4 %156
%160 = OpLoad %v4bool %b4
%161 = OpVectorShuffle %v4bool %160 %158 4 1 2 5
OpStore %b4 %161
%162 = OpAccessChain %_ptr_Function_bool %b3 %int_2
OpStore %162 %true
%164 = OpSelect %float %false %float_1 %float_0_0
%166 = OpLoad %v3bool %b3
%167 = OpCompositeExtract %bool %166 0
%168 = OpSelect %float %167 %float_1 %float_0_0
%169 = OpLoad %v4bool %b4
%170 = OpCompositeExtract %bool %169 0
%171 = OpSelect %float %170 %float_1 %float_0_0
%172 = OpCompositeConstruct %v4float %float_1 %164 %168 %171
OpStore %sk_FragColor %172
OpReturn
OpFunctionEnd
