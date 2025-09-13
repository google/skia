               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %infiniteValue "infiniteValue"    ; id %28
               OpName %finiteValue "finiteValue"        ; id %47

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %infiniteValue RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %finiteValue RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
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
%infiniteValue =   OpVariable %_ptr_Function_v4float Function   ; RelaxedPrecision
%finiteValue =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %111 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_0
         %33 =   OpLoad %mat2v2float %30            ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 0 0  ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %33 0 1  ; RelaxedPrecision
         %36 =   OpCompositeExtract %float %33 1 0  ; RelaxedPrecision
         %37 =   OpCompositeExtract %float %33 1 1  ; RelaxedPrecision
         %38 =   OpCompositeConstruct %v4float %34 %35 %36 %37  ; RelaxedPrecision
         %39 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %42 =   OpLoad %v4float %39                ; RelaxedPrecision
         %43 =   OpCompositeExtract %float %42 0    ; RelaxedPrecision
         %45 =   OpFDiv %float %float_1 %43         ; RelaxedPrecision
         %46 =   OpVectorTimesScalar %v4float %38 %45   ; RelaxedPrecision
                 OpStore %infiniteValue %46
         %48 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_0
         %49 =   OpLoad %mat2v2float %48            ; RelaxedPrecision
         %50 =   OpCompositeExtract %float %49 0 0  ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %49 0 1  ; RelaxedPrecision
         %52 =   OpCompositeExtract %float %49 1 0  ; RelaxedPrecision
         %53 =   OpCompositeExtract %float %49 1 1  ; RelaxedPrecision
         %54 =   OpCompositeConstruct %v4float %50 %51 %52 %53  ; RelaxedPrecision
         %55 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %56 =   OpLoad %v4float %55                ; RelaxedPrecision
         %57 =   OpCompositeExtract %float %56 1    ; RelaxedPrecision
         %58 =   OpFDiv %float %float_1 %57         ; RelaxedPrecision
         %59 =   OpVectorTimesScalar %v4float %54 %58   ; RelaxedPrecision
                 OpStore %finiteValue %59
         %63 =   OpCompositeExtract %float %46 0    ; RelaxedPrecision
         %62 =   OpIsInf %bool %63
                 OpSelectionMerge %65 None
                 OpBranchConditional %62 %64 %65

         %64 =     OpLabel
         %68 =       OpVectorShuffle %v2float %46 %46 0 1   ; RelaxedPrecision
         %67 =       OpIsInf %v2bool %68
         %66 =       OpAll %bool %67
                     OpBranch %65

         %65 = OpLabel
         %70 =   OpPhi %bool %false %27 %66 %64
                 OpSelectionMerge %72 None
                 OpBranchConditional %70 %71 %72

         %71 =     OpLabel
         %75 =       OpVectorShuffle %v3float %46 %46 0 1 2     ; RelaxedPrecision
         %74 =       OpIsInf %v3bool %75
         %73 =       OpAll %bool %74
                     OpBranch %72

         %72 = OpLabel
         %78 =   OpPhi %bool %false %65 %73 %71
                 OpSelectionMerge %80 None
                 OpBranchConditional %78 %79 %80

         %79 =     OpLabel
         %82 =       OpIsInf %v4bool %46
         %81 =       OpAll %bool %82
                     OpBranch %80

         %80 = OpLabel
         %84 =   OpPhi %bool %false %72 %81 %79
                 OpSelectionMerge %86 None
                 OpBranchConditional %84 %85 %86

         %85 =     OpLabel
         %89 =       OpCompositeExtract %float %59 0    ; RelaxedPrecision
         %88 =       OpIsInf %bool %89
         %87 =       OpLogicalNot %bool %88
                     OpBranch %86

         %86 = OpLabel
         %90 =   OpPhi %bool %false %80 %87 %85
                 OpSelectionMerge %92 None
                 OpBranchConditional %90 %91 %92

         %91 =     OpLabel
         %96 =       OpVectorShuffle %v2float %59 %59 0 1   ; RelaxedPrecision
         %95 =       OpIsInf %v2bool %96
         %94 =       OpAny %bool %95
         %93 =       OpLogicalNot %bool %94
                     OpBranch %92

         %92 = OpLabel
         %97 =   OpPhi %bool %false %86 %93 %91
                 OpSelectionMerge %99 None
                 OpBranchConditional %97 %98 %99

         %98 =     OpLabel
        %103 =       OpVectorShuffle %v3float %59 %59 0 1 2     ; RelaxedPrecision
        %102 =       OpIsInf %v3bool %103
        %101 =       OpAny %bool %102
        %100 =       OpLogicalNot %bool %101
                     OpBranch %99

         %99 = OpLabel
        %104 =   OpPhi %bool %false %92 %100 %98
                 OpSelectionMerge %106 None
                 OpBranchConditional %104 %105 %106

        %105 =     OpLabel
        %109 =       OpIsInf %v4bool %59
        %108 =       OpAny %bool %109
        %107 =       OpLogicalNot %bool %108
                     OpBranch %106

        %106 = OpLabel
        %110 =   OpPhi %bool %false %99 %107 %105
                 OpSelectionMerge %114 None
                 OpBranchConditional %110 %112 %113

        %112 =     OpLabel
        %115 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %116 =       OpLoad %v4float %115           ; RelaxedPrecision
                     OpStore %111 %116
                     OpBranch %114

        %113 =     OpLabel
        %117 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %119 =       OpLoad %v4float %117           ; RelaxedPrecision
                     OpStore %111 %119
                     OpBranch %114

        %114 = OpLabel
        %120 =   OpLoad %v4float %111               ; RelaxedPrecision
                 OpReturnValue %120
               OpFunctionEnd
