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
               OpName %valueIsNaN "valueIsNaN"          ; id %27
               OpName %valueIsNumber "valueIsNumber"    ; id %36

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
               OpDecorate %valueIsNaN RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %valueIsNumber RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision

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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
         %34 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
         %40 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
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
 %valueIsNaN =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
%valueIsNumber =   OpVariable %_ptr_Function_v4float Function   ; RelaxedPrecision
         %93 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
         %33 =   OpVectorShuffle %v4float %32 %32 1 1 1 1   ; RelaxedPrecision
         %35 =   OpFDiv %v4float %34 %33                    ; RelaxedPrecision
                 OpStore %valueIsNaN %35
         %38 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %39 =   OpLoad %v4float %38                ; RelaxedPrecision
         %41 =   OpFDiv %v4float %40 %39            ; RelaxedPrecision
                 OpStore %valueIsNumber %41
         %45 =   OpCompositeExtract %float %35 0    ; RelaxedPrecision
         %44 =   OpIsNan %bool %45
                 OpSelectionMerge %47 None
                 OpBranchConditional %44 %46 %47

         %46 =     OpLabel
         %50 =       OpVectorShuffle %v2float %35 %35 0 1   ; RelaxedPrecision
         %49 =       OpIsNan %v2bool %50
         %48 =       OpAll %bool %49
                     OpBranch %47

         %47 = OpLabel
         %52 =   OpPhi %bool %false %26 %48 %46
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %57 =       OpVectorShuffle %v3float %35 %35 0 1 2     ; RelaxedPrecision
         %56 =       OpIsNan %v3bool %57
         %55 =       OpAll %bool %56
                     OpBranch %54

         %54 = OpLabel
         %60 =   OpPhi %bool %false %47 %55 %53
                 OpSelectionMerge %62 None
                 OpBranchConditional %60 %61 %62

         %61 =     OpLabel
         %64 =       OpIsNan %v4bool %35
         %63 =       OpAll %bool %64
                     OpBranch %62

         %62 = OpLabel
         %66 =   OpPhi %bool %false %54 %63 %61
                 OpSelectionMerge %68 None
                 OpBranchConditional %66 %67 %68

         %67 =     OpLabel
         %71 =       OpCompositeExtract %float %41 0    ; RelaxedPrecision
         %70 =       OpIsNan %bool %71
         %69 =       OpLogicalNot %bool %70
                     OpBranch %68

         %68 = OpLabel
         %72 =   OpPhi %bool %false %62 %69 %67
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
         %78 =       OpVectorShuffle %v2float %41 %41 0 1   ; RelaxedPrecision
         %77 =       OpIsNan %v2bool %78
         %76 =       OpAny %bool %77
         %75 =       OpLogicalNot %bool %76
                     OpBranch %74

         %74 = OpLabel
         %79 =   OpPhi %bool %false %68 %75 %73
                 OpSelectionMerge %81 None
                 OpBranchConditional %79 %80 %81

         %80 =     OpLabel
         %85 =       OpVectorShuffle %v3float %41 %41 0 1 2     ; RelaxedPrecision
         %84 =       OpIsNan %v3bool %85
         %83 =       OpAny %bool %84
         %82 =       OpLogicalNot %bool %83
                     OpBranch %81

         %81 = OpLabel
         %86 =   OpPhi %bool %false %74 %82 %80
                 OpSelectionMerge %88 None
                 OpBranchConditional %86 %87 %88

         %87 =     OpLabel
         %91 =       OpIsNan %v4bool %41
         %90 =       OpAny %bool %91
         %89 =       OpLogicalNot %bool %90
                     OpBranch %88

         %88 = OpLabel
         %92 =   OpPhi %bool %false %81 %89 %87
                 OpSelectionMerge %96 None
                 OpBranchConditional %92 %94 %95

         %94 =     OpLabel
         %97 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %99 =       OpLoad %v4float %97            ; RelaxedPrecision
                     OpStore %93 %99
                     OpBranch %96

         %95 =     OpLabel
        %100 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %102 =       OpLoad %v4float %100           ; RelaxedPrecision
                     OpStore %93 %102
                     OpBranch %96

         %96 = OpLabel
        %103 =   OpLoad %v4float %93                ; RelaxedPrecision
                 OpReturnValue %103
               OpFunctionEnd
