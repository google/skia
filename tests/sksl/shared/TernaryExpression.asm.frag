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
               OpName %check "check"                    ; id %27

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
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision

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
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
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
      %check =   OpVariable %_ptr_Function_int Function
         %68 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %check %int_0
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %30                ; RelaxedPrecision
         %33 =   OpCompositeExtract %float %32 1    ; RelaxedPrecision
         %35 =   OpFOrdEqual %bool %33 %float_1
         %37 =   OpSelect %int %35 %int_0 %int_1
         %39 =   OpIAdd %int %int_0 %37
                 OpStore %check %39
         %40 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %41 =   OpLoad %v4float %40                ; RelaxedPrecision
         %42 =   OpCompositeExtract %float %41 0    ; RelaxedPrecision
         %43 =   OpFOrdEqual %bool %42 %float_1
         %44 =   OpSelect %int %43 %int_1 %int_0
         %45 =   OpIAdd %int %39 %44
                 OpStore %check %45
         %46 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
         %48 =   OpVectorShuffle %v2float %47 %47 1 0   ; RelaxedPrecision
         %49 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
         %51 =   OpVectorShuffle %v2float %50 %50 0 1   ; RelaxedPrecision
         %52 =   OpFOrdEqual %v2bool %48 %51
         %54 =   OpAll %bool %52
         %55 =   OpSelect %int %54 %int_0 %int_1
         %56 =   OpIAdd %int %45 %55
                 OpStore %check %56
         %57 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %58 =   OpLoad %v4float %57                ; RelaxedPrecision
         %59 =   OpVectorShuffle %v2float %58 %58 1 0   ; RelaxedPrecision
         %60 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %61 =   OpLoad %v4float %60                ; RelaxedPrecision
         %62 =   OpVectorShuffle %v2float %61 %61 0 1   ; RelaxedPrecision
         %63 =   OpFUnordNotEqual %v2bool %59 %62
         %64 =   OpAny %bool %63
         %65 =   OpSelect %int %64 %int_1 %int_0
         %66 =   OpIAdd %int %56 %65
                 OpStore %check %66
         %67 =   OpIEqual %bool %66 %int_0
                 OpSelectionMerge %72 None
                 OpBranchConditional %67 %70 %71

         %70 =     OpLabel
         %73 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %74 =       OpLoad %v4float %73            ; RelaxedPrecision
                     OpStore %68 %74
                     OpBranch %72

         %71 =     OpLabel
         %75 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %76 =       OpLoad %v4float %75            ; RelaxedPrecision
                     OpStore %68 %76
                     OpBranch %72

         %72 = OpLabel
         %77 =   OpLoad %v4float %68                ; RelaxedPrecision
                 OpReturnValue %77
               OpFunctionEnd
