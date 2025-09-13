               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %result "result"                  ; id %27

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
               OpDecorate %36 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision

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
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
         %38 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
    %float_3 = OpConstant %float 3
         %41 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
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
     %result =   OpVariable %_ptr_Function_v4bool Function
         %46 =   OpVariable %_ptr_Function_v4float Function
         %33 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %36 =   OpLoad %v4float %33                ; RelaxedPrecision
         %32 =   OpFOrdLessThan %v4bool %36 %38
         %42 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %44 =   OpLoad %v4float %42                ; RelaxedPrecision
         %39 =   OpFOrdGreaterThan %v4bool %41 %44
         %31 =   OpLogicalEqual %v4bool %32 %39
                 OpStore %result %31
         %45 =   OpAll %bool %31
                 OpSelectionMerge %50 None
                 OpBranchConditional %45 %48 %49

         %48 =     OpLabel
         %51 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %52 =       OpLoad %v4float %51            ; RelaxedPrecision
                     OpStore %46 %52
                     OpBranch %50

         %49 =     OpLabel
         %53 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %54 =       OpLoad %v4float %53            ; RelaxedPrecision
                     OpStore %46 %54
                     OpBranch %50

         %50 = OpLabel
         %55 =   OpLoad %v4float %46                ; RelaxedPrecision
                 OpReturnValue %55
               OpFunctionEnd
