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
               OpName %expected "expected"              ; id %44

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
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
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
      %int_0 = OpConstant %int 0
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
         %47 = OpConstantComposite %v4bool %true %false %true %false
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
         %71 = OpConstantComposite %v2bool %true %false
         %78 = OpConstantComposite %v3bool %true %false %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1


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
         %86 =   OpVariable %_ptr_Function_v4float Function
         %31 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %34 =   OpLoad %v4float %31                ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 0    ; RelaxedPrecision
         %36 =   OpFUnordNotEqual %bool %35 %float_0
         %37 =   OpCompositeExtract %float %34 1    ; RelaxedPrecision
         %38 =   OpFUnordNotEqual %bool %37 %float_0
         %39 =   OpCompositeExtract %float %34 2    ; RelaxedPrecision
         %40 =   OpFUnordNotEqual %bool %39 %float_0
         %41 =   OpCompositeExtract %float %34 3    ; RelaxedPrecision
         %42 =   OpFUnordNotEqual %bool %41 %float_0
         %43 =   OpCompositeConstruct %v4bool %36 %38 %40 %42
                 OpStore %inputVal %43
                 OpStore %expected %47
         %49 =   OpVectorShuffle %v2bool %43 %43 0 1
         %48 =   OpLogicalNot %v2bool %49
         %51 =   OpVectorShuffle %v2bool %47 %47 0 1
         %52 =   OpLogicalEqual %v2bool %48 %51
         %53 =   OpAll %bool %52
                 OpSelectionMerge %55 None
                 OpBranchConditional %53 %54 %55

         %54 =     OpLabel
         %57 =       OpVectorShuffle %v3bool %43 %43 0 1 2
         %56 =       OpLogicalNot %v3bool %57
         %59 =       OpVectorShuffle %v3bool %47 %47 0 1 2
         %60 =       OpLogicalEqual %v3bool %56 %59
         %61 =       OpAll %bool %60
                     OpBranch %55

         %55 = OpLabel
         %62 =   OpPhi %bool %false %26 %61 %54
                 OpSelectionMerge %64 None
                 OpBranchConditional %62 %63 %64

         %63 =     OpLabel
         %65 =       OpLogicalNot %v4bool %43
         %66 =       OpLogicalEqual %v4bool %65 %47
         %67 =       OpAll %bool %66
                     OpBranch %64

         %64 = OpLabel
         %68 =   OpPhi %bool %false %55 %67 %63
                 OpSelectionMerge %70 None
                 OpBranchConditional %68 %69 %70

         %69 =     OpLabel
         %72 =       OpVectorShuffle %v2bool %47 %47 0 1
         %73 =       OpLogicalEqual %v2bool %71 %72
         %74 =       OpAll %bool %73
                     OpBranch %70

         %70 = OpLabel
         %75 =   OpPhi %bool %false %64 %74 %69
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %79 =       OpVectorShuffle %v3bool %47 %47 0 1 2
         %80 =       OpLogicalEqual %v3bool %78 %79
         %81 =       OpAll %bool %80
                     OpBranch %77

         %77 = OpLabel
         %82 =   OpPhi %bool %false %70 %81 %76
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
                     OpBranch %84

         %84 = OpLabel
         %85 =   OpPhi %bool %false %77 %true %83
                 OpSelectionMerge %90 None
                 OpBranchConditional %85 %88 %89

         %88 =     OpLabel
         %91 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %92 =       OpLoad %v4float %91            ; RelaxedPrecision
                     OpStore %86 %92
                     OpBranch %90

         %89 =     OpLabel
         %93 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %95 =       OpLoad %v4float %93            ; RelaxedPrecision
                     OpStore %86 %95
                     OpBranch %90

         %90 = OpLabel
         %96 =   OpLoad %v4float %86                ; RelaxedPrecision
                 OpReturnValue %96
               OpFunctionEnd
