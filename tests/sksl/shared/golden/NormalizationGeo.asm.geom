### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '15[%sk_RTAdjust]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %sk_RTAdjust = OpVariable %_ptr_Uniform_v4float Uniform

OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %3 %8 %sk_InvocationID
OpExecutionMode %main InputPoints
OpExecutionMode %main OutputLineStrip
OpExecutionMode %main OutputVertices 2
OpExecutionMode %main Invocations 2
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_InvocationID "sk_InvocationID"
OpName %sk_RTAdjust "sk_RTAdjust"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %_arr_sk_PerVertex_int_1 ArrayStride 32
OpDecorate %sk_InvocationID BuiltIn InvocationId
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_arr_sk_PerVertex_int_1 = OpTypeArray %sk_PerVertex %int_1
%_ptr_Input__arr_sk_PerVertex_int_1 = OpTypePointer Input %_arr_sk_PerVertex_int_1
%8 = OpVariable %_ptr_Input__arr_sk_PerVertex_int_1 Input
%_ptr_Input_int = OpTypePointer Input %int
%sk_InvocationID = OpVariable %_ptr_Input_int Input
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%sk_RTAdjust = OpVariable %_ptr_Uniform_v4float Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%float_n0_5 = OpConstant %float -0.5
%float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%float_0_5 = OpConstant %float 0.5
%main = OpFunction %void None %18
%19 = OpLabel
%21 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%23 = OpLoad %v4float %21
%27 = OpLoad %int %sk_InvocationID
%26 = OpConvertSToF %float %27
%28 = OpCompositeConstruct %v4float %float_n0_5 %float_0 %float_0 %26
%29 = OpFAdd %v4float %23 %28
%30 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %30 %29
%32 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%33 = OpLoad %v4float %32
%34 = OpVectorShuffle %v2float %33 %33 0 1
%36 = OpLoad %v4float %sk_RTAdjust
%37 = OpVectorShuffle %v2float %36 %36 0 2
%38 = OpFMul %v2float %34 %37
%39 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%40 = OpLoad %v4float %39
%41 = OpVectorShuffle %v2float %40 %40 3 3
%42 = OpLoad %v4float %sk_RTAdjust
%43 = OpVectorShuffle %v2float %42 %42 1 3
%44 = OpFMul %v2float %41 %43
%45 = OpFAdd %v2float %38 %44
%46 = OpCompositeExtract %float %45 0
%47 = OpCompositeExtract %float %45 1
%48 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%49 = OpLoad %v4float %48
%50 = OpCompositeExtract %float %49 3
%51 = OpCompositeConstruct %v4float %46 %47 %float_0 %50
%52 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %52 %51
OpEmitVertex
%54 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%55 = OpLoad %v4float %54
%58 = OpLoad %int %sk_InvocationID
%57 = OpConvertSToF %float %58
%59 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %57
%60 = OpFAdd %v4float %55 %59
%61 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %61 %60
%62 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v2float %63 %63 0 1
%65 = OpLoad %v4float %sk_RTAdjust
%66 = OpVectorShuffle %v2float %65 %65 0 2
%67 = OpFMul %v2float %64 %66
%68 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%69 = OpLoad %v4float %68
%70 = OpVectorShuffle %v2float %69 %69 3 3
%71 = OpLoad %v4float %sk_RTAdjust
%72 = OpVectorShuffle %v2float %71 %71 1 3
%73 = OpFMul %v2float %70 %72
%74 = OpFAdd %v2float %67 %73
%75 = OpCompositeExtract %float %74 0
%76 = OpCompositeExtract %float %74 1
%77 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%78 = OpLoad %v4float %77
%79 = OpCompositeExtract %float %78 3
%80 = OpCompositeConstruct %v4float %75 %76 %float_0 %79
%81 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %81 %80
OpEmitVertex
OpEndPrimitive
OpReturn
OpFunctionEnd

1 error
