OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %b "b"
OpName %c "c"
OpName %x "x"
OpName %d "d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %51 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_5 = OpConstant %float 5
%float_4 = OpConstant %float 4
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
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function_float Function
OpStore %b %float_2
OpStore %c %float_3
OpStore %x %int_0
OpBranch %32
%32 = OpLabel
OpLoopMerge %36 %35 None
OpBranch %33
%33 = OpLabel
%37 = OpLoad %int %x
%39 = OpSLessThan %bool %37 %int_1
OpBranchConditional %39 %34 %36
%34 = OpLabel
OpBranch %35
%35 = OpLabel
%40 = OpLoad %int %x
%41 = OpIAdd %int %40 %int_1
OpStore %x %41
OpBranch %32
%36 = OpLabel
%43 = OpLoad %float %c
OpStore %d %43
%44 = OpLoad %float %b
%46 = OpFAdd %float %44 %float_1
OpStore %b %46
%47 = OpLoad %float %d
%48 = OpFAdd %float %47 %float_1
OpStore %d %48
%49 = OpLoad %float %b
%50 = OpFOrdEqual %bool %49 %float_2
%51 = OpSelect %float %50 %float_1 %float_0
%52 = OpLoad %float %b
%53 = OpFOrdEqual %bool %52 %float_3
%54 = OpSelect %float %53 %float_1 %float_0
%55 = OpLoad %float %d
%57 = OpFOrdEqual %bool %55 %float_5
%58 = OpSelect %float %57 %float_1 %float_0
%59 = OpLoad %float %d
%61 = OpFOrdEqual %bool %59 %float_4
%62 = OpSelect %float %61 %float_1 %float_0
%63 = OpCompositeConstruct %v4float %51 %54 %58 %62
OpReturnValue %63
OpFunctionEnd
