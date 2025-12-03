               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "I"
               OpMemberName %_UniformBuffer 1 "N"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %expectedX "expectedX"            ; id %27
               OpName %expectedXY "expectedXY"          ; id %37
               OpName %expectedXYZ "expectedXYZ"        ; id %41
               OpName %expectedXYZW "expectedXYZW"      ; id %48

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
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %expectedX RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %expectedXY RelaxedPrecision
               OpDecorate %expectedXYZ RelaxedPrecision
               OpDecorate %expectedXYZW RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_65504 = OpConstant %float 65504
  %float_222 = OpConstant %float 222
    %float_2 = OpConstant %float 2
%float_n65504 = OpConstant %float -65504
  %float_n49 = OpConstant %float -49
 %float_n169 = OpConstant %float -169
  %float_202 = OpConstant %float 202
         %40 = OpConstantComposite %v2float %float_n169 %float_202
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
 %float_n379 = OpConstant %float -379
  %float_454 = OpConstant %float 454
 %float_n529 = OpConstant %float -529
         %47 = OpConstantComposite %v3float %float_n379 %float_454 %float_n529
%_ptr_Function_v4float = OpTypePointer Function %v4float
 %float_n699 = OpConstant %float -699
  %float_838 = OpConstant %float 838
 %float_n977 = OpConstant %float -977
 %float_1116 = OpConstant %float 1116
         %54 = OpConstantComposite %v4float %float_n699 %float_838 %float_n977 %float_1116
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3


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
  %expectedX =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
 %expectedXY =   OpVariable %_ptr_Function_v2float Function     ; RelaxedPrecision
%expectedXYZ =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
%expectedXYZW =   OpVariable %_ptr_Function_v4float Function    ; RelaxedPrecision
        %118 =   OpVariable %_ptr_Function_v4float Function
         %32 =   OpFMul %float %float_65504 %float_222  ; RelaxedPrecision
         %34 =   OpFMul %float %32 %float_2             ; RelaxedPrecision
         %29 =   OpExtInst %float %5 Reflect %34 %float_n65504  ; RelaxedPrecision
                 OpStore %expectedX %29
                 OpStore %expectedX %float_n49
                 OpStore %expectedXY %40
                 OpStore %expectedXYZ %47
                 OpStore %expectedXYZW %54
         %58 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %61 =   OpLoad %v4float %58                ; RelaxedPrecision
         %62 =   OpCompositeExtract %float %61 0    ; RelaxedPrecision
         %63 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %65 =   OpLoad %v4float %63                ; RelaxedPrecision
         %66 =   OpCompositeExtract %float %65 0    ; RelaxedPrecision
         %57 =   OpExtInst %float %5 Reflect %62 %66    ; RelaxedPrecision
         %67 =   OpFOrdEqual %bool %57 %float_n49
                 OpSelectionMerge %69 None
                 OpBranchConditional %67 %68 %69

         %68 =     OpLabel
         %71 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %72 =       OpLoad %v4float %71            ; RelaxedPrecision
         %73 =       OpVectorShuffle %v2float %72 %72 0 1   ; RelaxedPrecision
         %74 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %75 =       OpLoad %v4float %74            ; RelaxedPrecision
         %76 =       OpVectorShuffle %v2float %75 %75 0 1   ; RelaxedPrecision
         %70 =       OpExtInst %v2float %5 Reflect %73 %76  ; RelaxedPrecision
         %77 =       OpFOrdEqual %v2bool %70 %40
         %79 =       OpAll %bool %77
                     OpBranch %69

         %69 = OpLabel
         %80 =   OpPhi %bool %false %26 %79 %68
                 OpSelectionMerge %82 None
                 OpBranchConditional %80 %81 %82

         %81 =     OpLabel
         %84 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %85 =       OpLoad %v4float %84            ; RelaxedPrecision
         %86 =       OpVectorShuffle %v3float %85 %85 0 1 2     ; RelaxedPrecision
         %87 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %88 =       OpLoad %v4float %87            ; RelaxedPrecision
         %89 =       OpVectorShuffle %v3float %88 %88 0 1 2     ; RelaxedPrecision
         %83 =       OpExtInst %v3float %5 Reflect %86 %89      ; RelaxedPrecision
         %90 =       OpFOrdEqual %v3bool %83 %47
         %92 =       OpAll %bool %90
                     OpBranch %82

         %82 = OpLabel
         %93 =   OpPhi %bool %false %69 %92 %81
                 OpSelectionMerge %95 None
                 OpBranchConditional %93 %94 %95

         %94 =     OpLabel
         %97 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %98 =       OpLoad %v4float %97            ; RelaxedPrecision
         %99 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %100 =       OpLoad %v4float %99            ; RelaxedPrecision
         %96 =       OpExtInst %v4float %5 Reflect %98 %100     ; RelaxedPrecision
        %101 =       OpFOrdEqual %v4bool %96 %54
        %103 =       OpAll %bool %101
                     OpBranch %95

         %95 = OpLabel
        %104 =   OpPhi %bool %false %82 %103 %94
                 OpSelectionMerge %106 None
                 OpBranchConditional %104 %105 %106

        %105 =     OpLabel
                     OpBranch %106

        %106 = OpLabel
        %108 =   OpPhi %bool %false %95 %true %105
                 OpSelectionMerge %110 None
                 OpBranchConditional %108 %109 %110

        %109 =     OpLabel
                     OpBranch %110

        %110 = OpLabel
        %111 =   OpPhi %bool %false %106 %true %109
                 OpSelectionMerge %113 None
                 OpBranchConditional %111 %112 %113

        %112 =     OpLabel
                     OpBranch %113

        %113 = OpLabel
        %114 =   OpPhi %bool %false %110 %true %112
                 OpSelectionMerge %116 None
                 OpBranchConditional %114 %115 %116

        %115 =     OpLabel
                     OpBranch %116

        %116 = OpLabel
        %117 =   OpPhi %bool %false %113 %true %115
                 OpSelectionMerge %121 None
                 OpBranchConditional %117 %119 %120

        %119 =     OpLabel
        %122 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %124 =       OpLoad %v4float %122           ; RelaxedPrecision
                     OpStore %118 %124
                     OpBranch %121

        %120 =     OpLabel
        %125 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %127 =       OpLoad %v4float %125           ; RelaxedPrecision
                     OpStore %118 %127
                     OpBranch %121

        %121 = OpLabel
        %128 =   OpLoad %v4float %118               ; RelaxedPrecision
                 OpReturnValue %128
               OpFunctionEnd
