OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_v2float_int_2 ArrayStride 16
OpDecorate %69 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%20 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%31 = OpConstantComposite %v2float %float_1 %float_0
%int_1 = OpConstant %int 1
%35 = OpConstantComposite %v2float %float_0 %float_1
%float_n1 = OpConstant %float -1
%float_2 = OpConstant %float 2
%39 = OpConstantComposite %v2float %float_n1 %float_2
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%17 = OpVariable %_ptr_Function_v2float Function
OpStore %17 %16
%19 = OpFunctionCall %v4float %main %17
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %20
%21 = OpFunctionParameter %_ptr_Function_v2float
%22 = OpLabel
%x = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%y = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%29 = OpAccessChain %_ptr_Function_v2float %x %int_0
OpStore %29 %16
%33 = OpAccessChain %_ptr_Function_v2float %x %int_1
OpStore %33 %31
%36 = OpAccessChain %_ptr_Function_v2float %y %int_0
OpStore %36 %35
%40 = OpAccessChain %_ptr_Function_v2float %y %int_1
OpStore %40 %39
%41 = OpAccessChain %_ptr_Function_v2float %x %int_0
%42 = OpLoad %v2float %41
%43 = OpCompositeExtract %float %42 0
%44 = OpAccessChain %_ptr_Function_v2float %x %int_0
%45 = OpLoad %v2float %44
%46 = OpCompositeExtract %float %45 1
%47 = OpFMul %float %43 %46
%48 = OpAccessChain %_ptr_Function_v2float %x %int_1
%49 = OpLoad %v2float %48
%50 = OpCompositeExtract %float %49 0
%51 = OpAccessChain %_ptr_Function_v2float %x %int_1
%52 = OpLoad %v2float %51
%53 = OpCompositeExtract %float %52 1
%54 = OpFSub %float %50 %53
%55 = OpAccessChain %_ptr_Function_v2float %y %int_0
%56 = OpLoad %v2float %55
%57 = OpCompositeExtract %float %56 0
%58 = OpAccessChain %_ptr_Function_v2float %y %int_0
%59 = OpLoad %v2float %58
%60 = OpCompositeExtract %float %59 1
%61 = OpFDiv %float %57 %60
%62 = OpAccessChain %_ptr_Function_v2float %y %int_1
%63 = OpLoad %v2float %62
%64 = OpCompositeExtract %float %63 0
%65 = OpAccessChain %_ptr_Function_v2float %y %int_1
%66 = OpLoad %v2float %65
%67 = OpCompositeExtract %float %66 1
%68 = OpFAdd %float %64 %67
%69 = OpCompositeConstruct %v4float %47 %54 %61 %68
OpReturnValue %69
OpFunctionEnd
