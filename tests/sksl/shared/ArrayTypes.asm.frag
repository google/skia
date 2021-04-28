OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %S "S"
OpMemberName %S 0 "v"
OpName %initialize_vS "initialize_vS"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %S 0 Offset 0
OpDecorate %_arr_S_int_2 ArrayStride 16
OpDecorate %_arr_v2float_int_2 ArrayStride 16
OpDecorate %97 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%13 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%S = OpTypeStruct %v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_S_int_2 = OpTypeArray %S %int_2
%_ptr_Function__arr_S_int_2 = OpTypePointer Function %_arr_S_int_2
%25 = OpTypeFunction %void %_ptr_Function__arr_S_int_2
%float_1 = OpConstant %float 1
%30 = OpConstantComposite %v2float %float_0 %float_1
%int_0 = OpConstant %int 0
%float_2 = OpConstant %float 2
%34 = OpConstantComposite %v2float %float_2 %float_1
%int_1 = OpConstant %int 1
%37 = OpTypeFunction %v4float %_ptr_Function_v2float
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
%44 = OpConstantComposite %v2float %float_1 %float_0
%float_n1 = OpConstant %float -1
%49 = OpConstantComposite %v2float %float_n1 %float_2
%_entrypoint_v = OpFunction %void None %13
%14 = OpLabel
%18 = OpVariable %_ptr_Function_v2float Function
OpStore %18 %17
%20 = OpFunctionCall %v4float %main %18
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%initialize_vS = OpFunction %void None %25
%27 = OpFunctionParameter %_ptr_Function__arr_S_int_2
%28 = OpLabel
%32 = OpAccessChain %_ptr_Function_v2float %27 %int_0 %int_0
OpStore %32 %30
%36 = OpAccessChain %_ptr_Function_v2float %27 %int_1 %int_0
OpStore %36 %34
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %37
%38 = OpFunctionParameter %_ptr_Function_v2float
%39 = OpLabel
%x = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%y = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%z = OpVariable %_ptr_Function__arr_S_int_2 Function
%43 = OpAccessChain %_ptr_Function_v2float %x %int_0
OpStore %43 %17
%45 = OpAccessChain %_ptr_Function_v2float %x %int_1
OpStore %45 %44
%47 = OpAccessChain %_ptr_Function_v2float %y %int_0
OpStore %47 %30
%50 = OpAccessChain %_ptr_Function_v2float %y %int_1
OpStore %50 %49
%52 = OpFunctionCall %void %initialize_vS %z
%53 = OpAccessChain %_ptr_Function_v2float %x %int_0
%54 = OpLoad %v2float %53
%55 = OpCompositeExtract %float %54 0
%56 = OpAccessChain %_ptr_Function_v2float %x %int_0
%57 = OpLoad %v2float %56
%58 = OpCompositeExtract %float %57 1
%59 = OpFMul %float %55 %58
%60 = OpAccessChain %_ptr_Function_v2float %z %int_0 %int_0
%61 = OpLoad %v2float %60
%62 = OpCompositeExtract %float %61 0
%63 = OpFAdd %float %59 %62
%64 = OpAccessChain %_ptr_Function_v2float %x %int_1
%65 = OpLoad %v2float %64
%66 = OpCompositeExtract %float %65 0
%67 = OpAccessChain %_ptr_Function_v2float %x %int_1
%68 = OpLoad %v2float %67
%69 = OpCompositeExtract %float %68 1
%70 = OpAccessChain %_ptr_Function_v2float %z %int_0 %int_0
%71 = OpLoad %v2float %70
%72 = OpCompositeExtract %float %71 1
%73 = OpFMul %float %69 %72
%74 = OpFSub %float %66 %73
%75 = OpAccessChain %_ptr_Function_v2float %y %int_0
%76 = OpLoad %v2float %75
%77 = OpCompositeExtract %float %76 0
%78 = OpAccessChain %_ptr_Function_v2float %y %int_0
%79 = OpLoad %v2float %78
%80 = OpCompositeExtract %float %79 1
%81 = OpFDiv %float %77 %80
%82 = OpAccessChain %_ptr_Function_v2float %z %int_1 %int_0
%83 = OpLoad %v2float %82
%84 = OpCompositeExtract %float %83 0
%85 = OpFDiv %float %81 %84
%86 = OpAccessChain %_ptr_Function_v2float %y %int_1
%87 = OpLoad %v2float %86
%88 = OpCompositeExtract %float %87 0
%89 = OpAccessChain %_ptr_Function_v2float %y %int_1
%90 = OpLoad %v2float %89
%91 = OpCompositeExtract %float %90 1
%92 = OpAccessChain %_ptr_Function_v2float %z %int_1 %int_0
%93 = OpLoad %v2float %92
%94 = OpCompositeExtract %float %93 1
%95 = OpFMul %float %91 %94
%96 = OpFAdd %float %88 %95
%97 = OpCompositeConstruct %v4float %63 %74 %85 %96
OpReturnValue %97
OpFunctionEnd
