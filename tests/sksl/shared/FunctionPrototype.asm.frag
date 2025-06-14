               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %this_function_is_prototyped_after_its_definition_h4h4 "this_function_is_prototyped_after_its_definition_h4h4"    ; id %6
               OpName %this_function_is_defined_before_use_h4h4 "this_function_is_defined_before_use_h4h4"                              ; id %7
               OpName %main "main"                                                                                                      ; id %8

               ; Annotations
               OpDecorate %this_function_is_prototyped_after_its_definition_h4h4 RelaxedPrecision
               OpDecorate %this_function_is_defined_before_use_h4h4 RelaxedPrecision
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %27 = OpTypeFunction %v4float %_ptr_Function_v4float
         %39 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function this_function_is_prototyped_after_its_definition_h4h4
%this_function_is_prototyped_after_its_definition_h4h4 = OpFunction %v4float None %27   ; RelaxedPrecision
         %28 = OpFunctionParameter %_ptr_Function_v4float                               ; RelaxedPrecision

         %29 = OpLabel
         %30 =   OpLoad %v4float %28                ; RelaxedPrecision
         %31 =   OpFNegate %v4float %30             ; RelaxedPrecision
                 OpReturnValue %31
               OpFunctionEnd


               ; Function this_function_is_defined_before_use_h4h4
%this_function_is_defined_before_use_h4h4 = OpFunction %v4float None %27    ; RelaxedPrecision
         %32 = OpFunctionParameter %_ptr_Function_v4float                   ; RelaxedPrecision

         %33 = OpLabel
         %36 =   OpVariable %_ptr_Function_v4float Function
         %34 =   OpLoad %v4float %32                ; RelaxedPrecision
         %35 =   OpFNegate %v4float %34             ; RelaxedPrecision
                 OpStore %36 %35
         %37 =   OpFunctionCall %v4float %this_function_is_prototyped_after_its_definition_h4h4 %36
         %38 =   OpFNegate %v4float %37             ; RelaxedPrecision
                 OpReturnValue %38
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %39         ; RelaxedPrecision
         %40 = OpFunctionParameter %_ptr_Function_v2float

         %41 = OpLabel
         %47 =   OpVariable %_ptr_Function_v4float Function
         %42 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %45 =   OpLoad %v4float %42                ; RelaxedPrecision
         %46 =   OpFNegate %v4float %45             ; RelaxedPrecision
                 OpStore %47 %46
         %48 =   OpFunctionCall %v4float %this_function_is_defined_before_use_h4h4 %47
                 OpReturnValue %48
               OpFunctionEnd
