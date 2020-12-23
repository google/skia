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
%32 = OpConstantComposite %v2float %float_1 %float_2
%v7 = OpVariable %_ptr_Private_v2float Private
%int_2 = OpConstant %int 2
%39 = OpConstantComposite %v2int %int_1 %int_2
%void = OpTypeVoid
%47 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %47
%48 = OpLabel
OpStore %v1 %13
OpStore %v2 %16
OpStore %v3 %19
OpStore %v4 %23
OpStore %v5 %29
%33 = OpCompositeExtract %float %32 0
%34 = OpConvertFToS %int %33
%35 = OpCompositeExtract %float %32 1
%36 = OpConvertFToS %int %35
%37 = OpCompositeConstruct %v2int %34 %36
OpStore %v6 %37
%41 = OpCompositeExtract %int %39 0
%42 = OpConvertSToF %float %41
%43 = OpCompositeExtract %int %39 1
%44 = OpConvertSToF %float %43
%45 = OpCompositeConstruct %v2float %42 %44
OpStore %v7 %45
%49 = OpLoad %v2float %v1
%50 = OpCompositeExtract %float %49 0
%51 = OpLoad %v2float %v2
%52 = OpCompositeExtract %float %51 0
%53 = OpFAdd %float %50 %52
%54 = OpLoad %v2float %v3
%55 = OpCompositeExtract %float %54 0
%56 = OpFAdd %float %53 %55
%57 = OpLoad %v3float %v4
%58 = OpCompositeExtract %float %57 0
%59 = OpFAdd %float %56 %58
%61 = OpLoad %v2int %v5
%62 = OpCompositeExtract %int %61 0
%60 = OpConvertSToF %float %62
%63 = OpFAdd %float %59 %60
%65 = OpLoad %v2int %v6
%66 = OpCompositeExtract %int %65 0
%64 = OpConvertSToF %float %66
%67 = OpFAdd %float %63 %64
%68 = OpLoad %v2float %v7
%69 = OpCompositeExtract %float %68 0
%70 = OpFAdd %float %67 %69
%71 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %71 %70
OpReturn
OpFunctionEnd

1 error
