               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %zero "zero"
               OpName %globalArray "globalArray"
               OpName %globalMatrix "globalMatrix"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %localArray "localArray"
               OpName %localMatrix "localMatrix"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %globalArray RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %16 RelaxedPrecision
               OpDecorate %globalMatrix RelaxedPrecision
               OpDecorate %localArray RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %localMatrix RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
       %zero = OpVariable %_ptr_Private_int Private
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Private__arr_float_int_2 = OpTypePointer Private %_arr_float_int_2
%globalArray = OpVariable %_ptr_Private__arr_float_int_2 Private
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Private_mat2v2float = OpTypePointer Private %mat2v2float
%globalMatrix = OpVariable %_ptr_Private_mat2v2float Private
         %21 = OpConstantComposite %v2float %float_1 %float_1
         %22 = OpConstantComposite %mat2v2float %21 %21
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %32 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %42 = OpConstantComposite %v2float %float_0 %float_1
         %43 = OpConstantComposite %v2float %float_2 %float_3
         %44 = OpConstantComposite %mat2v2float %42 %43
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Private_v2float = OpTypePointer Private %v2float
%_entrypoint_v = OpFunction %void None %25
         %26 = OpLabel
         %29 = OpVariable %_ptr_Function_v2float Function
               OpStore %29 %28
         %31 = OpFunctionCall %v4float %main %29
               OpStore %sk_FragColor %31
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %32
         %33 = OpFunctionParameter %_ptr_Function_v2float
         %34 = OpLabel
 %localArray = OpVariable %_ptr_Function__arr_float_int_2 Function
%localMatrix = OpVariable %_ptr_Function_mat2v2float Function
               OpStore %zero %int_0
         %16 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_1
               OpStore %globalArray %16
               OpStore %globalMatrix %22
         %37 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_1
               OpStore %localArray %37
               OpStore %localMatrix %44
         %45 = OpLoad %int %zero
         %46 = OpAccessChain %_ptr_Private_float %globalArray %45
         %48 = OpLoad %float %46
         %49 = OpLoad %int %zero
         %50 = OpAccessChain %_ptr_Function_float %localArray %49
         %52 = OpLoad %float %50
         %53 = OpFMul %float %48 %52
         %54 = OpLoad %int %zero
         %55 = OpVectorExtractDynamic %float %21 %54
         %56 = OpLoad %int %zero
         %57 = OpVectorExtractDynamic %float %21 %56
         %58 = OpFMul %float %55 %57
         %59 = OpLoad %int %zero
         %60 = OpAccessChain %_ptr_Private_v2float %globalMatrix %59
         %62 = OpLoad %v2float %60
         %63 = OpLoad %int %zero
         %64 = OpAccessChain %_ptr_Function_v2float %localMatrix %63
         %65 = OpLoad %v2float %64
         %66 = OpFMul %v2float %62 %65
         %67 = OpCompositeExtract %float %66 0
         %68 = OpCompositeExtract %float %66 1
         %69 = OpCompositeConstruct %v4float %53 %58 %67 %68
               OpReturnValue %69
               OpFunctionEnd
