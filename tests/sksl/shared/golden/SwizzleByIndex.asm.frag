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
OpDecorate %27 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
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
%18 = OpExtInst %float %1 Sqrt %float_1
%17 = OpConvertFToS %int %18
%20 = OpCompositeConstruct %v4int %17 %17 %17 %17
OpStore %_0_i %20
%23 = OpExtInst %float %1 Sqrt %float_1
%24 = OpCompositeConstruct %v4float %23 %23 %23 %23
OpStore %_1_v %24
%27 = OpLoad %v4float %_1_v
%28 = OpLoad %v4int %_0_i
%29 = OpCompositeExtract %int %28 0
%30 = OpVectorExtractDynamic %float %27 %29
OpStore %_2_x %30
%32 = OpLoad %v4float %_1_v
%33 = OpLoad %v4int %_0_i
%34 = OpCompositeExtract %int %33 1
%35 = OpVectorExtractDynamic %float %32 %34
OpStore %_3_y %35
%37 = OpLoad %v4float %_1_v
%38 = OpLoad %v4int %_0_i
%39 = OpCompositeExtract %int %38 2
%40 = OpVectorExtractDynamic %float %37 %39
OpStore %_4_z %40
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
%52 = OpExtInst %float %1 Sqrt %float_1
%53 = OpCompositeConstruct %v4float %52 %52 %52 %52
OpStore %_6_v %53
%55 = OpLoad %v4float %_6_v
%56 = OpCompositeExtract %float %55 0
OpStore %_7_x %56
%58 = OpLoad %v4float %_6_v
%59 = OpCompositeExtract %float %58 1
OpStore %_8_y %59
%61 = OpLoad %v4float %_6_v
%62 = OpCompositeExtract %float %61 2
OpStore %_9_z %62
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
