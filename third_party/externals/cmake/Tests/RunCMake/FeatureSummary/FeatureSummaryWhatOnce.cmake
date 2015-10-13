include(FeatureSummary)

set(WITH_FOO 1)

add_feature_info(Foo WITH_FOO "Foo decscription.")
add_feature_info(Foo WITH_FOO "Foo decscription.")

feature_summary(WHAT ENABLED_FEATURES)
