// Copyright 2021 Google LLC.

#include "experimental/sktext/src/Wrapper.h"

namespace skia {
namespace text {

bool Wrapper::process() {

    // TODO: define max line number based on this->fHeight
    return this->breakTextIntoLines(this->fWidth);
}

Range Wrapper::glyphRange(const TextRun* run, const Range& textRange) {
    Range glyphRange = EMPTY_RANGE;
    for (size_t i = 0; i < run->fClusters.size(); ++i) {
        auto cluster = run->fClusters[i];
        if (cluster < textRange.fStart) continue;
        if (cluster > textRange.fEnd) break;

        if (glyphRange.fStart == EMPTY_INDEX) {
            glyphRange.fStart = i;
        }
        glyphRange.fEnd = i;
    }
    return glyphRange;
}

Range Wrapper::textRange(const TextRun* run, const Range& glyphRange) {
    Range textRange = EMPTY_RANGE;
    for (size_t i = 0; i < run->fClusters.size(); ++i) {
        auto cluster = run->fClusters[i];
        if (i < glyphRange.fStart) continue;
        if (i > glyphRange.fEnd) break;

        if (textRange.fStart == EMPTY_INDEX) {
            textRange.fStart = cluster;
        }
        textRange.fEnd = cluster;
    }
    return textRange;
}

bool Wrapper::breakTextIntoLines(SkScalar width) {

    // line : spaces : clusters
    Stretch line;
    Stretch spaces;
    Stretch clusters;

    for (size_t runIndex = 0; runIndex < fProcessor->fRuns.size(); ++runIndex ) {

        auto& run = fProcessor->fRuns[runIndex];
        TextMetrics runMetrics(run.fFont);
        Stretch cluster;
        if (!run.leftToRight()) {
            cluster.fTextRange = { run.fUtf8Range.end(), run.fUtf8Range.end()};
        }

        for (size_t glyphIndex = 0; glyphIndex < run.fPositions.size(); ++glyphIndex) {
            auto textIndex = run.fClusters[glyphIndex];

            if (cluster.isEmpty()) {
                cluster = Stretch(GlyphPos(runIndex, glyphIndex), textIndex, runMetrics);
                continue;
            /*
            } else if (cluster.fTextRange.fStart == textIndex) {
                break;
            } else if (cluster.fTextRange.fEnd == textIndex) {
                break;
            */
          }

          // The entire cluster belongs to a single run
          SkASSERT(cluster.fGlyphStart.fRunIndex == runIndex);

          auto clusterWidth = run.fPositions[glyphIndex].fX - run.fPositions[cluster.fGlyphStart.fGlyphIndex].fX;
          cluster.finish(glyphIndex, textIndex, clusterWidth);

          auto isHardLineBreak = fProcessor->isHardLineBreak(cluster.fTextRange.fStart);
          auto isSoftLineBreak = fProcessor->isSoftLineBreak(cluster.fTextRange.fStart);
          auto isWhitespaces = fProcessor->isWhitespaces(cluster.fTextRange);
          auto isEndOfText = run.leftToRight() ? textIndex == run.fUtf8Range.end() : textIndex == run.fUtf8Range.begin();

          if (isHardLineBreak || isEndOfText || isSoftLineBreak || isWhitespaces) {
              if (!clusters.isEmpty()) {
                  line.moveTo(spaces);
                  line.moveTo(clusters);
              }
              if (isWhitespaces) {
                  spaces.moveTo(cluster);
              }
              if (isHardLineBreak) {
                  this->addLine(line, spaces);
                  break;
              }
              if (isEndOfText) {
                  line.moveTo(cluster);
                  if (!line.isEmpty()) {
                      this->addLine(line, spaces);
                  }
                  break;
              }
          }

          if (!SkScalarIsFinite(width)) {
              clusters.moveTo(cluster);
          } else if ((line.fWidth + spaces.fWidth + clusters.fWidth + cluster.fWidth) <= width) {
              clusters.moveTo(cluster);
          } else {
              // Wrapping the text by whitespaces
              if (line.isEmpty()) {
                  if (clusters.isEmpty()) {
                      line.moveTo(cluster);
                  } else {
                      line.moveTo(clusters);
                  }
              }
              this->addLine(line, spaces);
              clusters.moveTo(cluster);
          }
          cluster = Stretch(GlyphPos(runIndex, glyphIndex), textIndex, runMetrics);
        }
    }

    return true;
}

/*



type SingleRunSingleStyleCallback = (style, glyphRange) => number;





  breakRunIntoStyles(inputs, styleFlags : TextStyleFlagsEnum, runStart, run, glyphRange, callback: SingleRunSingleStyleCallback) {

      let lastFlags = TextStyleFlagsEnum.kNoStyle;
      let styleRange;
      let index = 0;
      for (let flags of this.fTextStyles) {
          let currentFlags = lastFlags = flags & styleFlags;
          if (currentFlags == lastFlags) {
              styleRange.end = index;
          } else {
              callback();
              lastFlags = currentFlags;
              styleRange = { start: index, end: index};
          }
      }

      const textRange = this.glyphRange(run, glyphRange);
      for (let block of inputs.blocks) {

          const intersect = this.intersect(block.textRange, textRange);
          if (this.width(intersect) == 0) {
              continue;
          }

          const glyphRange = this.glyphRange(run, intersect);
          runStart += callback(block.style, glyphRange);
      }
  }

  generateDrawingOperations(inputs, outputs) {

    const textLayout = this;
    // TODO: Sort runs on the line accordingly to the visual order (for LTR/RTL mixture)
    // Iterate through all the runs on the line
    let runStart = 0.0;
    let lineVerticalStart = 0.0;
    this.fLines.forEach(function (line, lineIndex) {
      var run = null;
      var lastRunIndex;
      for (var runIndex = line.textStart.runIndex; runIndex <= line.textEnd.runIndex; ++runIndex) {
        if (run != null && lastRunIndex != runIndex) {
            continue;
        }
        run = inputs.runs[runIndex];
        lastRunIndex = runIndex;
        let runGlyphRange = {
            start: line.textStart.runIndex ? line.textStart.glyphIndex : 0,
            end: line.textEnd.runIndex ? line.textEnd.glyphIndex : run.glyphs.size()
        }

        // Iterate through all the styles in the run
        // TODO: For now assume that the style edges are cluster edges and don't fall between clusters
        // TODO: Just show the text, not decorations of any kind
        textLayout.breakRunIntoStyles(inputs, runStart, run, runGlyphRange, function(style, glyphRange) {
          // left, top, right, bottom
          let runWidth = textLayout.glyphRangeWidth(run, glyphRange);
          if (style.background == undefined) {
            return runWidth;
          }
          let rectangle = {
              backgroundColor: style.background,
              left: runStart,
              top: lineVerticalStart,
              width: runWidth,
              height: run.font.metrics.ascent + run.font.metrics.descent + run.font.metrics.leading
          };
          outputs.rectangles.push(rectangle);
          return runWidth;
        });

        textLayout.breakRunIntoStyles(inputs, runStart, run, runGlyphRange, function(style, glyphRange) {
          // TODO: Ignore clipping cluster by position for now
          let textBlob = {
              foregroundColor: style.foreground,
              run : runIndex,
              glyphRange: glyphRange,
              shift: {
                  x: runStart,
                  y: lineVerticalStart
              }
          };
          outputs.textBlobs.push(textBlob);

          return textLayout.glyphRangeWidth(run, glyphRange);
        });
      }
    });
  }
}
*/

} // namespace text
} // namespace skia
