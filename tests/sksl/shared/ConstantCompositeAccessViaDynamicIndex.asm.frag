               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %zero "zero"
               OpName %globalArray "globalArray"
               OpName %globalMatrix "globalMatrix"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %localArray "localArray"
               OpName %localMatrix "localMatrix"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %globalArray RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %19 RelaxedPrecision
               OpDecorate %globalMatrix RelaxedPrecision
               OpDecorate %localArray RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %localMatrix RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
         %24 = OpConstantComposite %v2float %float_1 %float_1
         %25 = OpConstantComposite %mat2v2float %24 %24
       %void = OpTypeVoid
         %28 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %35 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %45 = OpConstantComposite %v2float %float_0 %float_1
         %46 = OpConstantComposite %v2float %float_2 %float_3
         %47 = OpConstantComposite %mat2v2float %45 %46
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Private_v2float = OpTypePointer Private %v2float
%_entrypoint_v = OpFunction %void None %28
         %29 = OpLabel
         %32 = OpVariable %_ptr_Function_v2float Function
               OpStore %32 %31
         %34 = OpFunctionCall %v4float %main %32
               OpStore %sk_FragColor %34
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %35
         %36 = OpFunctionParameter %_ptr_Function_v2float
         %37 = OpLabel
 %localArray = OpVariable %_ptr_Function__arr_float_int_2 Function
%localMatrix = OpVariable %_ptr_Function_mat2v2float Function
               OpStore %zero %int_0
         %19 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_1
               OpStore %globalArray %19
               OpStore %globalMatrix %25
         %40 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_1
               OpStore %localArray %40
               OpStore %localMatrix %47
         %48 = OpLoad %int %zero
         %49 = OpAccessChain %_ptr_Private_float %globalArray %48
         %51 = OpLoad %float %49
         %52 = OpLoad %int %zero
         %53 = OpAccessChain %_ptr_Function_float %localArray %52
         %55 = OpLoad %float %53
         %56 = OpFMul %float %51 %55
         %57 = OpLoad %int %zero
         %58 = OpVectorExtractDynamic %float %24 %57
         %59 = OpLoad %int %zero
         %60 = OpVectorExtractDynamic %float %24 %59
         %61 = OpFMul %float %58 %60
         %62 = OpLoad %int %zero
         %63 = OpAccessChain %_ptr_Private_v2float %globalMatrix %62
         %65 = OpLoad %v2float %63
         %66 = OpLoad %int %zero
         %67 = OpAccessChain %_ptr_Function_v2float %localMatrix %66
         %68 = OpLoad %v2float %67
         %69 = OpFMul %v2float %65 %68
         %70 = OpCompositeExtract %float %69 0
         %71 = OpCompositeExtract %float %69 1
         %72 = OpCompositeConstruct %v4float %56 %61 %70 %71
               OpReturnValue %72
               OpFunctionEnd
