OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %increment_vfff "increment_vfff"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %x "x"
OpName %d "d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%21 = OpTypeFunction %void %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float
%float_1 = OpConstant %float 1
%34 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_5 = OpConstant %float 5
%float_4 = OpConstant %float 4
%_entrypoint_v = OpFunction %void None %13
%14 = OpLabel
%18 = OpVariable %_ptr_Function_v2float Function
OpStore %18 %17
%20 = OpFunctionCall %v4float %main %18
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%increment_vfff = OpFunction %void None %21
%23 = OpFunctionParameter %_ptr_Function_float
%24 = OpFunctionParameter %_ptr_Function_float
%25 = OpFunctionParameter %_ptr_Function_float
%26 = OpLabel
%27 = OpLoad %float %23
%29 = OpFAdd %float %27 %float_1
OpStore %23 %29
%30 = OpLoad %float %24
%31 = OpFAdd %float %30 %float_1
OpStore %24 %31
%32 = OpLoad %float %25
%33 = OpFAdd %float %32 %float_1
OpStore %25 %33
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %34
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpLabel
%a = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function_float Function
OpStore %a %float_1
OpStore %b %float_2
OpStore %c %float_3
OpStore %x %int_0
OpBranch %46
%46 = OpLabel
OpLoopMerge %50 %49 None
OpBranch %47
%47 = OpLabel
%51 = OpLoad %int %x
%53 = OpSLessThan %bool %51 %int_1
OpBranchConditional %53 %48 %50
%48 = OpLabel
OpBranch %50
%49 = OpLabel
%54 = OpLoad %int %x
%55 = OpIAdd %int %54 %int_1
OpStore %x %55
OpBranch %46
%50 = OpLabel
%57 = OpLoad %float %c
OpStore %d %57
%58 = OpLoad %float %b
%59 = OpFAdd %float %58 %float_1
OpStore %b %59
%60 = OpLoad %float %d
%61 = OpFAdd %float %60 %float_1
OpStore %d %61
%62 = OpLoad %float %b
%63 = OpFOrdEqual %bool %62 %float_2
%64 = OpSelect %float %63 %float_1 %float_0
%65 = OpLoad %float %b
%66 = OpFOrdEqual %bool %65 %float_3
%67 = OpSelect %float %66 %float_1 %float_0
%68 = OpLoad %float %d
%70 = OpFOrdEqual %bool %68 %float_5
%71 = OpSelect %float %70 %float_1 %float_0
%72 = OpLoad %float %d
%74 = OpFOrdEqual %bool %72 %float_4
%75 = OpSelect %float %74 %float_1 %float_0
%76 = OpCompositeConstruct %v4float %64 %67 %71 %75
OpReturnValue %76
OpFunctionEnd
