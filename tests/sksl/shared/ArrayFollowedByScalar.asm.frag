               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %main "main"                      ; id %6
               OpName %rgb "rgb"                        ; id %24
               OpName %a "a"                            ; id %28

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %rgb RelaxedPrecision
               OpDecorate %_arr_float_int_3 ArrayStride 16
               OpDecorate %a RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_3 = OpConstant %int 3
%_arr_float_int_3 = OpTypeArray %float %int_3       ; ArrayStride 16
%_ptr_Function__arr_float_int_3 = OpTypePointer Function %_arr_float_int_3
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %13

         %14 = OpLabel
         %18 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %18 %17
         %20 =   OpFunctionCall %v4float %main %18
                 OpStore %sk_FragColor %20
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %21         ; RelaxedPrecision
         %22 = OpFunctionParameter %_ptr_Function_v2float

         %23 = OpLabel
        %rgb =   OpVariable %_ptr_Function__arr_float_int_3 Function    ; RelaxedPrecision
          %a =   OpVariable %_ptr_Function_float Function               ; RelaxedPrecision
         %31 =   OpAccessChain %_ptr_Function_float %rgb %int_0
                 OpStore %31 %float_0
         %34 =   OpAccessChain %_ptr_Function_float %rgb %int_1
                 OpStore %34 %float_1
         %36 =   OpAccessChain %_ptr_Function_float %rgb %int_2
                 OpStore %36 %float_0
                 OpStore %a %float_1
         %37 =   OpAccessChain %_ptr_Function_float %rgb %int_0
         %38 =   OpLoad %float %37                  ; RelaxedPrecision
         %39 =   OpAccessChain %_ptr_Function_float %rgb %int_1
         %40 =   OpLoad %float %39                  ; RelaxedPrecision
         %41 =   OpAccessChain %_ptr_Function_float %rgb %int_2
         %42 =   OpLoad %float %41                  ; RelaxedPrecision
         %43 =   OpCompositeConstruct %v4float %38 %40 %42 %float_1     ; RelaxedPrecision
                 OpReturnValue %43
               OpFunctionEnd
