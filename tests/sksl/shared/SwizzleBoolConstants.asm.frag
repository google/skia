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
               OpName %v "v"                            ; id %27
               OpName %result "result"                  ; id %38

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
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision

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
     %v2bool = OpTypeVector %bool 2
      %false = OpConstantFalse %bool
     %v3bool = OpTypeVector %bool 3
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
          %v =   OpVariable %_ptr_Function_v4bool Function
     %result =   OpVariable %_ptr_Function_v4bool Function
        %108 =   OpVariable %_ptr_Function_v4float Function
         %31 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %34 =   OpLoad %v4float %31                ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 1    ; RelaxedPrecision
         %36 =   OpFUnordNotEqual %bool %35 %float_0
         %37 =   OpCompositeConstruct %v4bool %36 %36 %36 %36
                 OpStore %v %37
         %39 =   OpCompositeExtract %bool %37 0
         %41 =   OpCompositeConstruct %v4bool %39 %true %true %true
                 OpStore %result %41
         %42 =   OpVectorShuffle %v2bool %37 %37 0 1
         %44 =   OpCompositeExtract %bool %42 0
         %45 =   OpCompositeExtract %bool %42 1
         %47 =   OpCompositeConstruct %v4bool %44 %45 %false %true
                 OpStore %result %47
         %48 =   OpCompositeConstruct %v4bool %39 %true %true %false
                 OpStore %result %48
         %49 =   OpCompositeExtract %bool %37 1
         %50 =   OpCompositeConstruct %v4bool %false %49 %true %true
                 OpStore %result %50
         %51 =   OpVectorShuffle %v3bool %37 %37 0 1 2
         %53 =   OpCompositeExtract %bool %51 0
         %54 =   OpCompositeExtract %bool %51 1
         %55 =   OpCompositeExtract %bool %51 2
         %56 =   OpCompositeConstruct %v4bool %53 %54 %55 %true
                 OpStore %result %56
         %57 =   OpVectorShuffle %v2bool %37 %37 0 1
         %58 =   OpCompositeExtract %bool %57 0
         %59 =   OpCompositeExtract %bool %57 1
         %60 =   OpCompositeConstruct %v4bool %58 %59 %true %true
                 OpStore %result %60
         %61 =   OpCompositeExtract %bool %37 2
         %62 =   OpCompositeConstruct %v4bool %39 %false %61 %true
                 OpStore %result %62
         %63 =   OpCompositeConstruct %v4bool %39 %true %false %false
                 OpStore %result %63
         %64 =   OpVectorShuffle %v2bool %37 %37 1 2
         %65 =   OpCompositeExtract %bool %64 0
         %66 =   OpCompositeExtract %bool %64 1
         %67 =   OpCompositeConstruct %v4bool %true %65 %66 %false
                 OpStore %result %67
         %68 =   OpCompositeConstruct %v4bool %false %49 %true %false
                 OpStore %result %68
         %69 =   OpCompositeConstruct %v4bool %true %true %61 %false
                 OpStore %result %69
                 OpStore %result %37
         %70 =   OpVectorShuffle %v3bool %37 %37 0 1 2
         %71 =   OpCompositeExtract %bool %70 0
         %72 =   OpCompositeExtract %bool %70 1
         %73 =   OpCompositeExtract %bool %70 2
         %74 =   OpCompositeConstruct %v4bool %71 %72 %73 %true
                 OpStore %result %74
         %75 =   OpVectorShuffle %v2bool %37 %37 0 1
         %76 =   OpCompositeExtract %bool %75 0
         %77 =   OpCompositeExtract %bool %75 1
         %78 =   OpCompositeExtract %bool %37 3
         %79 =   OpCompositeConstruct %v4bool %76 %77 %false %78
                 OpStore %result %79
         %80 =   OpVectorShuffle %v2bool %37 %37 0 1
         %81 =   OpCompositeExtract %bool %80 0
         %82 =   OpCompositeExtract %bool %80 1
         %83 =   OpCompositeConstruct %v4bool %81 %82 %true %false
                 OpStore %result %83
         %84 =   OpVectorShuffle %v2bool %37 %37 2 3
         %85 =   OpCompositeExtract %bool %84 0
         %86 =   OpCompositeExtract %bool %84 1
         %87 =   OpCompositeConstruct %v4bool %39 %true %85 %86
                 OpStore %result %87
                 OpStore %result %62
         %88 =   OpCompositeConstruct %v4bool %39 %true %true %78
                 OpStore %result %88
         %89 =   OpCompositeConstruct %v4bool %39 %true %false %true
                 OpStore %result %89
         %90 =   OpVectorShuffle %v3bool %37 %37 1 2 3
         %91 =   OpCompositeExtract %bool %90 0
         %92 =   OpCompositeExtract %bool %90 1
         %93 =   OpCompositeExtract %bool %90 2
         %94 =   OpCompositeConstruct %v4bool %true %91 %92 %93
                 OpStore %result %94
         %95 =   OpVectorShuffle %v2bool %37 %37 1 2
         %96 =   OpCompositeExtract %bool %95 0
         %97 =   OpCompositeExtract %bool %95 1
         %98 =   OpCompositeConstruct %v4bool %false %96 %97 %true
                 OpStore %result %98
         %99 =   OpCompositeConstruct %v4bool %false %49 %true %78
                 OpStore %result %99
        %100 =   OpCompositeConstruct %v4bool %true %49 %true %true
                 OpStore %result %100
        %101 =   OpVectorShuffle %v2bool %37 %37 2 3
        %102 =   OpCompositeExtract %bool %101 0
        %103 =   OpCompositeExtract %bool %101 1
        %104 =   OpCompositeConstruct %v4bool %false %false %102 %103
                 OpStore %result %104
        %105 =   OpCompositeConstruct %v4bool %false %false %61 %true
                 OpStore %result %105
        %106 =   OpCompositeConstruct %v4bool %false %true %true %78
                 OpStore %result %106
        %107 =   OpAny %bool %106
                 OpSelectionMerge %112 None
                 OpBranchConditional %107 %110 %111

        %110 =     OpLabel
        %113 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %114 =       OpLoad %v4float %113           ; RelaxedPrecision
                     OpStore %108 %114
                     OpBranch %112

        %111 =     OpLabel
        %115 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %117 =       OpLoad %v4float %115           ; RelaxedPrecision
                     OpStore %108 %117
                     OpBranch %112

        %112 = OpLabel
        %118 =   OpLoad %v4float %108               ; RelaxedPrecision
                 OpReturnValue %118
               OpFunctionEnd
