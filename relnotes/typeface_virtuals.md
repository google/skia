Virtuals in `SkTypeface` subclasses (5 of them) now take SkSpan instead of ptr/count. This
is part of the larger change where public APIs are being converted to take SkSpan where
applicable.

No real functionality change, but this new signature allows some of the methods to perform
range-checking, whereas before they could not.
