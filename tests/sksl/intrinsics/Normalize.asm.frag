               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "inputVal"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %expectedVec "expectedVec"        ; id %27

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
               OpDecorate %expectedVec RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision

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
    %float_1 = OpConstant %float 1
         %30 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %78 = OpConstantComposite %v2float %float_0 %float_1
         %85 = OpConstantComposite %v3float %float_0 %float_1 %float_0
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
%expectedVec =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %93 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expectedVec %30
         %34 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %37 =   OpLoad %v4float %34                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 0    ; RelaxedPrecision
         %33 =   OpExtInst %float %5 Normalize %38  ; RelaxedPrecision
         %39 =   OpFOrdEqual %bool %33 %float_1
                 OpSelectionMerge %41 None
                 OpBranchConditional %39 %40 %41

         %40 =     OpLabel
         %43 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %44 =       OpLoad %v4float %43            ; RelaxedPrecision
         %45 =       OpVectorShuffle %v2float %44 %44 0 1   ; RelaxedPrecision
         %42 =       OpExtInst %v2float %5 Normalize %45    ; RelaxedPrecision
         %46 =       OpVectorShuffle %v2float %30 %30 0 1   ; RelaxedPrecision
         %47 =       OpFOrdEqual %v2bool %42 %46
         %49 =       OpAll %bool %47
                     OpBranch %41

         %41 = OpLabel
         %50 =   OpPhi %bool %false %26 %49 %40
                 OpSelectionMerge %52 None
                 OpBranchConditional %50 %51 %52

         %51 =     OpLabel
         %54 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %55 =       OpLoad %v4float %54            ; RelaxedPrecision
         %56 =       OpVectorShuffle %v3float %55 %55 0 1 2     ; RelaxedPrecision
         %53 =       OpExtInst %v3float %5 Normalize %56        ; RelaxedPrecision
         %58 =       OpVectorShuffle %v3float %30 %30 0 1 2     ; RelaxedPrecision
         %59 =       OpFOrdEqual %v3bool %53 %58
         %61 =       OpAll %bool %59
                     OpBranch %52

         %52 = OpLabel
         %62 =   OpPhi %bool %false %41 %61 %51
                 OpSelectionMerge %64 None
                 OpBranchConditional %62 %63 %64

         %63 =     OpLabel
         %66 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %67 =       OpLoad %v4float %66            ; RelaxedPrecision
         %65 =       OpExtInst %v4float %5 Normalize %67    ; RelaxedPrecision
         %68 =       OpFOrdEqual %v4bool %65 %30
         %70 =       OpAll %bool %68
                     OpBranch %64

         %64 = OpLabel
         %71 =   OpPhi %bool %false %52 %70 %63
                 OpSelectionMerge %73 None
                 OpBranchConditional %71 %72 %73

         %72 =     OpLabel
                     OpBranch %73

         %73 = OpLabel
         %75 =   OpPhi %bool %false %64 %true %72
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %79 =       OpVectorShuffle %v2float %30 %30 1 0   ; RelaxedPrecision
         %80 =       OpFOrdEqual %v2bool %78 %79
         %81 =       OpAll %bool %80
                     OpBranch %77

         %77 = OpLabel
         %82 =   OpPhi %bool %false %73 %81 %76
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
         %86 =       OpVectorShuffle %v3float %30 %30 2 0 1     ; RelaxedPrecision
         %87 =       OpFOrdEqual %v3bool %85 %86
         %88 =       OpAll %bool %87
                     OpBranch %84

         %84 = OpLabel
         %89 =   OpPhi %bool %false %77 %88 %83
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
                     OpBranch %91

         %91 = OpLabel
         %92 =   OpPhi %bool %false %84 %true %90
                 OpSelectionMerge %96 None
                 OpBranchConditional %92 %94 %95

         %94 =     OpLabel
         %97 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %99 =       OpLoad %v4float %97            ; RelaxedPrecision
                     OpStore %93 %99
                     OpBranch %96

         %95 =     OpLabel
        %100 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %102 =       OpLoad %v4float %100           ; RelaxedPrecision
                     OpStore %93 %102
                     OpBranch %96

         %96 = OpLabel
        %103 =   OpLoad %v4float %93                ; RelaxedPrecision
                 OpReturnValue %103
               OpFunctionEnd
