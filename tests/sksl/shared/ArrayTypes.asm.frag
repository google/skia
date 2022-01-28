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
OpDecorate %99 RelaxedPrecision
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
%52 = OpVariable %_ptr_Function__arr_S_int_2 Function
%43 = OpAccessChain %_ptr_Function_v2float %x %int_0
OpStore %43 %17
%45 = OpAccessChain %_ptr_Function_v2float %x %int_1
OpStore %45 %44
%47 = OpAccessChain %_ptr_Function_v2float %y %int_0
OpStore %47 %30
%50 = OpAccessChain %_ptr_Function_v2float %y %int_1
OpStore %50 %49
%53 = OpFunctionCall %void %initialize_vS %52
%54 = OpLoad %_arr_S_int_2 %52
OpStore %z %54
%55 = OpAccessChain %_ptr_Function_v2float %x %int_0
%56 = OpLoad %v2float %55
%57 = OpCompositeExtract %float %56 0
%58 = OpAccessChain %_ptr_Function_v2float %x %int_0
%59 = OpLoad %v2float %58
%60 = OpCompositeExtract %float %59 1
%61 = OpFMul %float %57 %60
%62 = OpAccessChain %_ptr_Function_v2float %z %int_0 %int_0
%63 = OpLoad %v2float %62
%64 = OpCompositeExtract %float %63 0
%65 = OpFAdd %float %61 %64
%66 = OpAccessChain %_ptr_Function_v2float %x %int_1
%67 = OpLoad %v2float %66
%68 = OpCompositeExtract %float %67 0
%69 = OpAccessChain %_ptr_Function_v2float %x %int_1
%70 = OpLoad %v2float %69
%71 = OpCompositeExtract %float %70 1
%72 = OpAccessChain %_ptr_Function_v2float %z %int_0 %int_0
%73 = OpLoad %v2float %72
%74 = OpCompositeExtract %float %73 1
%75 = OpFMul %float %71 %74
%76 = OpFSub %float %68 %75
%77 = OpAccessChain %_ptr_Function_v2float %y %int_0
%78 = OpLoad %v2float %77
%79 = OpCompositeExtract %float %78 0
%80 = OpAccessChain %_ptr_Function_v2float %y %int_0
%81 = OpLoad %v2float %80
%82 = OpCompositeExtract %float %81 1
%83 = OpFDiv %float %79 %82
%84 = OpAccessChain %_ptr_Function_v2float %z %int_1 %int_0
%85 = OpLoad %v2float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpFDiv %float %83 %86
%88 = OpAccessChain %_ptr_Function_v2float %y %int_1
%89 = OpLoad %v2float %88
%90 = OpCompositeExtract %float %89 0
%91 = OpAccessChain %_ptr_Function_v2float %y %int_1
%92 = OpLoad %v2float %91
%93 = OpCompositeExtract %float %92 1
%94 = OpAccessChain %_ptr_Function_v2float %z %int_1 %int_0
%95 = OpLoad %v2float %94
%96 = OpCompositeExtract %float %95 1
%97 = OpFMul %float %93 %96
%98 = OpFAdd %float %90 %97
%99 = OpCompositeConstruct %v4float %65 %76 %87 %98
OpReturnValue %99
OpFunctionEnd
