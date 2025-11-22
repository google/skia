
The existing ContextOptions `PipelineCallback` has been deprecated in favor of the new `PipelineCachingCallback`.

The new callback provides extra information to the user allowing determination of how often a Pipeline is used and if any Precompiled Pipelines were unused. This information can be used to create a more effective set of Precompile PaintOptions.


