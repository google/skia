               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x "x"
               OpName %r "r"
               OpName %b "b"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %r RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
   %float_n5 = OpConstant %float -5
    %float_5 = OpConstant %float 5
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
          %x = OpVariable %_ptr_Function_v4float Function
          %r = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
               OpStore %x %29
               OpStore %r %float_n5
               OpBranch %33
         %33 = OpLabel
               OpLoopMerge %37 %36 None
               OpBranch %34
         %34 = OpLabel
         %38 = OpLoad %float %r
         %40 = OpFOrdLessThan %bool %38 %float_5
               OpBranchConditional %40 %35 %37
         %35 = OpLabel
         %43 = OpLoad %float %r
         %42 = OpExtInst %float %1 FClamp %43 %float_0 %float_1
         %45 = OpAccessChain %_ptr_Function_float %x %int_0
               OpStore %45 %42
         %46 = OpLoad %v4float %x
         %47 = OpCompositeExtract %float %46 0
         %48 = OpFOrdEqual %bool %47 %float_0
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
               OpBranch %37
         %50 = OpLabel
               OpBranch %36
         %36 = OpLabel
         %51 = OpLoad %float %r
         %52 = OpFAdd %float %51 %float_1
               OpStore %r %52
               OpBranch %33
         %37 = OpLabel
               OpStore %b %float_5
               OpBranch %54
         %54 = OpLabel
               OpLoopMerge %58 %57 None
               OpBranch %55
         %55 = OpLabel
         %59 = OpLoad %float %b
         %60 = OpFOrdGreaterThanEqual %bool %59 %float_0
               OpBranchConditional %60 %56 %58
         %56 = OpLabel
         %61 = OpLoad %float %b
         %62 = OpAccessChain %_ptr_Function_float %x %int_2
               OpStore %62 %61
         %64 = OpLoad %v4float %x
         %65 = OpCompositeExtract %float %64 3
         %66 = OpFOrdEqual %bool %65 %float_1
               OpSelectionMerge %68 None
               OpBranchConditional %66 %67 %68
         %67 = OpLabel
               OpBranch %57
         %68 = OpLabel
         %69 = OpAccessChain %_ptr_Function_float %x %int_1
               OpStore %69 %float_0
               OpBranch %57
         %57 = OpLabel
         %71 = OpLoad %float %b
         %72 = OpFSub %float %71 %float_1
               OpStore %b %72
               OpBranch %54
         %58 = OpLabel
         %73 = OpLoad %v4float %x
               OpReturnValue %73
               OpFunctionEnd
