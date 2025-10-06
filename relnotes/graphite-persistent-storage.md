
A new persistent pipeline storage feature has been added to Graphite. For now, it is only relevant to Graphite's native Vulkan backend. The API consists of:

1) A new PersistentPipelineStorage abstract base class which can be implemented to persist Pipeline data across Context lifetimes.

2) A matching ContextOptions::fPersistentPipelineStorage member variable which can be used to pass the PersistentPipelineStorage-derived object to Graphite.

3) A Context::syncPipelineData method that, when possible, passes the current Pipeline data to the ContextOptions::fPersistentPipelineStorage object.
