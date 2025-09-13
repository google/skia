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
               OpName %val "val"                        ; id %31
               OpName %mask "mask"                      ; id %40
               OpName %imask "imask"                    ; id %45

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
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision

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
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %false = OpConstantFalse %bool
     %uint_0 = OpConstant %uint 0
         %66 = OpConstantComposite %v2uint %uint_0 %uint_0
     %v2bool = OpTypeVector %bool 2
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
         %ok =   OpVariable %_ptr_Function_bool Function
        %val =   OpVariable %_ptr_Function_uint Function
       %mask =   OpVariable %_ptr_Function_v2uint Function
      %imask =   OpVariable %_ptr_Function_v2int Function
         %71 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %ok %true
         %34 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %37 =   OpLoad %v4float %34                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 0    ; RelaxedPrecision
         %39 =   OpConvertFToU %uint %38
                 OpStore %val %39
         %43 =   OpNot %uint %39
         %44 =   OpCompositeConstruct %v2uint %39 %43
                 OpStore %mask %44
         %48 =   OpNot %v2uint %44
         %49 =   OpCompositeExtract %uint %48 0
         %50 =   OpBitcast %int %49
         %51 =   OpCompositeExtract %uint %48 1
         %52 =   OpBitcast %int %51
         %53 =   OpCompositeConstruct %v2int %50 %52
                 OpStore %imask %53
         %54 =   OpNot %v2uint %44
         %55 =   OpNot %v2int %53
         %56 =   OpCompositeExtract %int %55 0
         %57 =   OpBitcast %uint %56
         %58 =   OpCompositeExtract %int %55 1
         %59 =   OpBitcast %uint %58
         %60 =   OpCompositeConstruct %v2uint %57 %59
         %61 =   OpBitwiseAnd %v2uint %54 %60
                 OpStore %mask %61
                 OpSelectionMerge %64 None
                 OpBranchConditional %true %63 %64

         %63 =     OpLabel
         %67 =       OpIEqual %v2bool %61 %66
         %69 =       OpAll %bool %67
                     OpBranch %64

         %64 = OpLabel
         %70 =   OpPhi %bool %false %26 %69 %63
                 OpStore %ok %70
                 OpSelectionMerge %75 None
                 OpBranchConditional %70 %73 %74

         %73 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %77 =       OpLoad %v4float %76            ; RelaxedPrecision
                     OpStore %71 %77
                     OpBranch %75

         %74 =     OpLabel
         %78 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %80 =       OpLoad %v4float %78            ; RelaxedPrecision
                     OpStore %71 %80
                     OpBranch %75

         %75 = OpLabel
         %81 =   OpLoad %v4float %71                ; RelaxedPrecision
                 OpReturnValue %81
               OpFunctionEnd
