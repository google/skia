Change `SkNamedTransferFn::kHLG` and `SkNamedTransferFn::kPQ` to use the
new skcms representations.

This will have the side-effect of changing `SkColorSpace::MakeCICP` to
use the new representations.
