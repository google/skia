               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %inout_params_are_distinct_bhh "inout_params_are_distinct_bhh"    ; id %6
               OpName %main "main"                                                      ; id %7
               OpName %x "x"                                                            ; id %41

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %x RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
         %27 = OpTypeFunction %bool %_ptr_Function_float %_ptr_Function_float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %38 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function inout_params_are_distinct_bhh
%inout_params_are_distinct_bhh = OpFunction %bool None %27
         %28 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision
         %29 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %30 = OpLabel
                 OpStore %28 %float_1
                 OpStore %29 %float_2
                 OpSelectionMerge %36 None
                 OpBranchConditional %true %35 %36

         %35 =     OpLabel
                     OpBranch %36

         %36 = OpLabel
         %37 =   OpPhi %bool %false %30 %true %35
                 OpReturnValue %37
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %38         ; RelaxedPrecision
         %39 = OpFunctionParameter %_ptr_Function_v2float

         %40 = OpLabel
          %x =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %42 =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %43 =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %47 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %x %float_0
                 OpStore %42 %float_0
                 OpStore %43 %float_0
         %44 =   OpFunctionCall %bool %inout_params_are_distinct_bhh %42 %43
         %45 =   OpLoad %float %42                  ; RelaxedPrecision
                 OpStore %x %45
         %46 =   OpLoad %float %43                  ; RelaxedPrecision
                 OpStore %x %46
                 OpSelectionMerge %51 None
                 OpBranchConditional %44 %49 %50

         %49 =     OpLabel
         %52 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %55 =       OpLoad %v4float %52            ; RelaxedPrecision
                     OpStore %47 %55
                     OpBranch %51

         %50 =     OpLabel
         %56 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %58 =       OpLoad %v4float %56            ; RelaxedPrecision
                     OpStore %47 %58
                     OpBranch %51

         %51 = OpLabel
         %59 =   OpLoad %v4float %47                ; RelaxedPrecision
                 OpReturnValue %59
               OpFunctionEnd
