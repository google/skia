               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpMemberName %_UniformBuffer 3 "u_skRTFlip"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %expected "expected"              ; id %28

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
               OpMemberDecorate %_UniformBuffer 3 Offset 16384
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v2float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %30 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
        %114 = OpConstantComposite %v2float %float_1 %float_1
        %127 = OpConstantComposite %v2float %float_0 %float_1
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %25         ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_v2float

         %27 = OpLabel
   %expected =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %131 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %30
         %34 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %37 =   OpLoad %v4float %34                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 0    ; RelaxedPrecision
         %33 =   OpDPdy %float %38                  ; RelaxedPrecision
         %40 =   OpAccessChain %_ptr_Uniform_v2float %12 %int_3
         %42 =   OpLoad %v2float %40
         %43 =   OpCompositeExtract %float %42 1
         %44 =   OpFMul %float %33 %43              ; RelaxedPrecision
         %45 =   OpFOrdEqual %bool %44 %float_0
                 OpSelectionMerge %47 None
                 OpBranchConditional %45 %46 %47

         %46 =     OpLabel
         %49 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %50 =       OpLoad %v4float %49            ; RelaxedPrecision
         %51 =       OpVectorShuffle %v2float %50 %50 0 1   ; RelaxedPrecision
         %48 =       OpDPdy %v2float %51                    ; RelaxedPrecision
         %52 =       OpAccessChain %_ptr_Uniform_v2float %12 %int_3
         %53 =       OpLoad %v2float %52
         %54 =       OpVectorShuffle %v2float %53 %53 1 1
         %55 =       OpFMul %v2float %48 %54        ; RelaxedPrecision
         %56 =       OpVectorShuffle %v2float %30 %30 0 1   ; RelaxedPrecision
         %57 =       OpFOrdEqual %v2bool %55 %56
         %59 =       OpAll %bool %57
                     OpBranch %47

         %47 = OpLabel
         %60 =   OpPhi %bool %false %27 %59 %46
                 OpSelectionMerge %62 None
                 OpBranchConditional %60 %61 %62

         %61 =     OpLabel
         %64 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %65 =       OpLoad %v4float %64            ; RelaxedPrecision
         %66 =       OpVectorShuffle %v3float %65 %65 0 1 2     ; RelaxedPrecision
         %63 =       OpDPdy %v3float %66                        ; RelaxedPrecision
         %68 =       OpAccessChain %_ptr_Uniform_v2float %12 %int_3
         %69 =       OpLoad %v2float %68
         %70 =       OpVectorShuffle %v3float %69 %69 1 1 1
         %71 =       OpFMul %v3float %63 %70        ; RelaxedPrecision
         %72 =       OpVectorShuffle %v3float %30 %30 0 1 2     ; RelaxedPrecision
         %73 =       OpFOrdEqual %v3bool %71 %72
         %75 =       OpAll %bool %73
                     OpBranch %62

         %62 = OpLabel
         %76 =   OpPhi %bool %false %47 %75 %61
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %80 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %81 =       OpLoad %v4float %80            ; RelaxedPrecision
         %79 =       OpDPdy %v4float %81            ; RelaxedPrecision
         %82 =       OpAccessChain %_ptr_Uniform_v2float %12 %int_3
         %83 =       OpLoad %v2float %82
         %84 =       OpVectorShuffle %v4float %83 %83 1 1 1 1
         %85 =       OpFMul %v4float %79 %84        ; RelaxedPrecision
         %86 =       OpFOrdEqual %v4bool %85 %30
         %88 =       OpAll %bool %86
                     OpBranch %78

         %78 = OpLabel
         %89 =   OpPhi %bool %false %62 %88 %77
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
         %94 =       OpLoad %v2float %26
         %95 =       OpVectorShuffle %v2float %94 %94 0 0
         %93 =       OpDPdy %v2float %95
         %96 =       OpAccessChain %_ptr_Uniform_v2float %12 %int_3
         %97 =       OpLoad %v2float %96
         %98 =       OpVectorShuffle %v2float %97 %97 1 1
         %99 =       OpFMul %v2float %93 %98
         %92 =       OpExtInst %v2float %5 FSign %99
        %100 =       OpFOrdEqual %v2bool %92 %21
        %101 =       OpAll %bool %100
                     OpBranch %91

         %91 = OpLabel
        %102 =   OpPhi %bool %false %78 %101 %90
                 OpSelectionMerge %104 None
                 OpBranchConditional %102 %103 %104

        %103 =     OpLabel
        %107 =       OpLoad %v2float %26
        %108 =       OpVectorShuffle %v2float %107 %107 1 1
        %106 =       OpDPdy %v2float %108
        %109 =       OpAccessChain %_ptr_Uniform_v2float %12 %int_3
        %110 =       OpLoad %v2float %109
        %111 =       OpVectorShuffle %v2float %110 %110 1 1
        %112 =       OpFMul %v2float %106 %111
        %105 =       OpExtInst %v2float %5 FSign %112
        %115 =       OpFOrdEqual %v2bool %105 %114
        %116 =       OpAll %bool %115
                     OpBranch %104

        %104 = OpLabel
        %117 =   OpPhi %bool %false %91 %116 %103
                 OpSelectionMerge %119 None
                 OpBranchConditional %117 %118 %119

        %118 =     OpLabel
        %122 =       OpLoad %v2float %26
        %121 =       OpDPdy %v2float %122
        %123 =       OpAccessChain %_ptr_Uniform_v2float %12 %int_3
        %124 =       OpLoad %v2float %123
        %125 =       OpVectorShuffle %v2float %124 %124 1 1
        %126 =       OpFMul %v2float %121 %125
        %120 =       OpExtInst %v2float %5 FSign %126
        %128 =       OpFOrdEqual %v2bool %120 %127
        %129 =       OpAll %bool %128
                     OpBranch %119

        %119 = OpLabel
        %130 =   OpPhi %bool %false %104 %129 %118
                 OpSelectionMerge %134 None
                 OpBranchConditional %130 %132 %133

        %132 =     OpLabel
        %135 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %137 =       OpLoad %v4float %135           ; RelaxedPrecision
                     OpStore %131 %137
                     OpBranch %134

        %133 =     OpLabel
        %138 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_2
        %140 =       OpLoad %v4float %138           ; RelaxedPrecision
                     OpStore %131 %140
                     OpBranch %134

        %134 = OpLabel
        %141 =   OpLoad %v4float %131               ; RelaxedPrecision
                 OpReturnValue %141
               OpFunctionEnd
