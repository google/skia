Graphite has a new `ContextOptions::fRequiredOrderedRecordings` flag that enables certain optimizations when the
client knows that recordings are played back in order. Otherwise Graphite will need to clear some caches at the
start of each recording to ensure proper playback, which can significantly affect performance.

This replaces the old `ContextOptions::fDisableCachedGlyphUploads` flag which was deprecated but still being used by some
clients to get the same behavior.
