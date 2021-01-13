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
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
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
%v9 = OpVariable %_ptr_Private_v2float Private
%v10 = OpVariable %_ptr_Private_v2int Private
%int_3 = OpConstant %int 3
%v4bool = OpTypeVector %bool 4
%_ptr_Private_v4bool = OpTypePointer Private %v4bool
%v11 = OpVariable %_ptr_Private_v4bool Private
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%69 = OpConstantComposite %v4bool %true %false %true %false
%void = OpTypeVoid
%71 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %71
%72 = OpLabel
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
%51 = OpConvertSToF %float %53
%55 = OpLoad %v2int %v6
%56 = OpCompositeExtract %int %55 1
%54 = OpConvertSToF %float %56
%57 = OpCompositeConstruct %v2float %51 %54
OpStore %v9 %57
%61 = OpLoad %v2float %v1
%62 = OpCompositeExtract %float %61 0
%60 = OpConvertFToS %int %62
%63 = OpCompositeConstruct %v2int %int_3 %60
OpStore %v10 %63
OpStore %v11 %69
%73 = OpLoad %v2float %v1
%74 = OpCompositeExtract %float %73 0
%75 = OpLoad %v2float %v2
%76 = OpCompositeExtract %float %75 0
%77 = OpFAdd %float %74 %76
%78 = OpLoad %v2float %v3
%79 = OpCompositeExtract %float %78 0
%80 = OpFAdd %float %77 %79
%81 = OpLoad %v3float %v4
%82 = OpCompositeExtract %float %81 0
%83 = OpFAdd %float %80 %82
%85 = OpLoad %v2int %v5
%86 = OpCompositeExtract %int %85 0
%84 = OpConvertSToF %float %86
%87 = OpFAdd %float %83 %84
%89 = OpLoad %v2int %v6
%90 = OpCompositeExtract %int %89 0
%88 = OpConvertSToF %float %90
%91 = OpFAdd %float %87 %88
%92 = OpLoad %v2float %v7
%93 = OpCompositeExtract %float %92 0
%94 = OpFAdd %float %91 %93
%95 = OpLoad %v2float %v8
%96 = OpCompositeExtract %float %95 0
%97 = OpFAdd %float %94 %96
%98 = OpLoad %v2float %v9
%99 = OpCompositeExtract %float %98 0
%100 = OpFAdd %float %97 %99
%102 = OpLoad %v2int %v10
%103 = OpCompositeExtract %int %102 0
%101 = OpConvertSToF %float %103
%104 = OpFAdd %float %100 %101
%106 = OpLoad %v4bool %v11
%107 = OpCompositeExtract %bool %106 0
%105 = OpSelect %float %107 %float_1 %float_0
%109 = OpFAdd %float %104 %105
%110 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %110 %109
OpReturn
OpFunctionEnd
