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
               OpName %expectedXY "expectedXY"          ; id %33
               OpName %expectedXYZ "expectedXYZ"        ; id %37
               OpName %expectedXYZW "expectedXYZW"      ; id %44

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
               OpDecorate %expectedXY RelaxedPrecision
               OpDecorate %expectedXYZ RelaxedPrecision
               OpDecorate %expectedXYZW RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision

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
%float_996878592 = OpConstant %float 996878592
%float_n1_99999996e_34 = OpConstant %float -1.99999996e+34
  %float_n49 = OpConstant %float -49
 %float_n169 = OpConstant %float -169
  %float_202 = OpConstant %float 202
         %36 = OpConstantComposite %v2float %float_n169 %float_202
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
 %float_n379 = OpConstant %float -379
  %float_454 = OpConstant %float 454
 %float_n529 = OpConstant %float -529
         %43 = OpConstantComposite %v3float %float_n379 %float_454 %float_n529
%_ptr_Function_v4float = OpTypePointer Function %v4float
 %float_n699 = OpConstant %float -699
  %float_838 = OpConstant %float 838
 %float_n977 = OpConstant %float -977
 %float_1116 = OpConstant %float 1116
         %50 = OpConstantComposite %v4float %float_n699 %float_838 %float_n977 %float_1116
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
        %114 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpExtInst %float %5 Reflect %float_996878592 %float_n1_99999996e_34    ; RelaxedPrecision
                 OpStore %expectedX %29
                 OpStore %expectedX %float_n49
                 OpStore %expectedXY %36
                 OpStore %expectedXYZ %43
                 OpStore %expectedXYZW %50
         %54 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %57 =   OpLoad %v4float %54                ; RelaxedPrecision
         %58 =   OpCompositeExtract %float %57 0    ; RelaxedPrecision
         %59 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %61 =   OpLoad %v4float %59                ; RelaxedPrecision
         %62 =   OpCompositeExtract %float %61 0    ; RelaxedPrecision
         %53 =   OpExtInst %float %5 Reflect %58 %62    ; RelaxedPrecision
         %63 =   OpFOrdEqual %bool %53 %float_n49
                 OpSelectionMerge %65 None
                 OpBranchConditional %63 %64 %65

         %64 =     OpLabel
         %67 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %68 =       OpLoad %v4float %67            ; RelaxedPrecision
         %69 =       OpVectorShuffle %v2float %68 %68 0 1   ; RelaxedPrecision
         %70 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %71 =       OpLoad %v4float %70            ; RelaxedPrecision
         %72 =       OpVectorShuffle %v2float %71 %71 0 1   ; RelaxedPrecision
         %66 =       OpExtInst %v2float %5 Reflect %69 %72  ; RelaxedPrecision
         %73 =       OpFOrdEqual %v2bool %66 %36
         %75 =       OpAll %bool %73
                     OpBranch %65

         %65 = OpLabel
         %76 =   OpPhi %bool %false %26 %75 %64
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %80 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %81 =       OpLoad %v4float %80            ; RelaxedPrecision
         %82 =       OpVectorShuffle %v3float %81 %81 0 1 2     ; RelaxedPrecision
         %83 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %84 =       OpLoad %v4float %83            ; RelaxedPrecision
         %85 =       OpVectorShuffle %v3float %84 %84 0 1 2     ; RelaxedPrecision
         %79 =       OpExtInst %v3float %5 Reflect %82 %85      ; RelaxedPrecision
         %86 =       OpFOrdEqual %v3bool %79 %43
         %88 =       OpAll %bool %86
                     OpBranch %78

         %78 = OpLabel
         %89 =   OpPhi %bool %false %65 %88 %77
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
         %93 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %94 =       OpLoad %v4float %93            ; RelaxedPrecision
         %95 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %96 =       OpLoad %v4float %95            ; RelaxedPrecision
         %92 =       OpExtInst %v4float %5 Reflect %94 %96  ; RelaxedPrecision
         %97 =       OpFOrdEqual %v4bool %92 %50
         %99 =       OpAll %bool %97
                     OpBranch %91

         %91 = OpLabel
        %100 =   OpPhi %bool %false %78 %99 %90
                 OpSelectionMerge %102 None
                 OpBranchConditional %100 %101 %102

        %101 =     OpLabel
                     OpBranch %102

        %102 = OpLabel
        %104 =   OpPhi %bool %false %91 %true %101
                 OpSelectionMerge %106 None
                 OpBranchConditional %104 %105 %106

        %105 =     OpLabel
                     OpBranch %106

        %106 = OpLabel
        %107 =   OpPhi %bool %false %102 %true %105
                 OpSelectionMerge %109 None
                 OpBranchConditional %107 %108 %109

        %108 =     OpLabel
                     OpBranch %109

        %109 = OpLabel
        %110 =   OpPhi %bool %false %106 %true %108
                 OpSelectionMerge %112 None
                 OpBranchConditional %110 %111 %112

        %111 =     OpLabel
                     OpBranch %112

        %112 = OpLabel
        %113 =   OpPhi %bool %false %109 %true %111
                 OpSelectionMerge %117 None
                 OpBranchConditional %113 %115 %116

        %115 =     OpLabel
        %118 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %120 =       OpLoad %v4float %118           ; RelaxedPrecision
                     OpStore %114 %120
                     OpBranch %117

        %116 =     OpLabel
        %121 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %123 =       OpLoad %v4float %121           ; RelaxedPrecision
                     OpStore %114 %123
                     OpBranch %117

        %117 = OpLabel
        %124 =   OpLoad %v4float %114               ; RelaxedPrecision
                 OpReturnValue %124
               OpFunctionEnd
