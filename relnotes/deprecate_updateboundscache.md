Since SkPath data is now immutable and we always compute the bounds
upfront, SkPath::updateBoundsCache() no longer serves any purpose
and has been removed.
