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

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %78 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision

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
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
 %float_0_75 = OpConstant %float 0.75
         %43 = OpConstantComposite %v2float %float_0_75 %float_0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %55 = OpConstantComposite %v3float %float_0_75 %float_0 %float_0_75
     %v3bool = OpTypeVector %bool 3
 %float_0_25 = OpConstant %float 0.25
         %66 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0_75 %float_0_25
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
         %71 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %33 =   OpLoad %v4float %30
         %34 =   OpCompositeExtract %float %33 0
         %29 =   OpExtInst %float %5 Fract %34
         %36 =   OpFOrdEqual %bool %29 %float_0_75
                 OpSelectionMerge %38 None
                 OpBranchConditional %36 %37 %38

         %37 =     OpLabel
         %40 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %41 =       OpLoad %v4float %40
         %42 =       OpVectorShuffle %v2float %41 %41 0 1
         %39 =       OpExtInst %v2float %5 Fract %42
         %44 =       OpFOrdEqual %v2bool %39 %43
         %46 =       OpAll %bool %44
                     OpBranch %38

         %38 = OpLabel
         %47 =   OpPhi %bool %false %26 %46 %37
                 OpSelectionMerge %49 None
                 OpBranchConditional %47 %48 %49

         %48 =     OpLabel
         %51 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %52 =       OpLoad %v4float %51
         %53 =       OpVectorShuffle %v3float %52 %52 0 1 2
         %50 =       OpExtInst %v3float %5 Fract %53
         %56 =       OpFOrdEqual %v3bool %50 %55
         %58 =       OpAll %bool %56
                     OpBranch %49

         %49 = OpLabel
         %59 =   OpPhi %bool %false %38 %58 %48
                 OpSelectionMerge %61 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
         %63 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %64 =       OpLoad %v4float %63
         %62 =       OpExtInst %v4float %5 Fract %64
         %67 =       OpFOrdEqual %v4bool %62 %66
         %69 =       OpAll %bool %67
                     OpBranch %61

         %61 = OpLabel
         %70 =   OpPhi %bool %false %49 %69 %60
                 OpSelectionMerge %75 None
                 OpBranchConditional %70 %73 %74

         %73 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %78 =       OpLoad %v4float %76            ; RelaxedPrecision
                     OpStore %71 %78
                     OpBranch %75

         %74 =     OpLabel
         %79 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %81 =       OpLoad %v4float %79            ; RelaxedPrecision
                     OpStore %71 %81
                     OpBranch %75

         %75 = OpLabel
         %82 =   OpLoad %v4float %71                ; RelaxedPrecision
                 OpReturnValue %82
               OpFunctionEnd
