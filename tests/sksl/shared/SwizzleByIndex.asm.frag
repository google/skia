OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorBlack"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %_0_v "_0_v"
OpName %_1_i "_1_i"
OpName %_2_x "_2_x"
OpName %_3_y "_3_y"
OpName %_4_z "_4_z"
OpName %_5_w "_5_w"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_n1_25 = OpConstant %float -1.25
%float_0 = OpConstant %float 0
%70 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%_0_v = OpVariable %_ptr_Function_v4float Function
%_1_i = OpVariable %_ptr_Function_v4int Function
%_2_x = OpVariable %_ptr_Function_float Function
%_3_y = OpVariable %_ptr_Function_float Function
%_4_z = OpVariable %_ptr_Function_float Function
%_5_w = OpVariable %_ptr_Function_float Function
%74 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
OpStore %_0_v %26
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%32 = OpLoad %v4float %30
%33 = OpCompositeExtract %float %32 0
%34 = OpConvertFToS %int %33
%35 = OpCompositeExtract %float %32 1
%36 = OpConvertFToS %int %35
%37 = OpCompositeExtract %float %32 2
%38 = OpConvertFToS %int %37
%39 = OpCompositeExtract %float %32 3
%40 = OpConvertFToS %int %39
%41 = OpCompositeConstruct %v4int %34 %36 %38 %40
OpStore %_1_i %41
%44 = OpLoad %v4float %_0_v
%45 = OpLoad %v4int %_1_i
%46 = OpCompositeExtract %int %45 0
%47 = OpVectorExtractDynamic %float %44 %46
OpStore %_2_x %47
%49 = OpLoad %v4float %_0_v
%50 = OpLoad %v4int %_1_i
%51 = OpCompositeExtract %int %50 1
%52 = OpVectorExtractDynamic %float %49 %51
OpStore %_3_y %52
%54 = OpLoad %v4float %_0_v
%55 = OpLoad %v4int %_1_i
%56 = OpCompositeExtract %int %55 2
%57 = OpVectorExtractDynamic %float %54 %56
OpStore %_4_z %57
%59 = OpLoad %v4float %_0_v
%60 = OpLoad %v4int %_1_i
%61 = OpCompositeExtract %int %60 3
%62 = OpVectorExtractDynamic %float %59 %61
OpStore %_5_w %62
%63 = OpLoad %float %_2_x
%64 = OpLoad %float %_3_y
%65 = OpLoad %float %_4_z
%66 = OpLoad %float %_5_w
%67 = OpCompositeConstruct %v4float %63 %64 %65 %66
%71 = OpFOrdEqual %v4bool %67 %70
%73 = OpAll %bool %71
OpSelectionMerge %77 None
OpBranchConditional %73 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%80 = OpLoad %v4float %78
OpStore %74 %80
OpBranch %77
%76 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%83 = OpLoad %v4float %81
OpStore %74 %83
OpBranch %77
%77 = OpLabel
%84 = OpLoad %v4float %74
OpReturnValue %84
OpFunctionEnd
