OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_v2float_int_2 ArrayStride 16
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%15 = OpTypeFunction %v4float
%v2float = OpTypeVector %float 2
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
%float_0 = OpConstant %float 0
%24 = OpConstantComposite %v2float %float_0 %float_0
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v2float %float_1 %float_0
%int_1 = OpConstant %int 1
%33 = OpConstantComposite %v2float %float_0 %float_1
%float_n1 = OpConstant %float -1
%float_2 = OpConstant %float 2
%37 = OpConstantComposite %v2float %float_n1 %float_2
%_entrypoint = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%x = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%y = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%26 = OpAccessChain %_ptr_Function_v2float %x %int_0
OpStore %26 %24
%31 = OpAccessChain %_ptr_Function_v2float %x %int_1
OpStore %31 %29
%34 = OpAccessChain %_ptr_Function_v2float %y %int_0
OpStore %34 %33
%38 = OpAccessChain %_ptr_Function_v2float %y %int_1
OpStore %38 %37
%39 = OpAccessChain %_ptr_Function_v2float %x %int_0
%40 = OpLoad %v2float %39
%41 = OpCompositeExtract %float %40 0
%42 = OpAccessChain %_ptr_Function_v2float %x %int_0
%43 = OpLoad %v2float %42
%44 = OpCompositeExtract %float %43 1
%45 = OpFMul %float %41 %44
%46 = OpAccessChain %_ptr_Function_v2float %x %int_1
%47 = OpLoad %v2float %46
%48 = OpCompositeExtract %float %47 0
%49 = OpAccessChain %_ptr_Function_v2float %x %int_1
%50 = OpLoad %v2float %49
%51 = OpCompositeExtract %float %50 1
%52 = OpFSub %float %48 %51
%53 = OpAccessChain %_ptr_Function_v2float %y %int_0
%54 = OpLoad %v2float %53
%55 = OpCompositeExtract %float %54 0
%56 = OpAccessChain %_ptr_Function_v2float %y %int_0
%57 = OpLoad %v2float %56
%58 = OpCompositeExtract %float %57 1
%59 = OpFDiv %float %55 %58
%60 = OpAccessChain %_ptr_Function_v2float %y %int_1
%61 = OpLoad %v2float %60
%62 = OpCompositeExtract %float %61 0
%63 = OpAccessChain %_ptr_Function_v2float %y %int_1
%64 = OpLoad %v2float %63
%65 = OpCompositeExtract %float %64 1
%66 = OpFAdd %float %62 %65
%67 = OpCompositeConstruct %v4float %45 %52 %59 %66
OpReturnValue %67
OpFunctionEnd
