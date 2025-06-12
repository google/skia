               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %5
               OpName %_UniformBuffer "_UniformBuffer"  ; id %10
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %12
               OpName %this_function_is_prototyped_after_its_definition_h4h4 "this_function_is_prototyped_after_its_definition_h4h4"    ; id %2
               OpName %this_function_is_defined_before_use_h4h4 "this_function_is_defined_before_use_h4h4"                              ; id %3
               OpName %main "main"                                                                                                      ; id %4

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
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %23 = OpTypeFunction %v4float %_ptr_Function_v4float
         %35 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %14

         %15 = OpLabel
         %19 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %19 %18
         %21 =   OpFunctionCall %v4float %main %19
                 OpStore %sk_FragColor %21
                 OpReturn
               OpFunctionEnd


               ; Function this_function_is_prototyped_after_its_definition_h4h4
%this_function_is_prototyped_after_its_definition_h4h4 = OpFunction %v4float None %23   ; RelaxedPrecision
         %24 = OpFunctionParameter %_ptr_Function_v4float                               ; RelaxedPrecision

         %25 = OpLabel
         %26 =   OpLoad %v4float %24                ; RelaxedPrecision
         %27 =   OpFNegate %v4float %26             ; RelaxedPrecision
                 OpReturnValue %27
               OpFunctionEnd


               ; Function this_function_is_defined_before_use_h4h4
%this_function_is_defined_before_use_h4h4 = OpFunction %v4float None %23    ; RelaxedPrecision
         %28 = OpFunctionParameter %_ptr_Function_v4float                   ; RelaxedPrecision

         %29 = OpLabel
         %32 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpLoad %v4float %28                ; RelaxedPrecision
         %31 =   OpFNegate %v4float %30             ; RelaxedPrecision
                 OpStore %32 %31
         %33 =   OpFunctionCall %v4float %this_function_is_prototyped_after_its_definition_h4h4 %32
         %34 =   OpFNegate %v4float %33             ; RelaxedPrecision
                 OpReturnValue %34
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %35         ; RelaxedPrecision
         %36 = OpFunctionParameter %_ptr_Function_v2float

         %37 = OpLabel
         %44 =   OpVariable %_ptr_Function_v4float Function
         %38 =   OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %42 =   OpLoad %v4float %38                ; RelaxedPrecision
         %43 =   OpFNegate %v4float %42             ; RelaxedPrecision
                 OpStore %44 %43
         %45 =   OpFunctionCall %v4float %this_function_is_defined_before_use_h4h4 %44
                 OpReturnValue %45
               OpFunctionEnd
