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
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
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
%17 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_4 = OpConstant %float 4
%21 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%v2float = OpTypeVector %float 2
%float_2 = OpConstant %float 2
%29 = OpConstantComposite %v2float %float_2 %float_2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
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
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%true = OpConstantTrue %bool
%143 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%false = OpConstantFalse %bool
%148 = OpConstantComposite %v4bool %false %false %false %false
%v2bool = OpTypeVector %bool 2
%150 = OpConstantComposite %v2bool %false %false
%_ptr_Function_bool = OpTypePointer Function %bool
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
%121 = OpVariable %_ptr_Function_mat3v3float Function
%129 = OpVariable %_ptr_Function_mat4v4float Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
OpStore %h3 %17
OpStore %h4 %21
%23 = OpAccessChain %_ptr_Function_float %h3 %int_1
OpStore %23 %float_1
%30 = OpLoad %v3float %h3
%31 = OpVectorShuffle %v3float %30 %29 3 1 4
OpStore %h3 %31
%32 = OpLoad %v4float %h4
%33 = OpVectorShuffle %v4float %32 %21 6 7 4 5
OpStore %h4 %33
%34 = OpLoad %v3float %h3
%35 = OpCompositeExtract %float %34 0
%36 = OpLoad %v4float %h4
%37 = OpCompositeExtract %float %36 0
%38 = OpCompositeConstruct %v4float %float_1 %float_2 %35 %37
OpStore %sk_FragColor %38
%44 = OpCompositeConstruct %v2float %float_2 %float_0
%45 = OpCompositeConstruct %v2float %float_0 %float_2
%42 = OpCompositeConstruct %mat2v2float %44 %45
OpStore %h2x2 %42
%50 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%51 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%52 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%49 = OpCompositeConstruct %mat3v3float %50 %51 %52
OpStore %h3x3 %49
%57 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%58 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%59 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%60 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%56 = OpCompositeConstruct %mat4v4float %57 %58 %59 %60
OpStore %h4x4 %56
%61 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
OpStore %61 %17
%63 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%64 = OpAccessChain %_ptr_Function_float %63 %int_3
OpStore %64 %float_1
%66 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%68 = OpAccessChain %_ptr_Function_float %66 %int_0
OpStore %68 %float_1
%69 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%70 = OpLoad %v2float %69
%71 = OpCompositeExtract %float %70 0
%72 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%73 = OpLoad %v3float %72
%74 = OpCompositeExtract %float %73 0
%75 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%76 = OpLoad %v4float %75
%77 = OpCompositeExtract %float %76 0
%78 = OpCompositeConstruct %v4float %71 %74 %77 %float_1
OpStore %sk_FragColor %78
OpStore %i2 %83
OpStore %i4 %88
%91 = OpLoad %v4int %i4
%92 = OpVectorShuffle %v4int %91 %90 4 5 6 3
OpStore %i4 %92
%93 = OpAccessChain %_ptr_Function_int %i2 %int_1
OpStore %93 %int_1
%95 = OpLoad %v2int %i2
%96 = OpCompositeExtract %int %95 0
%97 = OpConvertSToF %float %96
%98 = OpLoad %v4int %i4
%99 = OpCompositeExtract %int %98 0
%100 = OpConvertSToF %float %99
%101 = OpCompositeConstruct %v4float %float_1 %97 %float_3 %100
OpStore %sk_FragColor %101
OpStore %f2 %29
OpStore %f3 %17
%104 = OpLoad %v3float %f3
%105 = OpVectorShuffle %v3float %104 %29 3 4 2
OpStore %f3 %105
%106 = OpAccessChain %_ptr_Function_float %f2 %int_0
OpStore %106 %float_1
%107 = OpLoad %v2float %f2
%108 = OpCompositeExtract %float %107 0
%109 = OpLoad %v3float %f3
%110 = OpCompositeExtract %float %109 0
%111 = OpCompositeConstruct %v4float %float_1 %108 %110 %float_4
OpStore %sk_FragColor %111
%114 = OpCompositeConstruct %v2float %float_2 %float_0
%115 = OpCompositeConstruct %v2float %float_0 %float_2
%113 = OpCompositeConstruct %mat2v2float %114 %115
OpStore %f2x2 %113
%116 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%117 = OpAccessChain %_ptr_Function_float %116 %int_0
OpStore %117 %float_1
%118 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%119 = OpLoad %v2float %118
%120 = OpCompositeExtract %float %119 0
%123 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%124 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%125 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%122 = OpCompositeConstruct %mat3v3float %123 %124 %125
OpStore %121 %122
%126 = OpAccessChain %_ptr_Function_v3float %121 %int_0
%127 = OpLoad %v3float %126
%128 = OpCompositeExtract %float %127 0
%131 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%132 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%133 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%134 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%130 = OpCompositeConstruct %mat4v4float %131 %132 %133 %134
OpStore %129 %130
%135 = OpAccessChain %_ptr_Function_v4float %129 %int_0
%136 = OpLoad %v4float %135
%137 = OpCompositeExtract %float %136 0
%138 = OpCompositeConstruct %v4float %120 %128 %137 %float_1
OpStore %sk_FragColor %138
OpStore %b3 %143
OpStore %b4 %148
%151 = OpLoad %v4bool %b4
%152 = OpVectorShuffle %v4bool %151 %150 4 1 2 5
OpStore %b4 %152
%153 = OpAccessChain %_ptr_Function_bool %b3 %int_2
OpStore %153 %true
%155 = OpLoad %v3bool %b3
%156 = OpCompositeExtract %bool %155 0
%157 = OpSelect %float %156 %float_1 %float_0
%158 = OpLoad %v4bool %b4
%159 = OpCompositeExtract %bool %158 0
%160 = OpSelect %float %159 %float_1 %float_0
%161 = OpCompositeConstruct %v4float %float_1 %float_0 %157 %160
OpStore %sk_FragColor %161
OpReturn
OpFunctionEnd
