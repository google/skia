`SkMergePathEffect`, `SkMatrixPathEffect`, `SkStrokePathEffect`, and
`SkStrokeAndFillPathEffect` have been removed from the public API.
These effects can be implemented on the SkPath objects directly using other means and clients
will likely find performance boosts by doing so.