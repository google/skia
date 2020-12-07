### Compilation failed:

error: SPIR-V validation error: OpConstantComposite Constituent <id> count does not match Result Type <id> '22[%v3float]'s vector component count.
  %23 = OpConstantComposite %v3float %24 %float_1

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
OpName %v4 "v4"
OpName %v5 "v5"
OpName %v6 "v6"
OpName %v7 "v7"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_ptr_Private_v2float = OpTypePointer Private %v2float
%v1 = OpVariable %_ptr_Private_v2float Private
%float_1 = OpConstant %float 1
%13 = OpConstantComposite %v2float %float_1 %float_1
%v2 = OpVariable %_ptr_Private_v2float Private
%float_2 = OpConstant %float 2
%16 = OpConstantComposite %v2float %float_1 %float_2
%v3 = OpVariable %_ptr_Private_v2float Private
%19 = OpConstantComposite %v2float %float_1 %float_1
%v3float = OpTypeVector %float 3
%_ptr_Private_v3float = OpTypePointer Private %v3float
%v4 = OpVariable %_ptr_Private_v3float Private
%24 = OpConstantComposite %v2float %float_1 %float_1
%23 = OpConstantComposite %v3float %24 %float_1
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%_ptr_Private_v2int = OpTypePointer Private %v2int
%v5 = OpVariable %_ptr_Private_v2int Private
%int_1 = OpConstant %int 1
%29 = OpConstantComposite %v2int %int_1 %int_1
%v6 = OpVariable %_ptr_Private_v2int Private
%33 = OpConstantComposite %v2float %float_1 %float_2
%32 = OpConstantComposite %v2int %33 %33
%v7 = OpVariable %_ptr_Private_v2float Private
%int_2 = OpConstant %int 2
%36 = OpConstantComposite %v2int %int_1 %int_2
%35 = OpConstantComposite %v2float %36 %36
%void = OpTypeVoid
%39 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %39
%40 = OpLabel
OpStore %v1 %13
OpStore %v2 %16
OpStore %v3 %19
OpStore %v4 %23
OpStore %v5 %29
OpStore %v6 %32
OpStore %v7 %35
%41 = OpLoad %v2float %v1
%42 = OpCompositeExtract %float %41 0
%43 = OpLoad %v2float %v2
%44 = OpCompositeExtract %float %43 0
%45 = OpFAdd %float %42 %44
%46 = OpLoad %v2float %v3
%47 = OpCompositeExtract %float %46 0
%48 = OpFAdd %float %45 %47
%49 = OpLoad %v3float %v4
%50 = OpCompositeExtract %float %49 0
%51 = OpFAdd %float %48 %50
%53 = OpLoad %v2int %v5
%54 = OpCompositeExtract %int %53 0
%52 = OpConvertSToF %float %54
%55 = OpFAdd %float %51 %52
%57 = OpLoad %v2int %v6
%58 = OpCompositeExtract %int %57 0
%56 = OpConvertSToF %float %58
%59 = OpFAdd %float %55 %56
%60 = OpLoad %v2float %v7
%61 = OpCompositeExtract %float %60 0
%62 = OpFAdd %float %59 %61
%63 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %63 %62
OpReturn
OpFunctionEnd

1 error
