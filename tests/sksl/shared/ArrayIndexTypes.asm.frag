               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %main "main"                  ; id %6
               OpName %array "array"                ; id %14
               OpName %x "x"                        ; id %23
               OpName %y "y"                        ; id %26
               OpName %z "z"                        ; id %30
               OpName %w "w"                        ; id %32

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_4 ArrayStride 16
               OpDecorate %x RelaxedPrecision
               OpDecorate %y RelaxedPrecision
               OpDecorate %43 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
      %int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4       ; ArrayStride 16
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_1 = OpConstant %uint 1
      %int_2 = OpConstant %int 2
     %uint_3 = OpConstant %uint 3
%_ptr_Function_float = OpTypePointer Function %float


               ; Function main
       %main = OpFunction %void None %12

         %13 = OpLabel
      %array =   OpVariable %_ptr_Function__arr_float_int_4 Function
          %x =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
          %y =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
          %z =   OpVariable %_ptr_Function_int Function
          %w =   OpVariable %_ptr_Function_uint Function
         %22 =   OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
                 OpStore %array %22
                 OpStore %x %int_0
                 OpStore %y %uint_1
                 OpStore %z %int_2
                 OpStore %w %uint_3
         %34 =   OpAccessChain %_ptr_Function_float %array %int_0
         %36 =   OpLoad %float %34
         %37 =   OpAccessChain %_ptr_Function_float %array %uint_1
         %38 =   OpLoad %float %37
         %39 =   OpAccessChain %_ptr_Function_float %array %int_2
         %40 =   OpLoad %float %39
         %41 =   OpAccessChain %_ptr_Function_float %array %uint_3
         %42 =   OpLoad %float %41
         %43 =   OpCompositeConstruct %v4float %36 %38 %40 %42  ; RelaxedPrecision
                 OpStore %sk_FragColor %43
                 OpReturn
               OpFunctionEnd
