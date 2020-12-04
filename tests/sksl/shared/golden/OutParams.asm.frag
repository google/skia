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
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
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
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
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
%83 = OpConstantComposite %v2int %int_2 %int_2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_4 = OpConstant %int 4
%88 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%v3int = OpTypeVector %int 3
%90 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%_ptr_Function_int = OpTypePointer Function %int
%float_2_0 = OpConstant %float 2
%104 = OpConstantComposite %v2float %float_2_0 %float_2_0
%float_3_0 = OpConstant %float 3
%107 = OpConstantComposite %v3float %float_3_0 %float_3_0 %float_3_0
%109 = OpConstantComposite %v2float %float_2_0 %float_2_0
%float_1_0 = OpConstant %float 1
%float_4_0 = OpConstant %float 4
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%true = OpConstantTrue %bool
%149 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%false = OpConstantFalse %bool
%154 = OpConstantComposite %v4bool %false %false %false %false
%v2bool = OpTypeVector %bool 2
%156 = OpConstantComposite %v2bool %false %false
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
%128 = OpVariable %_ptr_Function_mat3v3float Function
%136 = OpVariable %_ptr_Function_mat4v4float Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
OpStore %h3 %16
OpStore %h4 %20
%25 = OpAccessChain %_ptr_Function_float %h3 %int_1
OpStore %25 %float_1
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
%68 = OpAccessChain %_ptr_Function_float %h2x2 %int_0 %int_0
OpStore %68 %float_1
%69 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%71 = OpLoad %v2float %69
%72 = OpVectorExtractDynamic %float %71 %int_0
%73 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%74 = OpLoad %v3float %73
%75 = OpVectorExtractDynamic %float %74 %int_0
%76 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%77 = OpLoad %v4float %76
%78 = OpVectorExtractDynamic %float %77 %int_0
%79 = OpCompositeConstruct %v4float %72 %75 %78 %float_1
OpStore %sk_FragColor %79
OpStore %i2 %83
OpStore %i4 %88
%92 = OpLoad %v4int %i4
%93 = OpVectorShuffle %v4int %92 %90 4 5 6 3
OpStore %i4 %93
%94 = OpAccessChain %_ptr_Function_int %i2 %int_1
OpStore %94 %int_1
%97 = OpLoad %v2int %i2
%98 = OpCompositeExtract %int %97 0
%96 = OpConvertSToF %float %98
%100 = OpLoad %v4int %i4
%101 = OpCompositeExtract %int %100 0
%99 = OpConvertSToF %float %101
%102 = OpCompositeConstruct %v4float %float_1 %96 %float_3 %99
OpStore %sk_FragColor %102
OpStore %f2 %104
OpStore %f3 %107
%110 = OpLoad %v3float %f3
%111 = OpVectorShuffle %v3float %110 %109 3 4 2
OpStore %f3 %111
%113 = OpAccessChain %_ptr_Function_float %f2 %int_0
OpStore %113 %float_1_0
%114 = OpLoad %v2float %f2
%115 = OpCompositeExtract %float %114 0
%116 = OpLoad %v3float %f3
%117 = OpCompositeExtract %float %116 0
%119 = OpCompositeConstruct %v4float %float_1_0 %115 %117 %float_4_0
OpStore %sk_FragColor %119
%122 = OpCompositeConstruct %v2float %float_2_0 %float_0
%123 = OpCompositeConstruct %v2float %float_0 %float_2_0
%121 = OpCompositeConstruct %mat2v2float %122 %123
OpStore %f2x2 %121
%124 = OpAccessChain %_ptr_Function_float %f2x2 %int_0 %int_0
OpStore %124 %float_1_0
%125 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%126 = OpLoad %v2float %125
%127 = OpVectorExtractDynamic %float %126 %int_0
%130 = OpCompositeConstruct %v3float %float_3_0 %float_0 %float_0
%131 = OpCompositeConstruct %v3float %float_0 %float_3_0 %float_0
%132 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3_0
%129 = OpCompositeConstruct %mat3v3float %130 %131 %132
OpStore %128 %129
%133 = OpAccessChain %_ptr_Function_v3float %128 %int_0
%134 = OpLoad %v3float %133
%135 = OpVectorExtractDynamic %float %134 %int_0
%138 = OpCompositeConstruct %v4float %float_4_0 %float_0 %float_0 %float_0
%139 = OpCompositeConstruct %v4float %float_0 %float_4_0 %float_0 %float_0
%140 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4_0 %float_0
%141 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4_0
%137 = OpCompositeConstruct %mat4v4float %138 %139 %140 %141
OpStore %136 %137
%142 = OpAccessChain %_ptr_Function_v4float %136 %int_0
%143 = OpLoad %v4float %142
%144 = OpVectorExtractDynamic %float %143 %int_0
%145 = OpCompositeConstruct %v4float %127 %135 %144 %float_1
OpStore %sk_FragColor %145
OpStore %b3 %149
OpStore %b4 %154
%158 = OpLoad %v4bool %b4
%159 = OpVectorShuffle %v4bool %158 %156 4 1 2 5
OpStore %b4 %159
%160 = OpAccessChain %_ptr_Function_bool %b3 %int_2
OpStore %160 %true
%162 = OpSelect %float %false %float_1 %float_0_0
%164 = OpLoad %v3bool %b3
%165 = OpCompositeExtract %bool %164 0
%166 = OpSelect %float %165 %float_1 %float_0_0
%167 = OpLoad %v4bool %b4
%168 = OpCompositeExtract %bool %167 0
%169 = OpSelect %float %168 %float_1 %float_0_0
%170 = OpCompositeConstruct %v4float %float_1 %162 %166 %169
OpStore %sk_FragColor %170
OpReturn
OpFunctionEnd
