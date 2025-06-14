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
               OpName %ok "ok"                          ; id %27
               OpName %TRUE "TRUE"                      ; id %31

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
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision

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
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %false = OpConstantFalse %bool
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
     %v2bool = OpTypeVector %bool 2
         %67 = OpConstantComposite %v2bool %true %true
      %v2int = OpTypeVector %int 2
         %75 = OpConstantComposite %v2int %int_1 %int_1
         %83 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float


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
         %ok =   OpVariable %_ptr_Function_bool Function
       %TRUE =   OpVariable %_ptr_Function_bool Function
         %89 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %ok %true
         %32 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %35 =   OpLoad %v4float %32                ; RelaxedPrecision
         %36 =   OpCompositeExtract %float %35 1    ; RelaxedPrecision
         %37 =   OpFUnordNotEqual %bool %36 %float_0
                 OpStore %TRUE %37
                 OpSelectionMerge %40 None
                 OpBranchConditional %true %39 %40

         %39 =     OpLabel
         %42 =       OpSelect %int %37 %int_1 %int_0
         %43 =       OpIEqual %bool %int_1 %42
                     OpBranch %40

         %40 = OpLabel
         %44 =   OpPhi %bool %false %26 %43 %39
                 OpStore %ok %44
                 OpSelectionMerge %46 None
                 OpBranchConditional %44 %45 %46

         %45 =     OpLabel
         %48 =       OpSelect %float %37 %float_1 %float_0
         %49 =       OpFOrdEqual %bool %float_1 %48
                     OpBranch %46

         %46 = OpLabel
         %50 =   OpPhi %bool %false %40 %49 %45
                 OpStore %ok %50
                 OpSelectionMerge %52 None
                 OpBranchConditional %50 %51 %52

         %51 =     OpLabel
                     OpBranch %52

         %52 = OpLabel
         %53 =   OpPhi %bool %false %46 %37 %51
                 OpStore %ok %53
                 OpSelectionMerge %55 None
                 OpBranchConditional %53 %54 %55

         %54 =     OpLabel
         %56 =       OpSelect %int %37 %int_1 %int_0
         %57 =       OpIEqual %bool %int_1 %56
                     OpBranch %55

         %55 = OpLabel
         %58 =   OpPhi %bool %false %52 %57 %54
                 OpStore %ok %58
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %59 %60

         %59 =     OpLabel
         %61 =       OpSelect %float %37 %float_1 %float_0
         %62 =       OpFOrdEqual %bool %float_1 %61
                     OpBranch %60

         %60 = OpLabel
         %63 =   OpPhi %bool %false %55 %62 %59
                 OpStore %ok %63
                 OpSelectionMerge %65 None
                 OpBranchConditional %63 %64 %65

         %64 =     OpLabel
         %68 =       OpCompositeConstruct %v2bool %37 %37
         %69 =       OpLogicalEqual %v2bool %67 %68
         %70 =       OpAll %bool %69
                     OpBranch %65

         %65 = OpLabel
         %71 =   OpPhi %bool %false %60 %70 %64
                 OpStore %ok %71
                 OpSelectionMerge %73 None
                 OpBranchConditional %71 %72 %73

         %72 =     OpLabel
         %76 =       OpSelect %int %37 %int_1 %int_0
         %77 =       OpCompositeConstruct %v2int %76 %76
         %78 =       OpIEqual %v2bool %75 %77
         %79 =       OpAll %bool %78
                     OpBranch %73

         %73 = OpLabel
         %80 =   OpPhi %bool %false %65 %79 %72
                 OpStore %ok %80
                 OpSelectionMerge %82 None
                 OpBranchConditional %80 %81 %82

         %81 =     OpLabel
         %84 =       OpSelect %float %37 %float_1 %float_0
         %85 =       OpCompositeConstruct %v2float %84 %84
         %86 =       OpFOrdEqual %v2bool %83 %85
         %87 =       OpAll %bool %86
                     OpBranch %82

         %82 = OpLabel
         %88 =   OpPhi %bool %false %73 %87 %81
                 OpStore %ok %88
                 OpSelectionMerge %93 None
                 OpBranchConditional %88 %91 %92

         %91 =     OpLabel
         %94 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %95 =       OpLoad %v4float %94            ; RelaxedPrecision
                     OpStore %89 %95
                     OpBranch %93

         %92 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %97 =       OpLoad %v4float %96            ; RelaxedPrecision
                     OpStore %89 %97
                     OpBranch %93

         %93 = OpLabel
         %98 =   OpLoad %v4float %89                ; RelaxedPrecision
                 OpReturnValue %98
               OpFunctionEnd
