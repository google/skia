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
               OpName %expected "expected"              ; id %27

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
               OpDecorate %expected RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision

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
   %float_n1 = OpConstant %float -1
    %float_1 = OpConstant %float 1
         %31 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %79 = OpConstantComposite %v2float %float_n1 %float_0
         %86 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
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
   %expected =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %94 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %31
         %35 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %38 =   OpLoad %v4float %35                ; RelaxedPrecision
         %39 =   OpCompositeExtract %float %38 0    ; RelaxedPrecision
         %34 =   OpExtInst %float %5 FSign %39      ; RelaxedPrecision
         %40 =   OpFOrdEqual %bool %34 %float_n1
                 OpSelectionMerge %42 None
                 OpBranchConditional %40 %41 %42

         %41 =     OpLabel
         %44 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %45 =       OpLoad %v4float %44            ; RelaxedPrecision
         %46 =       OpVectorShuffle %v2float %45 %45 0 1   ; RelaxedPrecision
         %43 =       OpExtInst %v2float %5 FSign %46        ; RelaxedPrecision
         %47 =       OpVectorShuffle %v2float %31 %31 0 1   ; RelaxedPrecision
         %48 =       OpFOrdEqual %v2bool %43 %47
         %50 =       OpAll %bool %48
                     OpBranch %42

         %42 = OpLabel
         %51 =   OpPhi %bool %false %26 %50 %41
                 OpSelectionMerge %53 None
                 OpBranchConditional %51 %52 %53

         %52 =     OpLabel
         %55 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %56 =       OpLoad %v4float %55            ; RelaxedPrecision
         %57 =       OpVectorShuffle %v3float %56 %56 0 1 2     ; RelaxedPrecision
         %54 =       OpExtInst %v3float %5 FSign %57            ; RelaxedPrecision
         %59 =       OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
         %60 =       OpFOrdEqual %v3bool %54 %59
         %62 =       OpAll %bool %60
                     OpBranch %53

         %53 = OpLabel
         %63 =   OpPhi %bool %false %42 %62 %52
                 OpSelectionMerge %65 None
                 OpBranchConditional %63 %64 %65

         %64 =     OpLabel
         %67 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %68 =       OpLoad %v4float %67            ; RelaxedPrecision
         %66 =       OpExtInst %v4float %5 FSign %68    ; RelaxedPrecision
         %69 =       OpFOrdEqual %v4bool %66 %31
         %71 =       OpAll %bool %69
                     OpBranch %65

         %65 = OpLabel
         %72 =   OpPhi %bool %false %53 %71 %64
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
                     OpBranch %74

         %74 = OpLabel
         %76 =   OpPhi %bool %false %65 %true %73
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %80 =       OpVectorShuffle %v2float %31 %31 0 1   ; RelaxedPrecision
         %81 =       OpFOrdEqual %v2bool %79 %80
         %82 =       OpAll %bool %81
                     OpBranch %78

         %78 = OpLabel
         %83 =   OpPhi %bool %false %74 %82 %77
                 OpSelectionMerge %85 None
                 OpBranchConditional %83 %84 %85

         %84 =     OpLabel
         %87 =       OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
         %88 =       OpFOrdEqual %v3bool %86 %87
         %89 =       OpAll %bool %88
                     OpBranch %85

         %85 = OpLabel
         %90 =   OpPhi %bool %false %78 %89 %84
                 OpSelectionMerge %92 None
                 OpBranchConditional %90 %91 %92

         %91 =     OpLabel
                     OpBranch %92

         %92 = OpLabel
         %93 =   OpPhi %bool %false %85 %true %91
                 OpSelectionMerge %97 None
                 OpBranchConditional %93 %95 %96

         %95 =     OpLabel
         %98 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %100 =       OpLoad %v4float %98            ; RelaxedPrecision
                     OpStore %94 %100
                     OpBranch %97

         %96 =     OpLabel
        %101 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %103 =       OpLoad %v4float %101           ; RelaxedPrecision
                     OpStore %94 %103
                     OpBranch %97

         %97 = OpLabel
        %104 =   OpLoad %v4float %94                ; RelaxedPrecision
                 OpReturnValue %104
               OpFunctionEnd
