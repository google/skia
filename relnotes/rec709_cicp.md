Change `SkNamedTransferFn::kRec709` to match the pure gamma 2.4 definition from
ITU-R BT.1886.

Apply this to transfer characteristics values 1, 6, 11, 14, and 16, since they
use the same definition.

Add reference text to the comments to clarify that this comes from the EOTF
definition, and that the ITU-T H.273 table 3 function definitions are not
necessarily inverse EOTFs, but are sometimes OETFs (as is the case for
`kRec709`).
