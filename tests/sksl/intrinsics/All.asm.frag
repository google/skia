               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %inputVal "inputVal"              ; id %27
               OpName %expected "expected"              ; id %45

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_0 = OpConstant %int 0


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
   %inputVal =   OpVariable %_ptr_Function_v4bool Function
   %expected =   OpVariable %_ptr_Function_v4bool Function
         %91 =   OpVariable %_ptr_Function_v4float Function
         %31 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %34 =   OpLoad %v4float %31                ; RelaxedPrecision
         %35 =   OpVectorShuffle %v4float %34 %34 0 0 2 3   ; RelaxedPrecision
         %36 =   OpCompositeExtract %float %35 0            ; RelaxedPrecision
         %37 =   OpFUnordNotEqual %bool %36 %float_0
         %38 =   OpCompositeExtract %float %35 1    ; RelaxedPrecision
         %39 =   OpFUnordNotEqual %bool %38 %float_0
         %40 =   OpCompositeExtract %float %35 2    ; RelaxedPrecision
         %41 =   OpFUnordNotEqual %bool %40 %float_0
         %42 =   OpCompositeExtract %float %35 3    ; RelaxedPrecision
         %43 =   OpFUnordNotEqual %bool %42 %float_0
         %44 =   OpCompositeConstruct %v4bool %37 %39 %41 %43
                 OpStore %inputVal %44
         %46 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
         %48 =   OpVectorShuffle %v4float %47 %47 0 1 2 2   ; RelaxedPrecision
         %49 =   OpCompositeExtract %float %48 0            ; RelaxedPrecision
         %50 =   OpFUnordNotEqual %bool %49 %float_0
         %51 =   OpCompositeExtract %float %48 1    ; RelaxedPrecision
         %52 =   OpFUnordNotEqual %bool %51 %float_0
         %53 =   OpCompositeExtract %float %48 2    ; RelaxedPrecision
         %54 =   OpFUnordNotEqual %bool %53 %float_0
         %55 =   OpCompositeExtract %float %48 3    ; RelaxedPrecision
         %56 =   OpFUnordNotEqual %bool %55 %float_0
         %57 =   OpCompositeConstruct %v4bool %50 %52 %54 %56
                 OpStore %expected %57
         %60 =   OpVectorShuffle %v2bool %44 %44 0 1
         %59 =   OpAll %bool %60
         %62 =   OpCompositeExtract %bool %57 0
         %63 =   OpLogicalEqual %bool %59 %62
                 OpSelectionMerge %65 None
                 OpBranchConditional %63 %64 %65

         %64 =     OpLabel
         %67 =       OpVectorShuffle %v3bool %44 %44 0 1 2
         %66 =       OpAll %bool %67
         %69 =       OpCompositeExtract %bool %57 1
         %70 =       OpLogicalEqual %bool %66 %69
                     OpBranch %65

         %65 = OpLabel
         %71 =   OpPhi %bool %false %26 %70 %64
                 OpSelectionMerge %73 None
                 OpBranchConditional %71 %72 %73

         %72 =     OpLabel
         %74 =       OpAll %bool %44
         %75 =       OpCompositeExtract %bool %57 2
         %76 =       OpLogicalEqual %bool %74 %75
                     OpBranch %73

         %73 = OpLabel
         %77 =   OpPhi %bool %false %65 %76 %72
                 OpSelectionMerge %79 None
                 OpBranchConditional %77 %78 %79

         %78 =     OpLabel
                     OpBranch %79

         %79 = OpLabel
         %80 =   OpPhi %bool %false %73 %62 %78
                 OpSelectionMerge %82 None
                 OpBranchConditional %80 %81 %82

         %81 =     OpLabel
         %83 =       OpCompositeExtract %bool %57 1
         %84 =       OpLogicalEqual %bool %false %83
                     OpBranch %82

         %82 = OpLabel
         %85 =   OpPhi %bool %false %79 %84 %81
                 OpSelectionMerge %87 None
                 OpBranchConditional %85 %86 %87

         %86 =     OpLabel
         %88 =       OpCompositeExtract %bool %57 2
         %89 =       OpLogicalEqual %bool %false %88
                     OpBranch %87

         %87 = OpLabel
         %90 =   OpPhi %bool %false %82 %89 %86
                 OpSelectionMerge %95 None
                 OpBranchConditional %90 %93 %94

         %93 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %98 =       OpLoad %v4float %96            ; RelaxedPrecision
                     OpStore %91 %98
                     OpBranch %95

         %94 =     OpLabel
         %99 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %100 =       OpLoad %v4float %99            ; RelaxedPrecision
                     OpStore %91 %100
                     OpBranch %95

         %95 = OpLabel
        %101 =   OpLoad %v4float %91                ; RelaxedPrecision
                 OpReturnValue %101
               OpFunctionEnd
