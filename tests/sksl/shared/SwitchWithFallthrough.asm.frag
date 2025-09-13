               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %switch_fallthrough_twice_bi "switch_fallthrough_twice_bi"    ; id %6
               OpName %ok "ok"                                                      ; id %30
               OpName %main "main"                                                  ; id %7
               OpName %x "x"                                                        ; id %45
               OpName %_0_ok "_0_ok"                                                ; id %52

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
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %41 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_int = OpTypePointer Function %int
         %27 = OpTypeFunction %bool %_ptr_Function_int
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %42 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function switch_fallthrough_twice_bi
%switch_fallthrough_twice_bi = OpFunction %bool None %27
         %28 = OpFunctionParameter %_ptr_Function_int

         %29 = OpLabel
         %ok =   OpVariable %_ptr_Function_bool Function
                 OpStore %ok %false
         %33 =   OpLoad %int %28
                 OpSelectionMerge %34 None
                 OpSwitch %33 %39 0 %35 1 %38 2 %38 3 %38

         %35 =     OpLabel
                     OpBranch %34

         %38 =     OpLabel
                     OpStore %ok %true
                     OpBranch %34

         %39 =     OpLabel
                     OpBranch %34

         %34 = OpLabel
         %41 =   OpLoad %bool %ok                   ; RelaxedPrecision
                 OpReturnValue %41
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %42         ; RelaxedPrecision
         %43 = OpFunctionParameter %_ptr_Function_v2float

         %44 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
      %_0_ok =   OpVariable %_ptr_Function_bool Function
         %61 =   OpVariable %_ptr_Function_int Function
         %64 =   OpVariable %_ptr_Function_v4float Function
         %46 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %49 =   OpLoad %v4float %46                ; RelaxedPrecision
         %50 =   OpCompositeExtract %float %49 1    ; RelaxedPrecision
         %51 =   OpConvertFToS %int %50
                 OpStore %x %51
                 OpStore %_0_ok %false
                 OpSelectionMerge %53 None
                 OpSwitch %51 %57 2 %54 1 %56 0 %56

         %54 =     OpLabel
                     OpBranch %53

         %56 =     OpLabel
                     OpStore %_0_ok %true
                     OpBranch %53

         %57 =     OpLabel
                     OpBranch %53

         %53 = OpLabel
         %58 =   OpLoad %bool %_0_ok                ; RelaxedPrecision
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %59 %60

         %59 =     OpLabel
                     OpStore %61 %51
         %62 =       OpFunctionCall %bool %switch_fallthrough_twice_bi %61
                     OpBranch %60

         %60 = OpLabel
         %63 =   OpPhi %bool %false %53 %62 %59
                 OpSelectionMerge %68 None
                 OpBranchConditional %63 %66 %67

         %66 =     OpLabel
         %69 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %70 =       OpLoad %v4float %69            ; RelaxedPrecision
                     OpStore %64 %70
                     OpBranch %68

         %67 =     OpLabel
         %71 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %73 =       OpLoad %v4float %71            ; RelaxedPrecision
                     OpStore %64 %73
                     OpBranch %68

         %68 = OpLabel
         %74 =   OpLoad %v4float %64                ; RelaxedPrecision
                 OpReturnValue %74
               OpFunctionEnd
