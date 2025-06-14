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
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %101 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision

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
 %float_1_25 = OpConstant %float 1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %32 = OpConstantComposite %v4float %float_1_25 %float_0 %float_0_75 %float_2_25
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %80 = OpConstantComposite %v2float %float_1_25 %float_0
         %87 = OpConstantComposite %v3float %float_1_25 %float_0 %float_0_75
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
   %expected =   OpVariable %_ptr_Function_v4float Function
         %95 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %32
         %36 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %39 =   OpLoad %v4float %36
         %40 =   OpCompositeExtract %float %39 0
         %35 =   OpExtInst %float %5 FAbs %40
         %41 =   OpFOrdEqual %bool %35 %float_1_25
                 OpSelectionMerge %43 None
                 OpBranchConditional %41 %42 %43

         %42 =     OpLabel
         %45 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %46 =       OpLoad %v4float %45
         %47 =       OpVectorShuffle %v2float %46 %46 0 1
         %44 =       OpExtInst %v2float %5 FAbs %47
         %48 =       OpVectorShuffle %v2float %32 %32 0 1
         %49 =       OpFOrdEqual %v2bool %44 %48
         %51 =       OpAll %bool %49
                     OpBranch %43

         %43 = OpLabel
         %52 =   OpPhi %bool %false %26 %51 %42
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %56 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %57 =       OpLoad %v4float %56
         %58 =       OpVectorShuffle %v3float %57 %57 0 1 2
         %55 =       OpExtInst %v3float %5 FAbs %58
         %60 =       OpVectorShuffle %v3float %32 %32 0 1 2
         %61 =       OpFOrdEqual %v3bool %55 %60
         %63 =       OpAll %bool %61
                     OpBranch %54

         %54 = OpLabel
         %64 =   OpPhi %bool %false %43 %63 %53
                 OpSelectionMerge %66 None
                 OpBranchConditional %64 %65 %66

         %65 =     OpLabel
         %68 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %69 =       OpLoad %v4float %68
         %67 =       OpExtInst %v4float %5 FAbs %69
         %70 =       OpFOrdEqual %v4bool %67 %32
         %72 =       OpAll %bool %70
                     OpBranch %66

         %66 = OpLabel
         %73 =   OpPhi %bool %false %54 %72 %65
                 OpSelectionMerge %75 None
                 OpBranchConditional %73 %74 %75

         %74 =     OpLabel
                     OpBranch %75

         %75 = OpLabel
         %77 =   OpPhi %bool %false %66 %true %74
                 OpSelectionMerge %79 None
                 OpBranchConditional %77 %78 %79

         %78 =     OpLabel
         %81 =       OpVectorShuffle %v2float %32 %32 0 1
         %82 =       OpFOrdEqual %v2bool %80 %81
         %83 =       OpAll %bool %82
                     OpBranch %79

         %79 = OpLabel
         %84 =   OpPhi %bool %false %75 %83 %78
                 OpSelectionMerge %86 None
                 OpBranchConditional %84 %85 %86

         %85 =     OpLabel
         %88 =       OpVectorShuffle %v3float %32 %32 0 1 2
         %89 =       OpFOrdEqual %v3bool %87 %88
         %90 =       OpAll %bool %89
                     OpBranch %86

         %86 = OpLabel
         %91 =   OpPhi %bool %false %79 %90 %85
                 OpSelectionMerge %93 None
                 OpBranchConditional %91 %92 %93

         %92 =     OpLabel
                     OpBranch %93

         %93 = OpLabel
         %94 =   OpPhi %bool %false %86 %true %92
                 OpSelectionMerge %98 None
                 OpBranchConditional %94 %96 %97

         %96 =     OpLabel
         %99 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %101 =       OpLoad %v4float %99            ; RelaxedPrecision
                     OpStore %95 %101
                     OpBranch %98

         %97 =     OpLabel
        %102 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %104 =       OpLoad %v4float %102           ; RelaxedPrecision
                     OpStore %95 %104
                     OpBranch %98

         %98 = OpLabel
        %105 =   OpLoad %v4float %95                ; RelaxedPrecision
                 OpReturnValue %105
               OpFunctionEnd
