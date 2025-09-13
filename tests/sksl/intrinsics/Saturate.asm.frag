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
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision

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
 %float_0_75 = OpConstant %float 0.75
    %float_1 = OpConstant %float 1
         %31 = OpConstantComposite %v4float %float_0 %float_0 %float_0_75 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
         %47 = OpConstantComposite %v2float %float_1 %float_1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %60 = OpConstantComposite %v3float %float_0 %float_0 %float_0
         %61 = OpConstantComposite %v3float %float_1 %float_1 %float_1
     %v3bool = OpTypeVector %bool 3
         %72 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %73 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %90 = OpConstantComposite %v3float %float_0 %float_0 %float_0_75
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
   %expected =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %98 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %31
         %35 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %38 =   OpLoad %v4float %35                ; RelaxedPrecision
         %39 =   OpCompositeExtract %float %38 0    ; RelaxedPrecision
         %34 =   OpExtInst %float %5 FClamp %39 %float_0 %float_1   ; RelaxedPrecision
         %40 =   OpFOrdEqual %bool %34 %float_0
                 OpSelectionMerge %42 None
                 OpBranchConditional %40 %41 %42

         %41 =     OpLabel
         %44 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %45 =       OpLoad %v4float %44            ; RelaxedPrecision
         %46 =       OpVectorShuffle %v2float %45 %45 0 1   ; RelaxedPrecision
         %43 =       OpExtInst %v2float %5 FClamp %46 %20 %47   ; RelaxedPrecision
         %48 =       OpVectorShuffle %v2float %31 %31 0 1       ; RelaxedPrecision
         %49 =       OpFOrdEqual %v2bool %43 %48
         %51 =       OpAll %bool %49
                     OpBranch %42

         %42 = OpLabel
         %52 =   OpPhi %bool %false %26 %51 %41
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %56 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %57 =       OpLoad %v4float %56            ; RelaxedPrecision
         %58 =       OpVectorShuffle %v3float %57 %57 0 1 2     ; RelaxedPrecision
         %55 =       OpExtInst %v3float %5 FClamp %58 %60 %61   ; RelaxedPrecision
         %62 =       OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
         %63 =       OpFOrdEqual %v3bool %55 %62
         %65 =       OpAll %bool %63
                     OpBranch %54

         %54 = OpLabel
         %66 =   OpPhi %bool %false %42 %65 %53
                 OpSelectionMerge %68 None
                 OpBranchConditional %66 %67 %68

         %67 =     OpLabel
         %70 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %71 =       OpLoad %v4float %70            ; RelaxedPrecision
         %69 =       OpExtInst %v4float %5 FClamp %71 %72 %73   ; RelaxedPrecision
         %74 =       OpFOrdEqual %v4bool %69 %31
         %76 =       OpAll %bool %74
                     OpBranch %68

         %68 = OpLabel
         %77 =   OpPhi %bool %false %54 %76 %67
                 OpSelectionMerge %79 None
                 OpBranchConditional %77 %78 %79

         %78 =     OpLabel
                     OpBranch %79

         %79 = OpLabel
         %81 =   OpPhi %bool %false %68 %true %78
                 OpSelectionMerge %83 None
                 OpBranchConditional %81 %82 %83

         %82 =     OpLabel
         %84 =       OpVectorShuffle %v2float %31 %31 0 1   ; RelaxedPrecision
         %85 =       OpFOrdEqual %v2bool %20 %84
         %86 =       OpAll %bool %85
                     OpBranch %83

         %83 = OpLabel
         %87 =   OpPhi %bool %false %79 %86 %82
                 OpSelectionMerge %89 None
                 OpBranchConditional %87 %88 %89

         %88 =     OpLabel
         %91 =       OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
         %92 =       OpFOrdEqual %v3bool %90 %91
         %93 =       OpAll %bool %92
                     OpBranch %89

         %89 = OpLabel
         %94 =   OpPhi %bool %false %83 %93 %88
                 OpSelectionMerge %96 None
                 OpBranchConditional %94 %95 %96

         %95 =     OpLabel
                     OpBranch %96

         %96 = OpLabel
         %97 =   OpPhi %bool %false %89 %true %95
                 OpSelectionMerge %101 None
                 OpBranchConditional %97 %99 %100

         %99 =     OpLabel
        %102 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %104 =       OpLoad %v4float %102           ; RelaxedPrecision
                     OpStore %98 %104
                     OpBranch %101

        %100 =     OpLabel
        %105 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %107 =       OpLoad %v4float %105           ; RelaxedPrecision
                     OpStore %98 %107
                     OpBranch %101

        %101 = OpLabel
        %108 =   OpLoad %v4float %98                ; RelaxedPrecision
                 OpReturnValue %108
               OpFunctionEnd
