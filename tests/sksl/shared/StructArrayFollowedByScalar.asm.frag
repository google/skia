               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %main "main"                      ; id %6
               OpName %S "S"                            ; id %27
               OpMemberName %S 0 "rgb"
               OpMemberName %S 1 "a"
               OpName %s "s"                        ; id %24

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_3 ArrayStride 16
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RelaxedPrecision
               OpMemberDecorate %S 1 Offset 48
               OpMemberDecorate %S 1 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision

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
          %S = OpTypeStruct %_arr_float_int_3 %float
%_ptr_Function_S = OpTypePointer Function %S
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
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
          %s =   OpVariable %_ptr_Function_S Function
         %30 =   OpAccessChain %_ptr_Function_float %s %int_0 %int_0
                 OpStore %30 %float_0
         %34 =   OpAccessChain %_ptr_Function_float %s %int_0 %int_1
                 OpStore %34 %float_1
         %36 =   OpAccessChain %_ptr_Function_float %s %int_0 %int_2
                 OpStore %36 %float_0
         %37 =   OpAccessChain %_ptr_Function_float %s %int_1
                 OpStore %37 %float_1
         %38 =   OpAccessChain %_ptr_Function_float %s %int_0 %int_0
         %39 =   OpLoad %float %38                  ; RelaxedPrecision
         %40 =   OpAccessChain %_ptr_Function_float %s %int_0 %int_1
         %41 =   OpLoad %float %40                  ; RelaxedPrecision
         %42 =   OpAccessChain %_ptr_Function_float %s %int_0 %int_2
         %43 =   OpLoad %float %42                  ; RelaxedPrecision
         %44 =   OpAccessChain %_ptr_Function_float %s %int_1
         %45 =   OpLoad %float %44                  ; RelaxedPrecision
         %46 =   OpCompositeConstruct %v4float %39 %41 %43 %45  ; RelaxedPrecision
                 OpReturnValue %46
               OpFunctionEnd
