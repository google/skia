               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_SampleMask %sk_SampleMaskIn
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %6
               OpName %sk_SampleMask "sk_SampleMask"    ; id %10
               OpName %sk_SampleMaskIn "sk_SampleMaskIn"    ; id %16
               OpName %samplemaskin_as_color_h4 "samplemaskin_as_color_h4"  ; id %2
               OpName %clear_samplemask_v "clear_samplemask_v"              ; id %3
               OpName %reset_samplemask_v "reset_samplemask_v"              ; id %4
               OpName %main "main"                                          ; id %5

               ; Annotations
               OpDecorate %samplemaskin_as_color_h4 RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_uint_int_1 ArrayStride 16
               OpDecorate %sk_SampleMask BuiltIn SampleMask
               OpDecorate %sk_SampleMaskIn BuiltIn SampleMask
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_arr_uint_int_1 = OpTypeArray %uint %int_1         ; ArrayStride 16
%_ptr_Output__arr_uint_int_1 = OpTypePointer Output %_arr_uint_int_1
%sk_SampleMask = OpVariable %_ptr_Output__arr_uint_int_1 Output     ; BuiltIn SampleMask
%_ptr_Input__arr_uint_int_1 = OpTypePointer Input %_arr_uint_int_1
%sk_SampleMaskIn = OpVariable %_ptr_Input__arr_uint_int_1 Input     ; BuiltIn SampleMask
         %18 = OpTypeFunction %v4float
%_ptr_Input_uint = OpTypePointer Input %uint
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
%_ptr_Output_uint = OpTypePointer Output %uint
%uint_4294967295 = OpConstant %uint 4294967295
%float_0_00390625 = OpConstant %float 0.00390625


               ; Function samplemaskin_as_color_h4
%samplemaskin_as_color_h4 = OpFunction %v4float None %18    ; RelaxedPrecision

         %19 = OpLabel
         %20 =   OpAccessChain %_ptr_Input_uint %sk_SampleMaskIn %int_0
         %23 =   OpLoad %uint %20
         %24 =   OpConvertUToF %float %23           ; RelaxedPrecision
         %25 =   OpCompositeConstruct %v4float %24 %24 %24 %24  ; RelaxedPrecision
                 OpReturnValue %25
               OpFunctionEnd


               ; Function clear_samplemask_v
%clear_samplemask_v = OpFunction %void None %27

         %28 = OpLabel
         %30 =   OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
                 OpStore %30 %uint_0
                 OpReturn
               OpFunctionEnd


               ; Function reset_samplemask_v
%reset_samplemask_v = OpFunction %void None %27

         %32 = OpLabel
         %33 =   OpAccessChain %_ptr_Input_uint %sk_SampleMaskIn %int_0
         %34 =   OpLoad %uint %33
         %35 =   OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
                 OpStore %35 %34
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %27

         %36 = OpLabel
         %37 =   OpFunctionCall %void %clear_samplemask_v
         %38 =   OpFunctionCall %void %reset_samplemask_v
         %40 =   OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
                 OpStore %40 %uint_4294967295
         %41 =   OpFunctionCall %v4float %samplemaskin_as_color_h4
         %43 =   OpVectorTimesScalar %v4float %41 %float_0_00390625     ; RelaxedPrecision
                 OpStore %sk_FragColor %43
                 OpReturn
               OpFunctionEnd
