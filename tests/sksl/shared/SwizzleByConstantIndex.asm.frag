               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %_0_v "_0_v"                      ; id %27
               OpName %_1_x "_1_x"                      ; id %33
               OpName %_2_y "_2_y"                      ; id %36
               OpName %_3_z "_3_z"                      ; id %38
               OpName %_4_w "_4_w"                      ; id %40
               OpName %a "a"                            ; id %42
               OpName %_9_x "_9_x"                      ; id %44
               OpName %_10_y "_10_y"                    ; id %48
               OpName %_11_z "_11_z"                    ; id %52
               OpName %_12_w "_12_w"                    ; id %56
               OpName %b "b"                            ; id %60
               OpName %c "c"                            ; id %62

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %_0_v RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %_1_x RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %_2_y RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %_3_z RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %_4_w RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %_9_x RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %_10_y RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %_11_z RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %_12_w RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %66 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %72 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
       %_0_v =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
       %_1_x =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
       %_2_y =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
       %_3_z =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
       %_4_w =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
       %_9_x =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
      %_10_y =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
      %_11_z =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
      %_12_w =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %c =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %85 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
                 OpStore %_0_v %32
         %35 =   OpCompositeExtract %float %32 0    ; RelaxedPrecision
                 OpStore %_1_x %35
         %37 =   OpCompositeExtract %float %32 1    ; RelaxedPrecision
                 OpStore %_2_y %37
         %39 =   OpCompositeExtract %float %32 2    ; RelaxedPrecision
                 OpStore %_3_z %39
         %41 =   OpCompositeExtract %float %32 3    ; RelaxedPrecision
                 OpStore %_4_w %41
         %43 =   OpCompositeConstruct %v4float %35 %37 %39 %41  ; RelaxedPrecision
                 OpStore %a %43
         %45 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %46 =   OpLoad %v4float %45                ; RelaxedPrecision
         %47 =   OpCompositeExtract %float %46 0    ; RelaxedPrecision
                 OpStore %_9_x %47
         %49 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 1    ; RelaxedPrecision
                 OpStore %_10_y %51
         %53 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %54 =   OpLoad %v4float %53                ; RelaxedPrecision
         %55 =   OpCompositeExtract %float %54 2    ; RelaxedPrecision
                 OpStore %_11_z %55
         %57 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %58 =   OpLoad %v4float %57                ; RelaxedPrecision
         %59 =   OpCompositeExtract %float %58 3    ; RelaxedPrecision
                 OpStore %_12_w %59
         %61 =   OpCompositeConstruct %v4float %47 %51 %55 %59  ; RelaxedPrecision
                 OpStore %b %61
                 OpStore %c %66
         %73 =   OpFOrdEqual %v4bool %43 %72
         %75 =   OpAll %bool %73
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %78 =       OpFOrdEqual %v4bool %61 %72
         %79 =       OpAll %bool %78
                     OpBranch %77

         %77 = OpLabel
         %80 =   OpPhi %bool %false %26 %79 %76
                 OpSelectionMerge %82 None
                 OpBranchConditional %80 %81 %82

         %81 =     OpLabel
                     OpBranch %82

         %82 = OpLabel
         %84 =   OpPhi %bool %false %77 %true %81
                 OpSelectionMerge %88 None
                 OpBranchConditional %84 %86 %87

         %86 =     OpLabel
         %89 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %91 =       OpLoad %v4float %89            ; RelaxedPrecision
                     OpStore %85 %91
                     OpBranch %88

         %87 =     OpLabel
         %92 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %94 =       OpLoad %v4float %92            ; RelaxedPrecision
                     OpStore %85 %94
                     OpBranch %88

         %88 = OpLabel
         %95 =   OpLoad %v4float %85                ; RelaxedPrecision
                 OpReturnValue %95
               OpFunctionEnd
