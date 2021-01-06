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
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
%void = OpTypeVoid
%44 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %44
%45 = OpLabel
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
%46 = OpLoad %v2float %v1
%47 = OpCompositeExtract %float %46 0
%48 = OpLoad %v2float %v2
%49 = OpCompositeExtract %float %48 0
%50 = OpFAdd %float %47 %49
%51 = OpLoad %v2float %v3
%52 = OpCompositeExtract %float %51 0
%53 = OpFAdd %float %50 %52
%54 = OpLoad %v3float %v4
%55 = OpCompositeExtract %float %54 0
%56 = OpFAdd %float %53 %55
%58 = OpLoad %v2int %v5
%59 = OpCompositeExtract %int %58 0
%57 = OpConvertSToF %float %59
%60 = OpFAdd %float %56 %57
%62 = OpLoad %v2int %v6
%63 = OpCompositeExtract %int %62 0
%61 = OpConvertSToF %float %63
%64 = OpFAdd %float %60 %61
%65 = OpLoad %v2float %v7
%66 = OpCompositeExtract %float %65 0
%67 = OpFAdd %float %64 %66
%68 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %68 %67
OpReturn
OpFunctionEnd
