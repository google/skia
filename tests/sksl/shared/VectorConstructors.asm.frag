OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
OpName %v4 "v4"
OpName %v5 "v5"
OpName %v6 "v6"
OpName %v7 "v7"
OpName %v8 "v8"
OpName %v9 "v9"
OpName %v10 "v10"
OpName %v11 "v11"
OpName %v12 "v12"
OpName %v13 "v13"
OpName %v14 "v14"
OpName %v15 "v15"
OpName %v16 "v16"
OpName %v17 "v17"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %v11 RelaxedPrecision
OpDecorate %v15 RelaxedPrecision
OpDecorate %v16 RelaxedPrecision
OpDecorate %v17 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_ptr_Private_v2float = OpTypePointer Private %v2float
%v1 = OpVariable %_ptr_Private_v2float Private
%float_1 = OpConstant %float 1
%14 = OpConstantComposite %v2float %float_1 %float_1
%v2 = OpVariable %_ptr_Private_v2float Private
%float_2 = OpConstant %float 2
%17 = OpConstantComposite %v2float %float_1 %float_2
%v3 = OpVariable %_ptr_Private_v2float Private
%v3float = OpTypeVector %float 3
%_ptr_Private_v3float = OpTypePointer Private %v3float
%v4 = OpVariable %_ptr_Private_v3float Private
%22 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%_ptr_Private_v2int = OpTypePointer Private %v2int
%v5 = OpVariable %_ptr_Private_v2int Private
%int_1 = OpConstant %int 1
%28 = OpConstantComposite %v2int %int_1 %int_1
%v6 = OpVariable %_ptr_Private_v2int Private
%v7 = OpVariable %_ptr_Private_v2float Private
%int_2 = OpConstant %int 2
%37 = OpConstantComposite %v2int %int_1 %int_2
%v8 = OpVariable %_ptr_Private_v2float Private
%_ptr_Private_v4float = OpTypePointer Private %v4float
%v9 = OpVariable %_ptr_Private_v4float Private
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%58 = OpConstantComposite %v2int %int_3 %int_4
%v10 = OpVariable %_ptr_Private_v2int Private
%v4bool = OpTypeVector %bool 4
%_ptr_Private_v4bool = OpTypePointer Private %v4bool
%v11 = OpVariable %_ptr_Private_v4bool Private
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%77 = OpConstantComposite %v4bool %true %false %true %false
%v12 = OpVariable %_ptr_Private_v2float Private
%float_0 = OpConstant %float 0
%80 = OpConstantComposite %v2float %float_1 %float_0
%v13 = OpVariable %_ptr_Private_v2float Private
%82 = OpConstantComposite %v2float %float_0 %float_0
%v14 = OpVariable %_ptr_Private_v2float Private
%v2bool = OpTypeVector %bool 2
%85 = OpConstantComposite %v2bool %false %false
%_ptr_Private_v2bool = OpTypePointer Private %v2bool
%v15 = OpVariable %_ptr_Private_v2bool Private
%93 = OpConstantComposite %v2bool %true %true
%v16 = OpVariable %_ptr_Private_v2bool Private
%v3bool = OpTypeVector %bool 3
%_ptr_Private_v3bool = OpTypePointer Private %v3bool
%v17 = OpVariable %_ptr_Private_v3bool Private
%103 = OpConstantComposite %v3bool %true %true %true
%void = OpTypeVoid
%105 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %105
%106 = OpLabel
OpStore %v1 %14
OpStore %v2 %17
OpStore %v3 %14
OpStore %v4 %22
OpStore %v5 %28
%30 = OpCompositeExtract %float %17 0
%31 = OpConvertFToS %int %30
%32 = OpCompositeExtract %float %17 1
%33 = OpConvertFToS %int %32
%34 = OpCompositeConstruct %v2int %31 %33
OpStore %v6 %34
%38 = OpCompositeExtract %int %37 0
%39 = OpConvertSToF %float %38
%40 = OpCompositeExtract %int %37 1
%41 = OpConvertSToF %float %40
%42 = OpCompositeConstruct %v2float %39 %41
OpStore %v7 %42
%44 = OpLoad %v2int %v5
%45 = OpCompositeExtract %int %44 0
%46 = OpConvertSToF %float %45
%47 = OpCompositeExtract %int %44 1
%48 = OpConvertSToF %float %47
%49 = OpCompositeConstruct %v2float %46 %48
OpStore %v8 %49
%52 = OpLoad %v2int %v6
%53 = OpCompositeExtract %int %52 0
%54 = OpConvertSToF %float %53
%55 = OpExtInst %float %1 Sqrt %float_2
%59 = OpCompositeExtract %int %58 0
%60 = OpConvertSToF %float %59
%61 = OpCompositeExtract %int %58 1
%62 = OpConvertSToF %float %61
%63 = OpCompositeConstruct %v2float %60 %62
%64 = OpCompositeExtract %float %63 0
%65 = OpCompositeExtract %float %63 1
%66 = OpCompositeConstruct %v4float %54 %55 %64 %65
OpStore %v9 %66
%68 = OpLoad %v2float %v1
%69 = OpCompositeExtract %float %68 0
%70 = OpConvertFToS %int %69
%71 = OpCompositeConstruct %v2int %int_3 %70
OpStore %v10 %71
OpStore %v11 %77
OpStore %v12 %80
OpStore %v13 %82
%86 = OpCompositeExtract %bool %85 0
%87 = OpSelect %float %86 %float_1 %float_0
%88 = OpCompositeExtract %bool %85 1
%89 = OpSelect %float %88 %float_1 %float_0
%90 = OpCompositeConstruct %v2float %87 %89
OpStore %v14 %90
OpStore %v15 %93
%95 = OpCompositeExtract %float %14 0
%96 = OpFUnordNotEqual %bool %95 %float_0
%97 = OpCompositeExtract %float %14 1
%98 = OpFUnordNotEqual %bool %97 %float_0
%99 = OpCompositeConstruct %v2bool %96 %98
OpStore %v16 %99
OpStore %v17 %103
%107 = OpLoad %v2float %v1
%108 = OpCompositeExtract %float %107 0
%109 = OpLoad %v2float %v2
%110 = OpCompositeExtract %float %109 0
%111 = OpFAdd %float %108 %110
%112 = OpLoad %v2float %v3
%113 = OpCompositeExtract %float %112 0
%114 = OpFAdd %float %111 %113
%115 = OpLoad %v3float %v4
%116 = OpCompositeExtract %float %115 0
%117 = OpFAdd %float %114 %116
%118 = OpLoad %v2int %v5
%119 = OpCompositeExtract %int %118 0
%120 = OpConvertSToF %float %119
%121 = OpFAdd %float %117 %120
%122 = OpLoad %v2int %v6
%123 = OpCompositeExtract %int %122 0
%124 = OpConvertSToF %float %123
%125 = OpFAdd %float %121 %124
%126 = OpLoad %v2float %v7
%127 = OpCompositeExtract %float %126 0
%128 = OpFAdd %float %125 %127
%129 = OpLoad %v2float %v8
%130 = OpCompositeExtract %float %129 0
%131 = OpFAdd %float %128 %130
%132 = OpLoad %v4float %v9
%133 = OpCompositeExtract %float %132 0
%134 = OpFAdd %float %131 %133
%135 = OpLoad %v2int %v10
%136 = OpCompositeExtract %int %135 0
%137 = OpConvertSToF %float %136
%138 = OpFAdd %float %134 %137
%139 = OpLoad %v4bool %v11
%140 = OpCompositeExtract %bool %139 0
%141 = OpSelect %float %140 %float_1 %float_0
%142 = OpFAdd %float %138 %141
%143 = OpLoad %v2float %v12
%144 = OpCompositeExtract %float %143 0
%145 = OpFAdd %float %142 %144
%146 = OpLoad %v2float %v13
%147 = OpCompositeExtract %float %146 0
%148 = OpFAdd %float %145 %147
%149 = OpLoad %v2float %v14
%150 = OpCompositeExtract %float %149 0
%151 = OpFAdd %float %148 %150
%152 = OpLoad %v2bool %v15
%153 = OpCompositeExtract %bool %152 0
%154 = OpSelect %float %153 %float_1 %float_0
%155 = OpFAdd %float %151 %154
%156 = OpLoad %v2bool %v16
%157 = OpCompositeExtract %bool %156 0
%158 = OpSelect %float %157 %float_1 %float_0
%159 = OpFAdd %float %155 %158
%160 = OpLoad %v3bool %v17
%161 = OpCompositeExtract %bool %160 0
%162 = OpSelect %float %161 %float_1 %float_0
%163 = OpFAdd %float %159 %162
%164 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %164 %163
OpReturn
OpFunctionEnd
