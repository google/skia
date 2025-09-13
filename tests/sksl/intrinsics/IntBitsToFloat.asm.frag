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
               OpName %inputVal "inputVal"              ; id %28
               OpName %expectedB "expectedB"            ; id %43

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
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
    %float_1 = OpConstant %float 1
   %float_n1 = OpConstant %float -1
         %41 = OpConstantComposite %v4float %float_1 %float_1 %float_n1 %float_n1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1065353216 = OpConstant %int 1065353216
%int_1073741824 = OpConstant %int 1073741824
%int_n1069547520 = OpConstant %int -1069547520
%int_n1065353216 = OpConstant %int -1065353216
         %50 = OpConstantComposite %v4int %int_1065353216 %int_1073741824 %int_n1069547520 %int_n1065353216
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
      %v3int = OpTypeVector %int 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
   %inputVal =   OpVariable %_ptr_Function_v4float Function
  %expectedB =   OpVariable %_ptr_Function_v4int Function
         %84 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_0
         %33 =   OpLoad %mat2v2float %30
         %34 =   OpCompositeExtract %float %33 0 0
         %35 =   OpCompositeExtract %float %33 0 1
         %36 =   OpCompositeExtract %float %33 1 0
         %37 =   OpCompositeExtract %float %33 1 1
         %38 =   OpCompositeConstruct %v4float %34 %35 %36 %37
         %42 =   OpFMul %v4float %38 %41
                 OpStore %inputVal %42
                 OpStore %expectedB %50
         %53 =   OpCompositeExtract %float %42 0
         %54 =   OpBitcast %float %int_1065353216
         %55 =   OpFOrdEqual %bool %53 %54
                 OpSelectionMerge %57 None
                 OpBranchConditional %55 %56 %57

         %56 =     OpLabel
         %58 =       OpVectorShuffle %v2float %42 %42 0 1
         %60 =       OpVectorShuffle %v2int %50 %50 0 1
         %59 =       OpBitcast %v2float %60
         %62 =       OpFOrdEqual %v2bool %58 %59
         %64 =       OpAll %bool %62
                     OpBranch %57

         %57 = OpLabel
         %65 =   OpPhi %bool %false %27 %64 %56
                 OpSelectionMerge %67 None
                 OpBranchConditional %65 %66 %67

         %66 =     OpLabel
         %68 =       OpVectorShuffle %v3float %42 %42 0 1 2
         %71 =       OpVectorShuffle %v3int %50 %50 0 1 2
         %70 =       OpBitcast %v3float %71
         %73 =       OpFOrdEqual %v3bool %68 %70
         %75 =       OpAll %bool %73
                     OpBranch %67

         %67 = OpLabel
         %76 =   OpPhi %bool %false %57 %75 %66
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %79 =       OpBitcast %v4float %50
         %80 =       OpFOrdEqual %v4bool %42 %79
         %82 =       OpAll %bool %80
                     OpBranch %78

         %78 = OpLabel
         %83 =   OpPhi %bool %false %67 %82 %77
                 OpSelectionMerge %87 None
                 OpBranchConditional %83 %85 %86

         %85 =     OpLabel
         %88 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %91 =       OpLoad %v4float %88            ; RelaxedPrecision
                     OpStore %84 %91
                     OpBranch %87

         %86 =     OpLabel
         %92 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %94 =       OpLoad %v4float %92            ; RelaxedPrecision
                     OpStore %84 %94
                     OpBranch %87

         %87 = OpLabel
         %95 =   OpLoad %v4float %84                ; RelaxedPrecision
                 OpReturnValue %95
               OpFunctionEnd
