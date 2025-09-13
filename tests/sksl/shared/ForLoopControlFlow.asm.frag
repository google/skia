               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %x "x"                            ; id %27
               OpName %r "r"                            ; id %33
               OpName %b "b"                            ; id %56

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %r RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
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
%_ptr_Function_float = OpTypePointer Function %float
   %float_n5 = OpConstant %float -5
    %float_5 = OpConstant %float 5
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
      %int_2 = OpConstant %int 2
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
          %x =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %r =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
                 OpStore %x %32
                 OpStore %r %float_n5
                 OpBranch %36

         %36 = OpLabel
                 OpLoopMerge %40 %39 None
                 OpBranch %37

         %37 =     OpLabel
         %41 =       OpLoad %float %r               ; RelaxedPrecision
         %43 =       OpFOrdLessThan %bool %41 %float_5
                     OpBranchConditional %43 %38 %40

         %38 =         OpLabel
         %46 =           OpLoad %float %r           ; RelaxedPrecision
         %45 =           OpExtInst %float %5 FClamp %46 %float_0 %float_1   ; RelaxedPrecision
         %48 =           OpAccessChain %_ptr_Function_float %x %int_0
                         OpStore %48 %45
         %49 =           OpLoad %v4float %x         ; RelaxedPrecision
         %50 =           OpCompositeExtract %float %49 0    ; RelaxedPrecision
         %51 =           OpFOrdEqual %bool %50 %float_0
                         OpSelectionMerge %53 None
                         OpBranchConditional %51 %52 %53

         %52 =             OpLabel
                             OpBranch %40

         %53 =         OpLabel
                         OpBranch %39

         %39 =   OpLabel
         %54 =     OpLoad %float %r                 ; RelaxedPrecision
         %55 =     OpFAdd %float %54 %float_1       ; RelaxedPrecision
                   OpStore %r %55
                   OpBranch %36

         %40 = OpLabel
                 OpStore %b %float_5
                 OpBranch %57

         %57 = OpLabel
                 OpLoopMerge %61 %60 None
                 OpBranch %58

         %58 =     OpLabel
         %62 =       OpLoad %float %b               ; RelaxedPrecision
         %63 =       OpFOrdGreaterThanEqual %bool %62 %float_0
                     OpBranchConditional %63 %59 %61

         %59 =         OpLabel
         %64 =           OpLoad %float %b           ; RelaxedPrecision
         %65 =           OpAccessChain %_ptr_Function_float %x %int_2
                         OpStore %65 %64
         %67 =           OpLoad %v4float %x         ; RelaxedPrecision
         %68 =           OpCompositeExtract %float %67 3    ; RelaxedPrecision
         %69 =           OpFOrdEqual %bool %68 %float_1
                         OpSelectionMerge %71 None
                         OpBranchConditional %69 %70 %71

         %70 =             OpLabel
                             OpBranch %60

         %71 =         OpLabel
         %72 =           OpAccessChain %_ptr_Function_float %x %int_1
                         OpStore %72 %float_0
                         OpBranch %60

         %60 =   OpLabel
         %74 =     OpLoad %float %b                 ; RelaxedPrecision
         %75 =     OpFSub %float %74 %float_1       ; RelaxedPrecision
                   OpStore %b %75
                   OpBranch %57

         %61 = OpLabel
         %76 =   OpLoad %v4float %x                 ; RelaxedPrecision
                 OpReturnValue %76
               OpFunctionEnd
