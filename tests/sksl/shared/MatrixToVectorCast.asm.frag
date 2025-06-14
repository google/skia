               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %ok "ok"                          ; id %28

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
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %48 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
     %v4bool = OpTypeVector %bool 4
      %v4int = OpTypeVector %int 4
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
         %83 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
        %101 = OpConstantComposite %v4bool %true %true %true %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


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
         %ok =   OpVariable %_ptr_Function_bool Function
        %105 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %ok %true
                 OpSelectionMerge %34 None
                 OpBranchConditional %true %33 %34

         %33 =     OpLabel
         %35 =       OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %38 =       OpLoad %mat2v2float %35        ; RelaxedPrecision
         %39 =       OpCompositeExtract %float %38 0 0  ; RelaxedPrecision
         %40 =       OpCompositeExtract %float %38 0 1  ; RelaxedPrecision
         %41 =       OpCompositeExtract %float %38 1 0  ; RelaxedPrecision
         %42 =       OpCompositeExtract %float %38 1 1  ; RelaxedPrecision
         %43 =       OpCompositeConstruct %v4float %39 %40 %41 %42  ; RelaxedPrecision
         %49 =       OpFOrdEqual %v4bool %43 %48
         %51 =       OpAll %bool %49
                     OpBranch %34

         %34 = OpLabel
         %52 =   OpPhi %bool %false %27 %51 %33
                 OpStore %ok %52
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %55 =       OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %56 =       OpLoad %mat2v2float %55        ; RelaxedPrecision
         %57 =       OpCompositeExtract %float %56 0 0  ; RelaxedPrecision
         %58 =       OpCompositeExtract %float %56 0 1  ; RelaxedPrecision
         %59 =       OpCompositeExtract %float %56 1 0  ; RelaxedPrecision
         %60 =       OpCompositeExtract %float %56 1 1  ; RelaxedPrecision
         %61 =       OpCompositeConstruct %v4float %57 %58 %59 %60  ; RelaxedPrecision
         %62 =       OpFOrdEqual %v4bool %61 %48
         %63 =       OpAll %bool %62
                     OpBranch %54

         %54 = OpLabel
         %64 =   OpPhi %bool %false %34 %63 %53
                 OpStore %ok %64
                 OpSelectionMerge %66 None
                 OpBranchConditional %64 %65 %66

         %65 =     OpLabel
         %67 =       OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %68 =       OpLoad %mat2v2float %67        ; RelaxedPrecision
         %69 =       OpCompositeExtract %float %68 0 0  ; RelaxedPrecision
         %70 =       OpCompositeExtract %float %68 0 1  ; RelaxedPrecision
         %71 =       OpCompositeExtract %float %68 1 0  ; RelaxedPrecision
         %72 =       OpCompositeExtract %float %68 1 1  ; RelaxedPrecision
         %73 =       OpCompositeConstruct %v4float %69 %70 %71 %72  ; RelaxedPrecision
         %74 =       OpConvertFToS %int %69
         %75 =       OpConvertFToS %int %70
         %76 =       OpConvertFToS %int %71
         %77 =       OpConvertFToS %int %72
         %79 =       OpCompositeConstruct %v4int %74 %75 %76 %77
         %84 =       OpIEqual %v4bool %79 %83
         %85 =       OpAll %bool %84
                     OpBranch %66

         %66 = OpLabel
         %86 =   OpPhi %bool %false %54 %85 %65
                 OpStore %ok %86
                 OpSelectionMerge %88 None
                 OpBranchConditional %86 %87 %88

         %87 =     OpLabel
         %89 =       OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %90 =       OpLoad %mat2v2float %89        ; RelaxedPrecision
         %91 =       OpCompositeExtract %float %90 0 0  ; RelaxedPrecision
         %92 =       OpCompositeExtract %float %90 0 1  ; RelaxedPrecision
         %93 =       OpCompositeExtract %float %90 1 0  ; RelaxedPrecision
         %94 =       OpCompositeExtract %float %90 1 1  ; RelaxedPrecision
         %95 =       OpCompositeConstruct %v4float %91 %92 %93 %94  ; RelaxedPrecision
         %96 =       OpFUnordNotEqual %bool %91 %float_0
         %97 =       OpFUnordNotEqual %bool %92 %float_0
         %98 =       OpFUnordNotEqual %bool %93 %float_0
         %99 =       OpFUnordNotEqual %bool %94 %float_0
        %100 =       OpCompositeConstruct %v4bool %96 %97 %98 %99
        %102 =       OpLogicalEqual %v4bool %100 %101
        %103 =       OpAll %bool %102
                     OpBranch %88

         %88 = OpLabel
        %104 =   OpPhi %bool %false %66 %103 %87
                 OpStore %ok %104
                 OpSelectionMerge %109 None
                 OpBranchConditional %104 %107 %108

        %107 =     OpLabel
        %110 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %113 =       OpLoad %v4float %110           ; RelaxedPrecision
                     OpStore %105 %113
                     OpBranch %109

        %108 =     OpLabel
        %114 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %115 =       OpLoad %v4float %114           ; RelaxedPrecision
                     OpStore %105 %115
                     OpBranch %109

        %109 = OpLabel
        %116 =   OpLoad %v4float %105               ; RelaxedPrecision
                 OpReturnValue %116
               OpFunctionEnd
