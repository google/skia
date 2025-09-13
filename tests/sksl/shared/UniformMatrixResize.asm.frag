               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %15
               OpMemberName %_UniformBuffer 0 "testMatrix3x3"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %17
               OpName %resizeMatrix_f22 "resizeMatrix_f22"  ; id %6
               OpName %main "main"                          ; id %7

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 48
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 64
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %90 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat3v3float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat2v2float = OpTypeMatrix %v2float 2
         %28 = OpTypeFunction %mat2v2float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %40 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_2 = OpConstant %float 2
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
         %49 = OpConstantComposite %v2float %float_1 %float_2
         %50 = OpConstantComposite %v2float %float_4 %float_5
         %51 = OpConstantComposite %mat2v2float %49 %50
     %v2bool = OpTypeVector %bool 2
         %67 = OpConstantComposite %v3float %float_0 %float_0 %float_1
         %69 = OpConstantComposite %v3float %float_1 %float_2 %float_0
         %70 = OpConstantComposite %v3float %float_4 %float_5 %float_0
         %71 = OpConstantComposite %mat3v3float %69 %70 %67
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %19

         %20 = OpLabel
         %24 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %24 %23
         %26 =   OpFunctionCall %v4float %main %24
                 OpStore %sk_FragColor %26
                 OpReturn
               OpFunctionEnd


               ; Function resizeMatrix_f22
%resizeMatrix_f22 = OpFunction %mat2v2float None %28

         %29 = OpLabel
         %30 =   OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_0
         %33 =   OpLoad %mat3v3float %30
         %35 =   OpCompositeExtract %v3float %33 0
         %36 =   OpVectorShuffle %v2float %35 %35 0 1
         %37 =   OpCompositeExtract %v3float %33 1
         %38 =   OpVectorShuffle %v2float %37 %37 0 1
         %39 =   OpCompositeConstruct %mat2v2float %36 %38
                 OpReturnValue %39
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %40         ; RelaxedPrecision
         %41 = OpFunctionParameter %_ptr_Function_v2float

         %42 = OpLabel
         %82 =   OpVariable %_ptr_Function_v4float Function
         %45 =   OpFunctionCall %mat2v2float %resizeMatrix_f22
         %53 =   OpCompositeExtract %v2float %45 0
         %54 =   OpFOrdEqual %v2bool %53 %49
         %55 =   OpAll %bool %54
         %56 =   OpCompositeExtract %v2float %45 1
         %57 =   OpFOrdEqual %v2bool %56 %50
         %58 =   OpAll %bool %57
         %59 =   OpLogicalAnd %bool %55 %58
                 OpSelectionMerge %61 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
         %62 =       OpFunctionCall %mat2v2float %resizeMatrix_f22
         %63 =       OpCompositeExtract %v2float %62 0
         %64 =       OpCompositeConstruct %v3float %63 %float_0
         %65 =       OpCompositeExtract %v2float %62 1
         %66 =       OpCompositeConstruct %v3float %65 %float_0
         %68 =       OpCompositeConstruct %mat3v3float %64 %66 %67
         %73 =       OpFOrdEqual %v3bool %64 %69
         %74 =       OpAll %bool %73
         %75 =       OpFOrdEqual %v3bool %66 %70
         %76 =       OpAll %bool %75
         %77 =       OpLogicalAnd %bool %74 %76
         %78 =       OpFOrdEqual %v3bool %67 %67
         %79 =       OpAll %bool %78
         %80 =       OpLogicalAnd %bool %77 %79
                     OpBranch %61

         %61 = OpLabel
         %81 =   OpPhi %bool %false %42 %80 %60
                 OpSelectionMerge %86 None
                 OpBranchConditional %81 %84 %85

         %84 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %90 =       OpLoad %v4float %87            ; RelaxedPrecision
                     OpStore %82 %90
                     OpBranch %86

         %85 =     OpLabel
         %91 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_2
         %93 =       OpLoad %v4float %91            ; RelaxedPrecision
                     OpStore %82 %93
                     OpBranch %86

         %86 = OpLabel
         %94 =   OpLoad %v4float %82                ; RelaxedPrecision
                 OpReturnValue %94
               OpFunctionEnd
