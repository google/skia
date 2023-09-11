               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %S "S"
               OpMemberName %S 0 "v"
               OpName %initialize_vS "initialize_vS"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpName %z "z"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_arr_S_int_2 ArrayStride 16
               OpDecorate %_arr_v2float_int_2 ArrayStride 16
               OpDecorate %96 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %14 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
          %S = OpTypeStruct %v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_S_int_2 = OpTypeArray %S %int_2
%_ptr_Function__arr_S_int_2 = OpTypePointer Function %_arr_S_int_2
         %23 = OpTypeFunction %void %_ptr_Function__arr_S_int_2
    %float_1 = OpConstant %float 1
         %27 = OpConstantComposite %v2float %float_0 %float_1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
         %31 = OpConstantComposite %v2float %float_2 %float_1
      %int_1 = OpConstant %int 1
         %34 = OpTypeFunction %v4float %_ptr_Function_v2float
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
         %41 = OpConstantComposite %v2float %float_1 %float_0
   %float_n1 = OpConstant %float -1
         %46 = OpConstantComposite %v2float %float_n1 %float_2
%_entrypoint_v = OpFunction %void None %10
         %11 = OpLabel
         %15 = OpVariable %_ptr_Function_v2float Function
               OpStore %15 %14
         %17 = OpFunctionCall %v4float %main %15
               OpStore %sk_FragColor %17
               OpReturn
               OpFunctionEnd
%initialize_vS = OpFunction %void None %23
         %24 = OpFunctionParameter %_ptr_Function__arr_S_int_2
         %25 = OpLabel
         %29 = OpAccessChain %_ptr_Function_v2float %24 %int_0 %int_0
               OpStore %29 %27
         %33 = OpAccessChain %_ptr_Function_v2float %24 %int_1 %int_0
               OpStore %33 %31
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %34
         %35 = OpFunctionParameter %_ptr_Function_v2float
         %36 = OpLabel
          %x = OpVariable %_ptr_Function__arr_v2float_int_2 Function
          %y = OpVariable %_ptr_Function__arr_v2float_int_2 Function
          %z = OpVariable %_ptr_Function__arr_S_int_2 Function
         %49 = OpVariable %_ptr_Function__arr_S_int_2 Function
         %40 = OpAccessChain %_ptr_Function_v2float %x %int_0
               OpStore %40 %14
         %42 = OpAccessChain %_ptr_Function_v2float %x %int_1
               OpStore %42 %41
         %44 = OpAccessChain %_ptr_Function_v2float %y %int_0
               OpStore %44 %27
         %47 = OpAccessChain %_ptr_Function_v2float %y %int_1
               OpStore %47 %46
         %50 = OpFunctionCall %void %initialize_vS %49
         %51 = OpLoad %_arr_S_int_2 %49
               OpStore %z %51
         %52 = OpAccessChain %_ptr_Function_v2float %x %int_0
         %53 = OpLoad %v2float %52
         %54 = OpCompositeExtract %float %53 0
         %55 = OpAccessChain %_ptr_Function_v2float %x %int_0
         %56 = OpLoad %v2float %55
         %57 = OpCompositeExtract %float %56 1
         %58 = OpFMul %float %54 %57
         %59 = OpAccessChain %_ptr_Function_v2float %z %int_0 %int_0
         %60 = OpLoad %v2float %59
         %61 = OpCompositeExtract %float %60 0
         %62 = OpFAdd %float %58 %61
         %63 = OpAccessChain %_ptr_Function_v2float %x %int_1
         %64 = OpLoad %v2float %63
         %65 = OpCompositeExtract %float %64 0
         %66 = OpAccessChain %_ptr_Function_v2float %x %int_1
         %67 = OpLoad %v2float %66
         %68 = OpCompositeExtract %float %67 1
         %69 = OpAccessChain %_ptr_Function_v2float %z %int_0 %int_0
         %70 = OpLoad %v2float %69
         %71 = OpCompositeExtract %float %70 1
         %72 = OpFMul %float %68 %71
         %73 = OpFSub %float %65 %72
         %74 = OpAccessChain %_ptr_Function_v2float %y %int_0
         %75 = OpLoad %v2float %74
         %76 = OpCompositeExtract %float %75 0
         %77 = OpAccessChain %_ptr_Function_v2float %y %int_0
         %78 = OpLoad %v2float %77
         %79 = OpCompositeExtract %float %78 1
         %80 = OpFDiv %float %76 %79
         %81 = OpAccessChain %_ptr_Function_v2float %z %int_1 %int_0
         %82 = OpLoad %v2float %81
         %83 = OpCompositeExtract %float %82 0
         %84 = OpFDiv %float %80 %83
         %85 = OpAccessChain %_ptr_Function_v2float %y %int_1
         %86 = OpLoad %v2float %85
         %87 = OpCompositeExtract %float %86 0
         %88 = OpAccessChain %_ptr_Function_v2float %y %int_1
         %89 = OpLoad %v2float %88
         %90 = OpCompositeExtract %float %89 1
         %91 = OpAccessChain %_ptr_Function_v2float %z %int_1 %int_0
         %92 = OpLoad %v2float %91
         %93 = OpCompositeExtract %float %92 1
         %94 = OpFMul %float %90 %93
         %95 = OpFAdd %float %87 %94
         %96 = OpCompositeConstruct %v4float %62 %73 %84 %95
               OpReturnValue %96
               OpFunctionEnd
