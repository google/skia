`ContextOptions` now contains an `fUserDefinedKnownRuntimeEffects` member variable.
Clients can add `SkRuntimeEffects` to this `SkSpan` and have them be registered as *known*
runtime effects. Such runtime effects can then be represented in the serialized Pipeline keys.
