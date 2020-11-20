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
OpName %h3x3 "h3x3"
OpName %h4x4 "h4x4"
OpName %i4 "i4"
OpName %f3 "f3"
OpName %f2x2 "f2x2"
OpName %b4 "b4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
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
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%22 = OpConstantComposite %v2float %float_2 %float_2
%27 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_1 = OpConstant %float 1
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_0 = OpConstant %float 0
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%52 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_4 = OpConstant %int 4
%81 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%v3int = OpTypeVector %int 3
%83 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%float_3_0 = OpConstant %float 3
%92 = OpConstantComposite %v3float %float_3_0 %float_3_0 %float_3_0
%float_2_0 = OpConstant %float 2
%94 = OpConstantComposite %v2float %float_2_0 %float_2_0
%float_1_0 = OpConstant %float 1
%float_4_0 = OpConstant %float 4
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%false = OpConstantFalse %bool
%132 = OpConstantComposite %v4bool %false %false %false %false
%v2bool = OpTypeVector %bool 2
%134 = OpConstantComposite %v2bool %false %false
%float_0_0 = OpConstant %float 0
%true = OpConstantTrue %bool
%main = OpFunction %void None %11
%12 = OpLabel
%h3 = OpVariable %_ptr_Function_v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%60 = OpVariable %_ptr_Function_mat2v2float Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f3 = OpVariable %_ptr_Function_v3float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%111 = OpVariable %_ptr_Function_mat3v3float Function
%119 = OpVariable %_ptr_Function_mat4v4float Function
%b4 = OpVariable %_ptr_Function_v4bool Function
OpStore %h3 %16
OpStore %h4 %20
%25 = OpLoad %v3float %h3
%26 = OpVectorShuffle %v3float %25 %22 3 1 4
OpStore %h3 %26
%28 = OpLoad %v4float %h4
%29 = OpVectorShuffle %v4float %28 %27 6 7 4 5
OpStore %h4 %29
%31 = OpLoad %v3float %h3
%32 = OpCompositeExtract %float %31 0
%33 = OpLoad %v4float %h4
%34 = OpCompositeExtract %float %33 0
%35 = OpCompositeConstruct %v4float %float_1 %float_2 %32 %34
OpStore %sk_FragColor %35
%41 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%42 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%43 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%39 = OpCompositeConstruct %mat3v3float %41 %42 %43
OpStore %h3x3 %39
%48 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%49 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%50 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%51 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%47 = OpCompositeConstruct %mat4v4float %48 %49 %50 %51
OpStore %h4x4 %47
%55 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
OpStore %55 %52
%57 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%58 = OpAccessChain %_ptr_Function_float %57 %int_3
OpStore %58 %float_1
%64 = OpCompositeConstruct %v2float %float_2 %float_0
%65 = OpCompositeConstruct %v2float %float_0 %float_2
%63 = OpCompositeConstruct %mat2v2float %64 %65
OpStore %60 %63
%67 = OpAccessChain %_ptr_Function_v2float %60 %int_0
%69 = OpLoad %v2float %67
%70 = OpVectorExtractDynamic %float %69 %int_0
%71 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%72 = OpLoad %v3float %71
%73 = OpVectorExtractDynamic %float %72 %int_0
%74 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%75 = OpLoad %v4float %74
%76 = OpVectorExtractDynamic %float %75 %int_0
%77 = OpCompositeConstruct %v4float %70 %73 %76 %float_1
OpStore %sk_FragColor %77
OpStore %i4 %81
%85 = OpLoad %v4int %i4
%86 = OpVectorShuffle %v4int %85 %83 4 5 6 3
OpStore %i4 %86
%88 = OpLoad %v4int %i4
%89 = OpCompositeExtract %int %88 0
%87 = OpConvertSToF %float %89
%90 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %87
OpStore %sk_FragColor %90
OpStore %f3 %92
%96 = OpLoad %v3float %f3
%97 = OpVectorShuffle %v3float %96 %94 3 4 2
OpStore %f3 %97
%99 = OpLoad %v3float %f3
%100 = OpCompositeExtract %float %99 0
%102 = OpCompositeConstruct %v4float %float_1_0 %float_2_0 %100 %float_4_0
OpStore %sk_FragColor %102
%105 = OpCompositeConstruct %v2float %float_2_0 %float_0
%106 = OpCompositeConstruct %v2float %float_0 %float_2_0
%104 = OpCompositeConstruct %mat2v2float %105 %106
OpStore %f2x2 %104
%107 = OpAccessChain %_ptr_Function_float %f2x2 %int_0 %int_0
OpStore %107 %float_1_0
%108 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%109 = OpLoad %v2float %108
%110 = OpVectorExtractDynamic %float %109 %int_0
%113 = OpCompositeConstruct %v3float %float_3_0 %float_0 %float_0
%114 = OpCompositeConstruct %v3float %float_0 %float_3_0 %float_0
%115 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3_0
%112 = OpCompositeConstruct %mat3v3float %113 %114 %115
OpStore %111 %112
%116 = OpAccessChain %_ptr_Function_v3float %111 %int_0
%117 = OpLoad %v3float %116
%118 = OpVectorExtractDynamic %float %117 %int_0
%121 = OpCompositeConstruct %v4float %float_4_0 %float_0 %float_0 %float_0
%122 = OpCompositeConstruct %v4float %float_0 %float_4_0 %float_0 %float_0
%123 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4_0 %float_0
%124 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4_0
%120 = OpCompositeConstruct %mat4v4float %121 %122 %123 %124
OpStore %119 %120
%125 = OpAccessChain %_ptr_Function_v4float %119 %int_0
%126 = OpLoad %v4float %125
%127 = OpVectorExtractDynamic %float %126 %int_0
%128 = OpCompositeConstruct %v4float %110 %118 %127 %float_1
OpStore %sk_FragColor %128
OpStore %b4 %132
%136 = OpLoad %v4bool %b4
%137 = OpVectorShuffle %v4bool %136 %134 4 1 2 5
OpStore %b4 %137
%138 = OpSelect %float %false %float_1 %float_0_0
%141 = OpSelect %float %true %float_1 %float_0_0
%142 = OpLoad %v4bool %b4
%143 = OpCompositeExtract %bool %142 0
%144 = OpSelect %float %143 %float_1 %float_0_0
%145 = OpCompositeConstruct %v4float %float_1 %138 %141 %144
OpStore %sk_FragColor %145
OpReturn
OpFunctionEnd
