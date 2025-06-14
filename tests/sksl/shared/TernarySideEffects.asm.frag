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
               OpName %x "x"                            ; id %27
               OpName %y "y"                            ; id %30
               OpName %b "b"                            ; id %125
               OpName %c "c"                            ; id %127

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
               OpDecorate %x RelaxedPrecision
               OpDecorate %y RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision

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
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
    %float_9 = OpConstant %float 9
    %float_2 = OpConstant %float 2
    %float_4 = OpConstant %float 4
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_8 = OpConstant %float 8
   %float_17 = OpConstant %float 17
      %int_0 = OpConstant %int 0


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
          %x =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
          %y =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %33 =   OpVariable %_ptr_Function_float Function
         %43 =   OpVariable %_ptr_Function_float Function
         %56 =   OpVariable %_ptr_Function_float Function
         %69 =   OpVariable %_ptr_Function_float Function
         %82 =   OpVariable %_ptr_Function_float Function
         %93 =   OpVariable %_ptr_Function_float Function
        %105 =   OpVariable %_ptr_Function_float Function
        %116 =   OpVariable %_ptr_Function_float Function
          %b =   OpVariable %_ptr_Function_bool Function
          %c =   OpVariable %_ptr_Function_bool Function
        %129 =   OpVariable %_ptr_Function_bool Function
        %134 =   OpVariable %_ptr_Function_v4float Function
        %152 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %x %float_1
                 OpStore %y %float_1
                 OpSelectionMerge %36 None
                 OpBranchConditional %true %34 %35

         %34 =     OpLabel
         %37 =       OpFAdd %float %float_1 %float_1    ; RelaxedPrecision
                     OpStore %x %37
                     OpStore %33 %37
                     OpBranch %36

         %35 =     OpLabel
         %38 =       OpFAdd %float %float_1 %float_1    ; RelaxedPrecision
                     OpStore %y %38
                     OpStore %33 %38
                     OpBranch %36

         %36 = OpLabel
         %39 =   OpLoad %float %33                  ; RelaxedPrecision
         %40 =   OpLoad %float %x                   ; RelaxedPrecision
         %41 =   OpLoad %float %y                   ; RelaxedPrecision
         %42 =   OpFOrdEqual %bool %40 %41
                 OpSelectionMerge %46 None
                 OpBranchConditional %42 %44 %45

         %44 =     OpLabel
         %47 =       OpLoad %float %x               ; RelaxedPrecision
         %49 =       OpFAdd %float %47 %float_3     ; RelaxedPrecision
                     OpStore %x %49
                     OpStore %43 %49
                     OpBranch %46

         %45 =     OpLabel
         %50 =       OpLoad %float %y               ; RelaxedPrecision
         %51 =       OpFAdd %float %50 %float_3     ; RelaxedPrecision
                     OpStore %y %51
                     OpStore %43 %51
                     OpBranch %46

         %46 = OpLabel
         %52 =   OpLoad %float %43                  ; RelaxedPrecision
         %53 =   OpLoad %float %x                   ; RelaxedPrecision
         %54 =   OpLoad %float %y                   ; RelaxedPrecision
         %55 =   OpFOrdLessThan %bool %53 %54
                 OpSelectionMerge %59 None
                 OpBranchConditional %55 %57 %58

         %57 =     OpLabel
         %60 =       OpLoad %float %x               ; RelaxedPrecision
         %62 =       OpFAdd %float %60 %float_5     ; RelaxedPrecision
                     OpStore %x %62
                     OpStore %56 %62
                     OpBranch %59

         %58 =     OpLabel
         %63 =       OpLoad %float %y               ; RelaxedPrecision
         %64 =       OpFAdd %float %63 %float_5     ; RelaxedPrecision
                     OpStore %y %64
                     OpStore %56 %64
                     OpBranch %59

         %59 = OpLabel
         %65 =   OpLoad %float %56                  ; RelaxedPrecision
         %66 =   OpLoad %float %y                   ; RelaxedPrecision
         %67 =   OpLoad %float %x                   ; RelaxedPrecision
         %68 =   OpFOrdGreaterThanEqual %bool %66 %67
                 OpSelectionMerge %72 None
                 OpBranchConditional %68 %70 %71

         %70 =     OpLabel
         %73 =       OpLoad %float %x               ; RelaxedPrecision
         %75 =       OpFAdd %float %73 %float_9     ; RelaxedPrecision
                     OpStore %x %75
                     OpStore %69 %75
                     OpBranch %72

         %71 =     OpLabel
         %76 =       OpLoad %float %y               ; RelaxedPrecision
         %77 =       OpFAdd %float %76 %float_9     ; RelaxedPrecision
                     OpStore %y %77
                     OpStore %69 %77
                     OpBranch %72

         %72 = OpLabel
         %78 =   OpLoad %float %69                  ; RelaxedPrecision
         %79 =   OpLoad %float %x                   ; RelaxedPrecision
         %80 =   OpLoad %float %y                   ; RelaxedPrecision
         %81 =   OpFUnordNotEqual %bool %79 %80
                 OpSelectionMerge %85 None
                 OpBranchConditional %81 %83 %84

         %83 =     OpLabel
         %86 =       OpLoad %float %x               ; RelaxedPrecision
         %87 =       OpFAdd %float %86 %float_1     ; RelaxedPrecision
                     OpStore %x %87
                     OpStore %82 %87
                     OpBranch %85

         %84 =     OpLabel
         %88 =       OpLoad %float %y               ; RelaxedPrecision
                     OpStore %82 %88
                     OpBranch %85

         %85 = OpLabel
         %89 =   OpLoad %float %82                  ; RelaxedPrecision
         %90 =   OpLoad %float %x                   ; RelaxedPrecision
         %91 =   OpLoad %float %y                   ; RelaxedPrecision
         %92 =   OpFOrdEqual %bool %90 %91
                 OpSelectionMerge %96 None
                 OpBranchConditional %92 %94 %95

         %94 =     OpLabel
         %97 =       OpLoad %float %x               ; RelaxedPrecision
         %99 =       OpFAdd %float %97 %float_2     ; RelaxedPrecision
                     OpStore %x %99
                     OpStore %93 %99
                     OpBranch %96

         %95 =     OpLabel
        %100 =       OpLoad %float %y               ; RelaxedPrecision
                     OpStore %93 %100
                     OpBranch %96

         %96 = OpLabel
        %101 =   OpLoad %float %93                  ; RelaxedPrecision
        %102 =   OpLoad %float %x                   ; RelaxedPrecision
        %103 =   OpLoad %float %y                   ; RelaxedPrecision
        %104 =   OpFUnordNotEqual %bool %102 %103
                 OpSelectionMerge %108 None
                 OpBranchConditional %104 %106 %107

        %106 =     OpLabel
        %109 =       OpLoad %float %x               ; RelaxedPrecision
                     OpStore %105 %109
                     OpBranch %108

        %107 =     OpLabel
        %110 =       OpLoad %float %y               ; RelaxedPrecision
        %111 =       OpFAdd %float %110 %float_3    ; RelaxedPrecision
                     OpStore %y %111
                     OpStore %105 %111
                     OpBranch %108

        %108 = OpLabel
        %112 =   OpLoad %float %105                 ; RelaxedPrecision
        %113 =   OpLoad %float %x                   ; RelaxedPrecision
        %114 =   OpLoad %float %y                   ; RelaxedPrecision
        %115 =   OpFOrdEqual %bool %113 %114
                 OpSelectionMerge %119 None
                 OpBranchConditional %115 %117 %118

        %117 =     OpLabel
        %120 =       OpLoad %float %x               ; RelaxedPrecision
                     OpStore %116 %120
                     OpBranch %119

        %118 =     OpLabel
        %121 =       OpLoad %float %y               ; RelaxedPrecision
        %123 =       OpFAdd %float %121 %float_4    ; RelaxedPrecision
                     OpStore %y %123
                     OpStore %116 %123
                     OpBranch %119

        %119 = OpLabel
        %124 =   OpLoad %float %116                 ; RelaxedPrecision
                 OpStore %b %true
                 OpStore %b %false
                 OpSelectionMerge %132 None
                 OpBranchConditional %false %130 %131

        %130 =     OpLabel
                     OpStore %129 %false
                     OpBranch %132

        %131 =     OpLabel
                     OpStore %129 %false
                     OpBranch %132

        %132 = OpLabel
        %133 =   OpLoad %bool %129
                 OpStore %c %133
                 OpSelectionMerge %138 None
                 OpBranchConditional %133 %136 %137

        %136 =     OpLabel
        %139 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %142 =       OpLoad %v4float %139           ; RelaxedPrecision
                     OpStore %134 %142
                     OpBranch %138

        %137 =     OpLabel
        %143 =       OpLoad %float %x               ; RelaxedPrecision
        %145 =       OpFOrdEqual %bool %143 %float_8
                     OpSelectionMerge %147 None
                     OpBranchConditional %145 %146 %147

        %146 =         OpLabel
        %148 =           OpLoad %float %y           ; RelaxedPrecision
        %150 =           OpFOrdEqual %bool %148 %float_17
                         OpBranch %147

        %147 =     OpLabel
        %151 =       OpPhi %bool %false %137 %150 %146
                     OpSelectionMerge %155 None
                     OpBranchConditional %151 %153 %154

        %153 =         OpLabel
        %156 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %158 =           OpLoad %v4float %156       ; RelaxedPrecision
                         OpStore %152 %158
                         OpBranch %155

        %154 =         OpLabel
        %159 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %160 =           OpLoad %v4float %159       ; RelaxedPrecision
                         OpStore %152 %160
                         OpBranch %155

        %155 =     OpLabel
        %161 =       OpLoad %v4float %152           ; RelaxedPrecision
                     OpStore %134 %161
                     OpBranch %138

        %138 = OpLabel
        %162 =   OpLoad %v4float %134               ; RelaxedPrecision
                 OpReturnValue %162
               OpFunctionEnd
