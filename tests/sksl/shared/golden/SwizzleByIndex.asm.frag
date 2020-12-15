OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %_0_i "_0_i"
OpName %_1_v "_1_v"
OpName %_2_x "_2_x"
OpName %_3_y "_3_y"
OpName %_4_z "_4_z"
OpName %_5_w "_5_w"
OpName %_6_v "_6_v"
OpName %_7_x "_7_x"
OpName %_8_y "_8_y"
OpName %_9_z "_9_z"
OpName %_10_w "_10_w"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%71 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%main = OpFunction %void None %11
%12 = OpLabel
%_0_i = OpVariable %_ptr_Function_v4int Function
%_1_v = OpVariable %_ptr_Function_v4float Function
%_2_x = OpVariable %_ptr_Function_float Function
%_3_y = OpVariable %_ptr_Function_float Function
%_4_z = OpVariable %_ptr_Function_float Function
%_5_w = OpVariable %_ptr_Function_float Function
%_6_v = OpVariable %_ptr_Function_v4float Function
%_7_x = OpVariable %_ptr_Function_float Function
%_8_y = OpVariable %_ptr_Function_float Function
%_9_z = OpVariable %_ptr_Function_float Function
%_10_w = OpVariable %_ptr_Function_float Function
%25 = OpExtInst %float %1 Sqrt %float_1
%24 = OpConvertFToS %int %25
%27 = OpCompositeConstruct %v4int %24 %24 %24 %24
OpStore %_0_i %27
%28 = OpExtInst %float %1 Sqrt %float_1
%29 = OpCompositeConstruct %v4float %28 %28 %28 %28
OpStore %_1_v %29
%30 = OpLoad %v4float %_1_v
%31 = OpLoad %v4int %_0_i
%32 = OpCompositeExtract %int %31 0
%33 = OpVectorExtractDynamic %float %30 %32
OpStore %_2_x %33
%34 = OpLoad %v4float %_1_v
%35 = OpLoad %v4int %_0_i
%36 = OpCompositeExtract %int %35 1
%37 = OpVectorExtractDynamic %float %34 %36
OpStore %_3_y %37
%38 = OpLoad %v4float %_1_v
%39 = OpLoad %v4int %_0_i
%40 = OpCompositeExtract %int %39 2
%41 = OpVectorExtractDynamic %float %38 %40
OpStore %_4_z %41
%42 = OpLoad %v4float %_1_v
%43 = OpLoad %v4int %_0_i
%44 = OpCompositeExtract %int %43 3
%45 = OpVectorExtractDynamic %float %42 %44
OpStore %_5_w %45
%46 = OpLoad %float %_2_x
%47 = OpLoad %float %_3_y
%48 = OpLoad %float %_4_z
%49 = OpLoad %float %_5_w
%50 = OpCompositeConstruct %v4float %46 %47 %48 %49
OpStore %sk_FragColor %50
%56 = OpExtInst %float %1 Sqrt %float_1
%57 = OpCompositeConstruct %v4float %56 %56 %56 %56
OpStore %_6_v %57
%58 = OpLoad %v4float %_6_v
%59 = OpCompositeExtract %float %58 0
OpStore %_7_x %59
%60 = OpLoad %v4float %_6_v
%61 = OpCompositeExtract %float %60 1
OpStore %_8_y %61
%62 = OpLoad %v4float %_6_v
%63 = OpCompositeExtract %float %62 2
OpStore %_9_z %63
%64 = OpLoad %v4float %_6_v
%65 = OpCompositeExtract %float %64 3
OpStore %_10_w %65
%66 = OpLoad %float %_7_x
%67 = OpLoad %float %_8_y
%68 = OpLoad %float %_9_z
%69 = OpLoad %float %_10_w
%70 = OpCompositeConstruct %v4float %66 %67 %68 %69
OpStore %sk_FragColor %70
OpStore %sk_FragColor %71
OpReturn
OpFunctionEnd
