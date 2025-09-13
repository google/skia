               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_SampleMask %sk_SampleMaskIn
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %10
               OpName %sk_SampleMask "sk_SampleMask"    ; id %14
               OpName %sk_SampleMaskIn "sk_SampleMaskIn"    ; id %19
               OpName %samplemaskin_as_color_h4 "samplemaskin_as_color_h4"  ; id %6
               OpName %clear_samplemask_v "clear_samplemask_v"              ; id %7
               OpName %reset_samplemask_v "reset_samplemask_v"              ; id %8
               OpName %main "main"                                          ; id %9

               ; Annotations
               OpDecorate %samplemaskin_as_color_h4 RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_uint_int_1 ArrayStride 16
               OpDecorate %sk_SampleMask BuiltIn SampleMask
               OpDecorate %sk_SampleMaskIn BuiltIn SampleMask
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %uint = OpTypeInt 32 0
      %int_1 = OpConstant %int 1
%_arr_uint_int_1 = OpTypeArray %uint %int_1         ; ArrayStride 16
%_ptr_Output__arr_uint_int_1 = OpTypePointer Output %_arr_uint_int_1
%sk_SampleMask = OpVariable %_ptr_Output__arr_uint_int_1 Output     ; BuiltIn SampleMask
%_ptr_Input__arr_uint_int_1 = OpTypePointer Input %_arr_uint_int_1
%sk_SampleMaskIn = OpVariable %_ptr_Input__arr_uint_int_1 Input     ; BuiltIn SampleMask
         %21 = OpTypeFunction %v4float
%_ptr_Input_uint = OpTypePointer Input %uint
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %30 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
%_ptr_Output_uint = OpTypePointer Output %uint
%uint_4294967295 = OpConstant %uint 4294967295
%float_0_00390625 = OpConstant %float 0.00390625


               ; Function samplemaskin_as_color_h4
%samplemaskin_as_color_h4 = OpFunction %v4float None %21    ; RelaxedPrecision

         %22 = OpLabel
         %23 =   OpAccessChain %_ptr_Input_uint %sk_SampleMaskIn %int_0
         %26 =   OpLoad %uint %23
         %27 =   OpConvertUToF %float %26           ; RelaxedPrecision
         %28 =   OpCompositeConstruct %v4float %27 %27 %27 %27  ; RelaxedPrecision
                 OpReturnValue %28
               OpFunctionEnd


               ; Function clear_samplemask_v
%clear_samplemask_v = OpFunction %void None %30

         %31 = OpLabel
         %33 =   OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
                 OpStore %33 %uint_0
                 OpReturn
               OpFunctionEnd


               ; Function reset_samplemask_v
%reset_samplemask_v = OpFunction %void None %30

         %35 = OpLabel
         %36 =   OpAccessChain %_ptr_Input_uint %sk_SampleMaskIn %int_0
         %37 =   OpLoad %uint %36
         %38 =   OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
                 OpStore %38 %37
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %30

         %39 = OpLabel
         %40 =   OpFunctionCall %void %clear_samplemask_v
         %41 =   OpFunctionCall %void %reset_samplemask_v
         %43 =   OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
                 OpStore %43 %uint_4294967295
         %44 =   OpFunctionCall %v4float %samplemaskin_as_color_h4
         %46 =   OpVectorTimesScalar %v4float %44 %float_0_00390625     ; RelaxedPrecision
                 OpStore %sk_FragColor %46
                 OpReturn
               OpFunctionEnd
