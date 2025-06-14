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
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %xy "xy"                          ; id %27
               OpName %zw "zw"                          ; id %36

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
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %68 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision

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
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_0_015625 = OpConstant %float 0.015625
         %48 = OpConstantComposite %v2float %float_0_015625 %float_0_015625
     %v2bool = OpTypeVector %bool 2
 %float_0_75 = OpConstant %float 0.75
    %float_1 = OpConstant %float 1
         %58 = OpConstantComposite %v2float %float_0_75 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_0 = OpConstant %int 0
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
         %xy =   OpVariable %_ptr_Function_uint Function
         %zw =   OpVariable %_ptr_Function_uint Function
         %61 =   OpVariable %_ptr_Function_v4float Function
         %31 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %34 =   OpLoad %v4float %31
         %35 =   OpVectorShuffle %v2float %34 %34 0 1
         %30 =   OpExtInst %uint %5 PackUnorm2x16 %35
                 OpStore %xy %30
         %38 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %39 =   OpLoad %v4float %38
         %40 =   OpVectorShuffle %v2float %39 %39 2 3
         %37 =   OpExtInst %uint %5 PackUnorm2x16 %40
                 OpStore %zw %37
         %46 =   OpExtInst %v2float %5 UnpackUnorm2x16 %30
         %45 =   OpExtInst %v2float %5 FAbs %46
         %44 =   OpFOrdLessThan %v2bool %45 %48
         %43 =   OpAll %bool %44
                 OpSelectionMerge %51 None
                 OpBranchConditional %43 %50 %51

         %50 =     OpLabel
         %55 =       OpExtInst %v2float %5 UnpackUnorm2x16 %37
         %59 =       OpFSub %v2float %55 %58
         %54 =       OpExtInst %v2float %5 FAbs %59
         %53 =       OpFOrdLessThan %v2bool %54 %48
         %52 =       OpAll %bool %53
                     OpBranch %51

         %51 = OpLabel
         %60 =   OpPhi %bool %false %26 %52 %50
                 OpSelectionMerge %65 None
                 OpBranchConditional %60 %63 %64

         %63 =     OpLabel
         %66 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %68 =       OpLoad %v4float %66            ; RelaxedPrecision
                     OpStore %61 %68
                     OpBranch %65

         %64 =     OpLabel
         %69 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %71 =       OpLoad %v4float %69            ; RelaxedPrecision
                     OpStore %61 %71
                     OpBranch %65

         %65 = OpLabel
         %72 =   OpLoad %v4float %61                ; RelaxedPrecision
                 OpReturnValue %72
               OpFunctionEnd
