               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %colorBlue "colorBlue"
               OpName %colorGreen "colorGreen"
               OpName %colorRed "colorRed"
               OpName %result "result"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %colorBlue RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %colorRed RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
  %colorBlue = OpVariable %_ptr_Function_v4float Function
 %colorGreen = OpVariable %_ptr_Function_v4float Function
   %colorRed = OpVariable %_ptr_Function_v4float Function
     %result = OpVariable %_ptr_Function_v4float Function
         %59 = OpVariable %_ptr_Function_v4float Function
         %65 = OpVariable %_ptr_Function_v4float Function
         %72 = OpVariable %_ptr_Function_v4float Function
         %82 = OpVariable %_ptr_Function_v4float Function
         %90 = OpVariable %_ptr_Function_v4float Function
         %98 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
         %33 = OpVectorShuffle %v2float %32 %32 2 3
         %34 = OpCompositeExtract %float %33 0
         %35 = OpCompositeExtract %float %33 1
         %36 = OpCompositeConstruct %v4float %float_0 %float_0 %34 %35
               OpStore %colorBlue %36
         %38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %39 = OpLoad %v4float %38
         %40 = OpCompositeExtract %float %39 1
         %41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %42 = OpLoad %v4float %41
         %43 = OpCompositeExtract %float %42 3
         %44 = OpCompositeConstruct %v4float %float_0 %40 %float_0 %43
               OpStore %colorGreen %44
         %46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %47 = OpLoad %v4float %46
         %48 = OpCompositeExtract %float %47 0
         %49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %50 = OpLoad %v4float %49
         %51 = OpCompositeExtract %float %50 3
         %52 = OpCompositeConstruct %v4float %48 %float_0 %float_0 %51
               OpStore %colorRed %52
         %54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %55 = OpLoad %v4float %54
         %56 = OpFUnordNotEqual %v4bool %55 %36
         %58 = OpAny %bool %56
               OpSelectionMerge %62 None
               OpBranchConditional %58 %60 %61
         %60 = OpLabel
         %63 = OpFOrdEqual %v4bool %44 %52
         %64 = OpAll %bool %63
               OpSelectionMerge %68 None
               OpBranchConditional %64 %66 %67
         %66 = OpLabel
               OpStore %65 %52
               OpBranch %68
         %67 = OpLabel
               OpStore %65 %44
               OpBranch %68
         %68 = OpLabel
         %69 = OpLoad %v4float %65
               OpStore %59 %69
               OpBranch %62
         %61 = OpLabel
         %70 = OpFUnordNotEqual %v4bool %52 %44
         %71 = OpAny %bool %70
               OpSelectionMerge %75 None
               OpBranchConditional %71 %73 %74
         %73 = OpLabel
               OpStore %72 %36
               OpBranch %75
         %74 = OpLabel
         %76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %77 = OpLoad %v4float %76
               OpStore %72 %77
               OpBranch %75
         %75 = OpLabel
         %78 = OpLoad %v4float %72
               OpStore %59 %78
               OpBranch %62
         %62 = OpLabel
         %79 = OpLoad %v4float %59
               OpStore %result %79
         %80 = OpFOrdEqual %v4bool %52 %36
         %81 = OpAll %bool %80
               OpSelectionMerge %85 None
               OpBranchConditional %81 %83 %84
         %83 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %87 = OpLoad %v4float %86
               OpStore %82 %87
               OpBranch %85
         %84 = OpLabel
         %88 = OpFUnordNotEqual %v4bool %52 %44
         %89 = OpAny %bool %88
               OpSelectionMerge %93 None
               OpBranchConditional %89 %91 %92
         %91 = OpLabel
               OpStore %90 %79
               OpBranch %93
         %92 = OpLabel
         %94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %95 = OpLoad %v4float %94
         %96 = OpFOrdEqual %v4bool %52 %95
         %97 = OpAll %bool %96
               OpSelectionMerge %101 None
               OpBranchConditional %97 %99 %100
         %99 = OpLabel
               OpStore %98 %36
               OpBranch %101
        %100 = OpLabel
               OpStore %98 %52
               OpBranch %101
        %101 = OpLabel
        %102 = OpLoad %v4float %98
               OpStore %90 %102
               OpBranch %93
         %93 = OpLabel
        %103 = OpLoad %v4float %90
               OpStore %82 %103
               OpBranch %85
         %85 = OpLabel
        %104 = OpLoad %v4float %82
               OpReturnValue %104
               OpFunctionEnd
