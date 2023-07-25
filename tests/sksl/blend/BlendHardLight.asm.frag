               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_overlay_component_Qhh2h2 "blend_overlay_component_Qhh2h2"
               OpName %blend_overlay_h4h4h4 "blend_overlay_h4h4h4"
               OpName %result "result"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %17 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %59 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
       %void = OpTypeVoid
        %116 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%blend_overlay_component_Qhh2h2 = OpFunction %float None %17
         %18 = OpFunctionParameter %_ptr_Function_v2float
         %19 = OpFunctionParameter %_ptr_Function_v2float
         %20 = OpLabel
         %28 = OpVariable %_ptr_Function_float Function
         %22 = OpLoad %v2float %19
         %23 = OpCompositeExtract %float %22 0
         %24 = OpFMul %float %float_2 %23
         %25 = OpLoad %v2float %19
         %26 = OpCompositeExtract %float %25 1
         %27 = OpFOrdLessThanEqual %bool %24 %26
               OpSelectionMerge %32 None
               OpBranchConditional %27 %30 %31
         %30 = OpLabel
         %33 = OpLoad %v2float %18
         %34 = OpCompositeExtract %float %33 0
         %35 = OpFMul %float %float_2 %34
         %36 = OpLoad %v2float %19
         %37 = OpCompositeExtract %float %36 0
         %38 = OpFMul %float %35 %37
               OpStore %28 %38
               OpBranch %32
         %31 = OpLabel
         %39 = OpLoad %v2float %18
         %40 = OpCompositeExtract %float %39 1
         %41 = OpLoad %v2float %19
         %42 = OpCompositeExtract %float %41 1
         %43 = OpFMul %float %40 %42
         %44 = OpLoad %v2float %19
         %45 = OpCompositeExtract %float %44 1
         %46 = OpLoad %v2float %19
         %47 = OpCompositeExtract %float %46 0
         %48 = OpFSub %float %45 %47
         %49 = OpFMul %float %float_2 %48
         %50 = OpLoad %v2float %18
         %51 = OpCompositeExtract %float %50 1
         %52 = OpLoad %v2float %18
         %53 = OpCompositeExtract %float %52 0
         %54 = OpFSub %float %51 %53
         %55 = OpFMul %float %49 %54
         %56 = OpFSub %float %43 %55
               OpStore %28 %56
               OpBranch %32
         %32 = OpLabel
         %57 = OpLoad %float %28
               OpReturnValue %57
               OpFunctionEnd
%blend_overlay_h4h4h4 = OpFunction %v4float None %59
         %60 = OpFunctionParameter %_ptr_Function_v4float
         %61 = OpFunctionParameter %_ptr_Function_v4float
         %62 = OpLabel
     %result = OpVariable %_ptr_Function_v4float Function
         %66 = OpVariable %_ptr_Function_v2float Function
         %69 = OpVariable %_ptr_Function_v2float Function
         %73 = OpVariable %_ptr_Function_v2float Function
         %76 = OpVariable %_ptr_Function_v2float Function
         %80 = OpVariable %_ptr_Function_v2float Function
         %83 = OpVariable %_ptr_Function_v2float Function
         %64 = OpLoad %v4float %60
         %65 = OpVectorShuffle %v2float %64 %64 0 3
               OpStore %66 %65
         %67 = OpLoad %v4float %61
         %68 = OpVectorShuffle %v2float %67 %67 0 3
               OpStore %69 %68
         %70 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %66 %69
         %71 = OpLoad %v4float %60
         %72 = OpVectorShuffle %v2float %71 %71 1 3
               OpStore %73 %72
         %74 = OpLoad %v4float %61
         %75 = OpVectorShuffle %v2float %74 %74 1 3
               OpStore %76 %75
         %77 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %73 %76
         %78 = OpLoad %v4float %60
         %79 = OpVectorShuffle %v2float %78 %78 2 3
               OpStore %80 %79
         %81 = OpLoad %v4float %61
         %82 = OpVectorShuffle %v2float %81 %81 2 3
               OpStore %83 %82
         %84 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %80 %83
         %85 = OpLoad %v4float %60
         %86 = OpCompositeExtract %float %85 3
         %88 = OpLoad %v4float %60
         %89 = OpCompositeExtract %float %88 3
         %90 = OpFSub %float %float_1 %89
         %91 = OpLoad %v4float %61
         %92 = OpCompositeExtract %float %91 3
         %93 = OpFMul %float %90 %92
         %94 = OpFAdd %float %86 %93
         %95 = OpCompositeConstruct %v4float %70 %77 %84 %94
               OpStore %result %95
         %96 = OpLoad %v4float %result
         %97 = OpVectorShuffle %v3float %96 %96 0 1 2
         %99 = OpLoad %v4float %61
        %100 = OpVectorShuffle %v3float %99 %99 0 1 2
        %101 = OpLoad %v4float %60
        %102 = OpCompositeExtract %float %101 3
        %103 = OpFSub %float %float_1 %102
        %104 = OpVectorTimesScalar %v3float %100 %103
        %105 = OpLoad %v4float %60
        %106 = OpVectorShuffle %v3float %105 %105 0 1 2
        %107 = OpLoad %v4float %61
        %108 = OpCompositeExtract %float %107 3
        %109 = OpFSub %float %float_1 %108
        %110 = OpVectorTimesScalar %v3float %106 %109
        %111 = OpFAdd %v3float %104 %110
        %112 = OpFAdd %v3float %97 %111
        %113 = OpLoad %v4float %result
        %114 = OpVectorShuffle %v4float %113 %112 4 5 6 3
               OpStore %result %114
               OpReturnValue %114
               OpFunctionEnd
       %main = OpFunction %void None %116
        %117 = OpLabel
        %123 = OpVariable %_ptr_Function_v4float Function
        %127 = OpVariable %_ptr_Function_v4float Function
        %118 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %122 = OpLoad %v4float %118
               OpStore %123 %122
        %124 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %126 = OpLoad %v4float %124
               OpStore %127 %126
        %128 = OpFunctionCall %v4float %blend_overlay_h4h4h4 %123 %127
               OpStore %sk_FragColor %128
               OpReturn
               OpFunctionEnd
