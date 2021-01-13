OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %f "f"
OpName %i "i"
OpName %b "b"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %i1 "i1"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %b1 "b1"
OpName %b2 "b2"
OpName %b3 "b3"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %b RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_float = OpTypePointer Private %float
%f = OpVariable %_ptr_Private_float Private
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%i = OpVariable %_ptr_Private_int Private
%int_1 = OpConstant %int 1
%_ptr_Private_bool = OpTypePointer Private %bool
%b = OpVariable %_ptr_Private_bool Private
%true = OpConstantTrue %bool
%void = OpTypeVoid
%21 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %21
%22 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%i1 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_int Function
%i3 = OpVariable %_ptr_Function_int Function
%b1 = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_bool Function
%b3 = OpVariable %_ptr_Function_bool Function
OpStore %f %float_1
OpStore %i %int_1
OpStore %b %true
%25 = OpLoad %float %f
OpStore %f1 %25
%27 = OpLoad %int %i
%28 = OpConvertSToF %float %27
OpStore %f2 %28
%30 = OpLoad %bool %b
%31 = OpSelect %float %30 %float_1 %float_0
OpStore %f3 %31
%35 = OpLoad %float %f
%36 = OpConvertFToS %int %35
OpStore %i1 %36
%38 = OpLoad %int %i
OpStore %i2 %38
%40 = OpLoad %bool %b
%41 = OpSelect %int %40 %int_1 %int_0
OpStore %i3 %41
%45 = OpLoad %float %f
%46 = OpFOrdNotEqual %bool %45 %float_0
OpStore %b1 %46
%48 = OpLoad %int %i
%49 = OpINotEqual %bool %48 %int_0
OpStore %b2 %49
%51 = OpLoad %bool %b
OpStore %b3 %51
%52 = OpLoad %float %f1
%53 = OpLoad %float %f2
%54 = OpFAdd %float %52 %53
%55 = OpLoad %float %f3
%56 = OpFAdd %float %54 %55
%57 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %57 %56
%59 = OpLoad %int %i1
%60 = OpLoad %int %i2
%61 = OpIAdd %int %59 %60
%62 = OpLoad %int %i3
%63 = OpIAdd %int %61 %62
%64 = OpConvertSToF %float %63
%65 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %65 %64
%66 = OpLoad %bool %b1
OpSelectionMerge %68 None
OpBranchConditional %66 %68 %67
%67 = OpLabel
%69 = OpLoad %bool %b2
OpBranch %68
%68 = OpLabel
%70 = OpPhi %bool %true %22 %69 %67
OpSelectionMerge %72 None
OpBranchConditional %70 %72 %71
%71 = OpLabel
%73 = OpLoad %bool %b3
OpBranch %72
%72 = OpLabel
%74 = OpPhi %bool %true %68 %73 %71
%75 = OpSelect %float %74 %float_1 %float_0
%76 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %76 %75
OpReturn
OpFunctionEnd
