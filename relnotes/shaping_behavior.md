By default, //modules/skottie and //modules/svg will use primitive text shaping.
Clients that wish to use harfbuzz/icu for more correct shaping will need to
use one of the builders and call `setTextShapingFactory` with a newly-created
`SkShapers::Factory` implementation during construction.

For ease of configuration, `modules/skshaper/utils/FactoryHelpers.h` can be used
to provide this, but only if the client is depending on the correct skshaper
and skunicode modules (which should set defines such as `SK_SHAPER_HARFBUZZ_AVAILABLE`).

For example `builder.setTextShapingFactory(SkShapers::BestAvailable())` will use
Harfbuzz or CoreText for shaping if they were compiled in to the clients binary.