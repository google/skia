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
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
%uint_1065353216 = OpConstant %uint 1065353216
     %v2uint = OpTypeVector %uint 2
%uint_1073741824 = OpConstant %uint 1073741824
         %56 = OpConstantComposite %v2uint %uint_1065353216 %uint_1073741824
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3uint = OpTypeVector %uint 3
%uint_3225419776 = OpConstant %uint 3225419776
         %68 = OpConstantComposite %v3uint %uint_1065353216 %uint_1073741824 %uint_3225419776
     %v3bool = OpTypeVector %bool 3
     %v4uint = OpTypeVector %uint 4
%uint_3229614080 = OpConstant %uint 3229614080
         %78 = OpConstantComposite %v4uint %uint_1065353216 %uint_1073741824 %uint_3225419776 %uint_3229614080
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
         %83 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_0
         %33 =   OpLoad %mat2v2float %30
         %34 =   OpCompositeExtract %float %33 0 0
         %35 =   OpCompositeExtract %float %33 0 1
         %36 =   OpCompositeExtract %float %33 1 0
         %37 =   OpCompositeExtract %float %33 1 1
         %38 =   OpCompositeConstruct %v4float %34 %35 %36 %37
         %42 =   OpFMul %v4float %38 %41
                 OpStore %inputVal %42
         %46 =   OpCompositeExtract %float %42 0
         %45 =   OpBitcast %uint %46
         %49 =   OpIEqual %bool %45 %uint_1065353216
                 OpSelectionMerge %51 None
                 OpBranchConditional %49 %50 %51

         %50 =     OpLabel
         %53 =       OpVectorShuffle %v2float %42 %42 0 1
         %52 =       OpBitcast %v2uint %53
         %57 =       OpIEqual %v2bool %52 %56
         %59 =       OpAll %bool %57
                     OpBranch %51

         %51 = OpLabel
         %60 =   OpPhi %bool %false %27 %59 %50
                 OpSelectionMerge %62 None
                 OpBranchConditional %60 %61 %62

         %61 =     OpLabel
         %64 =       OpVectorShuffle %v3float %42 %42 0 1 2
         %63 =       OpBitcast %v3uint %64
         %69 =       OpIEqual %v3bool %63 %68
         %71 =       OpAll %bool %69
                     OpBranch %62

         %62 = OpLabel
         %72 =   OpPhi %bool %false %51 %71 %61
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
         %75 =       OpBitcast %v4uint %42
         %79 =       OpIEqual %v4bool %75 %78
         %81 =       OpAll %bool %79
                     OpBranch %74

         %74 = OpLabel
         %82 =   OpPhi %bool %false %62 %81 %73
                 OpSelectionMerge %86 None
                 OpBranchConditional %82 %84 %85

         %84 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %90 =       OpLoad %v4float %87            ; RelaxedPrecision
                     OpStore %83 %90
                     OpBranch %86

         %85 =     OpLabel
         %91 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %93 =       OpLoad %v4float %91            ; RelaxedPrecision
                     OpStore %83 %93
                     OpBranch %86

         %86 = OpLabel
         %94 =   OpLoad %v4float %83                ; RelaxedPrecision
                 OpReturnValue %94
               OpFunctionEnd
