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
               OpDecorate %92 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision

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
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%uint_1065353216 = OpConstant %uint 1065353216
%uint_1073741824 = OpConstant %uint 1073741824
%uint_3225419776 = OpConstant %uint 3225419776
%uint_3229614080 = OpConstant %uint 3229614080
         %51 = OpConstantComposite %v4uint %uint_1065353216 %uint_1073741824 %uint_3225419776 %uint_3229614080
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3uint = OpTypeVector %uint 3
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
  %expectedB =   OpVariable %_ptr_Function_v4uint Function
         %85 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_0
         %33 =   OpLoad %mat2v2float %30
         %34 =   OpCompositeExtract %float %33 0 0
         %35 =   OpCompositeExtract %float %33 0 1
         %36 =   OpCompositeExtract %float %33 1 0
         %37 =   OpCompositeExtract %float %33 1 1
         %38 =   OpCompositeConstruct %v4float %34 %35 %36 %37
         %42 =   OpFMul %v4float %38 %41
                 OpStore %inputVal %42
                 OpStore %expectedB %51
         %54 =   OpCompositeExtract %float %42 0
         %55 =   OpBitcast %float %uint_1065353216
         %56 =   OpFOrdEqual %bool %54 %55
                 OpSelectionMerge %58 None
                 OpBranchConditional %56 %57 %58

         %57 =     OpLabel
         %59 =       OpVectorShuffle %v2float %42 %42 0 1
         %61 =       OpVectorShuffle %v2uint %51 %51 0 1
         %60 =       OpBitcast %v2float %61
         %63 =       OpFOrdEqual %v2bool %59 %60
         %65 =       OpAll %bool %63
                     OpBranch %58

         %58 = OpLabel
         %66 =   OpPhi %bool %false %27 %65 %57
                 OpSelectionMerge %68 None
                 OpBranchConditional %66 %67 %68

         %67 =     OpLabel
         %69 =       OpVectorShuffle %v3float %42 %42 0 1 2
         %72 =       OpVectorShuffle %v3uint %51 %51 0 1 2
         %71 =       OpBitcast %v3float %72
         %74 =       OpFOrdEqual %v3bool %69 %71
         %76 =       OpAll %bool %74
                     OpBranch %68

         %68 = OpLabel
         %77 =   OpPhi %bool %false %58 %76 %67
                 OpSelectionMerge %79 None
                 OpBranchConditional %77 %78 %79

         %78 =     OpLabel
         %80 =       OpBitcast %v4float %51
         %81 =       OpFOrdEqual %v4bool %42 %80
         %83 =       OpAll %bool %81
                     OpBranch %79

         %79 = OpLabel
         %84 =   OpPhi %bool %false %68 %83 %78
                 OpSelectionMerge %88 None
                 OpBranchConditional %84 %86 %87

         %86 =     OpLabel
         %89 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %92 =       OpLoad %v4float %89            ; RelaxedPrecision
                     OpStore %85 %92
                     OpBranch %88

         %87 =     OpLabel
         %93 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %95 =       OpLoad %v4float %93            ; RelaxedPrecision
                     OpStore %85 %95
                     OpBranch %88

         %88 = OpLabel
         %96 =   OpLoad %v4float %85                ; RelaxedPrecision
                 OpReturnValue %96
               OpFunctionEnd
