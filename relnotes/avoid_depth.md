Add skgpu::graphite::ContextOptions::fAvoidDepth. Enabling this will lead
graphite to avoid using the depth/stencil buffer, and fallback to analytic path
and turn off depth occlusion culling when necessary.
