`SkFloat2Bits` and `SkBits2Float` have been removed from the Skia public headers. These were always
private API (since they lived in `/include/private`) but they had leaked into some example code, and
tended to be available once a handful of Skia headers were #included.
