
The SkExecutor class is now templated so it can support different work submission models.

The prior std::function-based model is preserved as SkFunctionalExecutor and SkExecutor.

A new callback-style model is now supported with SkCallbackExecutor.

The new SK_GRAPHITE_USE_CALLBACK_EXECUTOR #define can be used to make Graphite take an SkCallbackExecutor in its ContextOptions. Ganesh continues to take an SkFunctionalExecutor.
