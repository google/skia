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
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %v11 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
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
%void = OpTypeVoid
%79 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %79
%80 = OpLabel
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
%53 = OpLoad %v2int %v6
%54 = OpCompositeExtract %int %53 0
%52 = OpConvertSToF %float %54
%55 = OpExtInst %float %1 Sqrt %float_2
%59 = OpCompositeExtract %int %58 0
%60 = OpConvertSToF %float %59
%61 = OpCompositeExtract %int %58 1
%62 = OpConvertSToF %float %61
%63 = OpCompositeConstruct %v2float %60 %62
%64 = OpCompositeExtract %float %63 0
%65 = OpCompositeExtract %float %63 1
%66 = OpCompositeConstruct %v4float %52 %55 %64 %65
OpStore %v9 %66
%69 = OpLoad %v2float %v1
%70 = OpCompositeExtract %float %69 0
%68 = OpConvertFToS %int %70
%71 = OpCompositeConstruct %v2int %int_3 %68
OpStore %v10 %71
OpStore %v11 %77
%81 = OpLoad %v2float %v1
%82 = OpCompositeExtract %float %81 0
%83 = OpLoad %v2float %v2
%84 = OpCompositeExtract %float %83 0
%85 = OpFAdd %float %82 %84
%86 = OpLoad %v2float %v3
%87 = OpCompositeExtract %float %86 0
%88 = OpFAdd %float %85 %87
%89 = OpLoad %v3float %v4
%90 = OpCompositeExtract %float %89 0
%91 = OpFAdd %float %88 %90
%93 = OpLoad %v2int %v5
%94 = OpCompositeExtract %int %93 0
%92 = OpConvertSToF %float %94
%95 = OpFAdd %float %91 %92
%97 = OpLoad %v2int %v6
%98 = OpCompositeExtract %int %97 0
%96 = OpConvertSToF %float %98
%99 = OpFAdd %float %95 %96
%100 = OpLoad %v2float %v7
%101 = OpCompositeExtract %float %100 0
%102 = OpFAdd %float %99 %101
%103 = OpLoad %v2float %v8
%104 = OpCompositeExtract %float %103 0
%105 = OpFAdd %float %102 %104
%106 = OpLoad %v4float %v9
%107 = OpCompositeExtract %float %106 0
%108 = OpFAdd %float %105 %107
%110 = OpLoad %v2int %v10
%111 = OpCompositeExtract %int %110 0
%109 = OpConvertSToF %float %111
%112 = OpFAdd %float %108 %109
%114 = OpLoad %v4bool %v11
%115 = OpCompositeExtract %bool %114 0
%113 = OpSelect %float %115 %float_1 %float_0
%117 = OpFAdd %float %112 %113
%118 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %118 %117
OpReturn
OpFunctionEnd
