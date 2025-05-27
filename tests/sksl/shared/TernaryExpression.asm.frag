               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %check "check"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
      %check = OpVariable %_ptr_Function_int Function
         %65 = OpVariable %_ptr_Function_v4float Function
               OpStore %check %int_0
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %27
         %30 = OpCompositeExtract %float %29 1
         %32 = OpFOrdEqual %bool %30 %float_1
         %34 = OpSelect %int %32 %int_0 %int_1
         %36 = OpIAdd %int %int_0 %34
               OpStore %check %36
         %37 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %38 = OpLoad %v4float %37
         %39 = OpCompositeExtract %float %38 0
         %40 = OpFOrdEqual %bool %39 %float_1
         %41 = OpSelect %int %40 %int_1 %int_0
         %42 = OpIAdd %int %36 %41
               OpStore %check %42
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpVectorShuffle %v2float %44 %44 1 0
         %46 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %47 = OpLoad %v4float %46
         %48 = OpVectorShuffle %v2float %47 %47 0 1
         %49 = OpFOrdEqual %v2bool %45 %48
         %51 = OpAll %bool %49
         %52 = OpSelect %int %51 %int_0 %int_1
         %53 = OpIAdd %int %42 %52
               OpStore %check %53
         %54 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %55 = OpLoad %v4float %54
         %56 = OpVectorShuffle %v2float %55 %55 1 0
         %57 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %58 = OpLoad %v4float %57
         %59 = OpVectorShuffle %v2float %58 %58 0 1
         %60 = OpFUnordNotEqual %v2bool %56 %59
         %61 = OpAny %bool %60
         %62 = OpSelect %int %61 %int_1 %int_0
         %63 = OpIAdd %int %53 %62
               OpStore %check %63
         %64 = OpIEqual %bool %63 %int_0
               OpSelectionMerge %69 None
               OpBranchConditional %64 %67 %68
         %67 = OpLabel
         %70 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %71 = OpLoad %v4float %70
               OpStore %65 %71
               OpBranch %69
         %68 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %73 = OpLoad %v4float %72
               OpStore %65 %73
               OpBranch %69
         %69 = OpLabel
         %74 = OpLoad %v4float %65
               OpReturnValue %74
               OpFunctionEnd
