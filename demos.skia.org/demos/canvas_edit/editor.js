/**
 * CanvasPathEditor: Core text editing engine and renderer.
 * Leverages Canvas 2D TextMetrics, TextClusters, getSelectionRects, and
 * getIndexFromOffset to layout and edit text along arbitrary PathGuides.
 */

export class CanvasPathEditor {
  constructor(text, pathGuide) {
    this.text = text;
    this.path = pathGuide;

    // Caret and selection indices (grapheme/character indices)
    this.cursorIndex = text.length;
    this.selectionStart = text.length;
    this.selectionEnd = text.length;

    // Typography & style tokens
    this.fontFamily = 'Inter, system-ui, -apple-system, sans-serif';
    this.fontSize = 60;
    this.fontWeight = '600';
    this.fontStyle = 'normal';
    this.color = '#f8fafc';
    this.textAlign = 'center';
    this.textBaseline = 'alphabetic';

    // Animation & UI states
    this.lastInputTime = Date.now();
    this.isFocused = true;
    this.experimentalApiDetected = false;
    this.forceFallback = false;
  }

  /**
   * Apply current typography settings to canvas context.
   * @param {CanvasRenderingContext2D} ctx
   */
  applyStyles(ctx) {
    ctx.font = `${this.fontStyle} ${this.fontWeight} ${this.fontSize}px ${this.fontFamily}`;
    ctx.fillStyle = this.color;
    ctx.textAlign = 'left'; // Always use left for consistent string/cluster coordinate origins
    ctx.textBaseline = this.textBaseline;
  }

  /**
   * Check if experimental APIs are available on context.
   * @param {CanvasRenderingContext2D} ctx
   */
  checkExperimentalApis(ctx) {
    if (this.forceFallback) {
      this.experimentalApiDetected = false;
      return false;
    }
    this.applyStyles(ctx);
    const metrics = ctx.measureText(this.text || ' ');
    this.experimentalApiDetected =
      typeof metrics.getTextClusters === 'function' &&
      typeof ctx.fillTextCluster === 'function';
    return this.experimentalApiDetected;
  }

  /**
   * Set active path geometry.
   */
  setPath(pathGuide) {
    this.path = pathGuide;
  }

  /**
   * Set cursor index and optionally update selection.
   */
  setCursor(index, extendSelection = false) {
    this.cursorIndex = Math.max(0, Math.min(this.text.length, index));
    if (extendSelection) {
      this.selectionEnd = this.cursorIndex;
    } else {
      this.selectionStart = this.cursorIndex;
      this.selectionEnd = this.cursorIndex;
    }
    this.lastInputTime = Date.now();
  }

  /**
   * Move cursor by delta (grapheme/character step).
   */
  moveCursor(delta, extendSelection = false) {
    if (!extendSelection && this.hasSelection()) {
      // Collapse selection
      this.setCursor(delta < 0 ? Math.min(this.selectionStart, this.selectionEnd)
                               : Math.max(this.selectionStart, this.selectionEnd));
      return;
    }
    this.setCursor(this.cursorIndex + delta, extendSelection);
  }

  hasSelection() {
    return this.selectionStart !== this.selectionEnd;
  }

  getSelectionRange() {
    return [
      Math.min(this.selectionStart, this.selectionEnd),
      Math.max(this.selectionStart, this.selectionEnd)
    ];
  }

  selectAll() {
    this.selectionStart = 0;
    this.selectionEnd = this.text.length;
    this.cursorIndex = this.text.length;
    this.lastInputTime = Date.now();
  }

  getRunStart(metrics) {
    return 0;
  }

  getRunEnd(metrics) {
    return metrics ? (metrics.width || 0) : 0;
  }

  getBaselineVerticalMetrics() {
    const fs = this.fontSize;
    switch (this.textBaseline) {
      case 'middle':
        return { top: -fs * 0.55, bottom: fs * 0.55, height: fs * 1.1 };
      case 'top':
        return { top: -fs * 0.1, bottom: fs * 1.0, height: fs * 1.1 };
      case 'bottom':
        return { top: -fs * 1.0, bottom: fs * 0.1, height: fs * 1.1 };
      case 'alphabetic':
      default:
        return { top: -fs * 0.85, bottom: fs * 0.25, height: fs * 1.1 };
    }
  }

  canFitText(ctx, candidateText) {
    this.applyStyles(ctx);
    const width = ctx.measureText(candidateText || '').width;
    const maxLen = this.path && typeof this.path.getMaxLength === 'function'
      ? this.path.getMaxLength()
      : Infinity;
    return width <= maxLen;
  }

  /**
   * Insert text at cursor position or replace active selection.
   */
  insertText(ctx, str) {
    if (this.hasSelection()) {
      this.deleteSelection();
    }
    const before = this.text.slice(0, this.cursorIndex);
    const after = this.text.slice(this.cursorIndex);
    const candidate = before + str + after;
    if (!this.canFitText(ctx, candidate)) return false;
    this.text = candidate;
    this.setCursor(before.length + str.length);
    return true;
  }

  /**
   * Delete character before (-1) or after (+1) cursor, or delete active selection.
   */
  deleteText(direction = -1) {
    if (this.hasSelection()) {
      this.deleteSelection();
      return;
    }
    if (direction < 0 && this.cursorIndex > 0) {
      const before = this.text.slice(0, this.cursorIndex - 1);
      const after = this.text.slice(this.cursorIndex);
      this.text = before + after;
      this.setCursor(this.cursorIndex - 1);
    } else if (direction > 0 && this.cursorIndex < this.text.length) {
      const before = this.text.slice(0, this.cursorIndex);
      const after = this.text.slice(this.cursorIndex + 1);
      this.text = before + after;
      this.setCursor(this.cursorIndex);
    }
  }

  deleteSelection() {
    if (!this.hasSelection()) return;
    const [start, end] = this.getSelectionRange();
    this.text = this.text.slice(0, start) + this.text.slice(end);
    this.setCursor(start);
  }

  isClusterRtl(clusters, i) {
    if (!clusters || clusters.length <= 1) return false;
    if (i < clusters.length - 1) {
      return clusters[i].start > clusters[i + 1].start;
    }
    if (i > 0) {
      return clusters[i - 1].start > clusters[i].start;
    }
    return false;
  }

  /**
   * Project canvas point (px, py) to character index.
   */
  setCursorAtPoint(ctx, px, py, extendSelection = false) {
    this.applyStyles(ctx);
    const metrics = ctx.measureText(this.text || ' ');
    const s = this.path.getLengthAtPoint(px, py, this.textAlign, metrics.width || 0);

    let targetIndex = 0;
    if (this.text.length > 0 && !this.forceFallback && metrics &&
        typeof metrics.getIndexFromOffset === 'function') {
      targetIndex = metrics.getIndexFromOffset(s);
    } else if (this.text.length > 0 && !this.forceFallback && metrics &&
               typeof metrics.getTextClusters === 'function') {
      const clusters = metrics.getTextClusters();
      let bestDist = Infinity;
      for (let i = 0; i < clusters.length; i++) {
        const cl = clusters[i];
        const nextX = i + 1 < clusters.length ? clusters[i + 1].x : (metrics.width || 0);
        const isRtl = this.isClusterRtl(clusters, i);

        const leftIdx = isRtl ? cl.end : cl.start;
        const rightIdx = isRtl ? cl.start : cl.end;

        const distLeft = Math.abs(cl.x - s);
        if (distLeft < bestDist) {
          bestDist = distLeft;
          targetIndex = leftIdx;
        }
        const distRight = Math.abs(nextX - s);
        if (distRight < bestDist) {
          bestDist = distRight;
          targetIndex = rightIdx;
        }
      }
    } else {
      // Linear advance fallback estimation
      let bestDist = Infinity;
      let curS = this.getRunStart(metrics);
      for (let i = 0; i <= this.text.length; i++) {
        const dist = Math.abs(curS - s);
        if (dist < bestDist) {
          bestDist = dist;
          targetIndex = i;
        }
        if (i < this.text.length) {
          curS += ctx.measureText(this.text[i]).width;
        }
      }
    }

    this.setCursor(targetIndex, extendSelection);
  }

  /**
   * Compute arc length s for a given character index.
   */
  getArcLengthForIndex(ctx, index) {
    this.applyStyles(ctx);
    const metrics = ctx.measureText(this.text);
    if (this.text.length === 0 || index <= 0) return this.getRunStart(metrics);
    if (index >= this.text.length) return this.getRunEnd(metrics);

    if (this.text.length > 0 && !this.forceFallback && metrics &&
        typeof metrics.getTextClusters === 'function') {
      const clusters = metrics.getTextClusters();
      for (let i = 0; i < clusters.length; i++) {
        const cl = clusters[i];
        const nextX = i + 1 < clusters.length ? clusters[i + 1].x : (metrics.width || 0);
        const isRtl = this.isClusterRtl(clusters, i);

        if (index === cl.start) {
          return isRtl ? nextX : cl.x;
        }
        if (index === cl.end) {
          return isRtl ? cl.x : nextX;
        }
        if (cl.start < index && index < cl.end) {
          const t = (index - cl.start) / (cl.end - cl.start);
          return isRtl ? nextX - t * (nextX - cl.x) : cl.x + t * (nextX - cl.x);
        }
      }
      return this.getRunEnd(metrics);
    }

    // Fallback estimation
    let s = this.getRunStart(metrics);
    for (let i = 0; i < index && i < this.text.length; i++) {
      s += ctx.measureText(this.text[i]).width;
    }
    return s;
  }

  /**
   * Render path guide, selection highlights, text clusters, and caret.
   * @param {CanvasRenderingContext2D} ctx
   */
  draw(ctx) {
    ctx.save();
    this.applyStyles(ctx);
    const metrics = ctx.measureText(this.text);

    // 1. Draw Path Guide
    this.path.drawGuide(ctx);

    let clusters = [];
    if (this.text.length > 0 && !this.forceFallback && metrics &&
        typeof metrics.getTextClusters === 'function') {
      clusters = metrics.getTextClusters();
    }

    // 2. Draw Selection Highlights
    if (this.hasSelection()) {
      const [selStart, selEnd] = this.getSelectionRange();
      const vm = this.getBaselineVerticalMetrics();

      if (this.text.length > 0 && !this.forceFallback && metrics &&
          typeof metrics.getSelectionRects === 'function') {
        const rects = metrics.getSelectionRects(selStart, selEnd);
        for (const r of rects) {
          this.drawCurvedHighlight(ctx, r.x, r.x + r.width, r.y || vm.top, r.height || vm.height);
        }
      } else if (clusters.length > 0) {
        // Fallback using clusters
        for (let i = 0; i < clusters.length; i++) {
          const cl = clusters[i];
          if (cl.start < selEnd && cl.end > selStart) {
            const nextX = i + 1 < clusters.length ? clusters[i + 1].x : metrics.width;
            this.drawCurvedHighlight(ctx, cl.x, nextX, vm.top, vm.height);
          }
        }
      } else {
        const s1 = this.getArcLengthForIndex(ctx, selStart);
        const s2 = this.getArcLengthForIndex(ctx, selEnd);
        this.drawCurvedHighlight(ctx, s1, s2, vm.top, vm.height);
      }
    }

    // 3. Draw Text Clusters
    if (clusters.length > 0) {
      for (let i = 0; i < clusters.length; i++) {
        const cluster = clusters[i];
        const pt = this.path.getPointAtLength(cluster.x, this.textAlign, metrics.width || 0);

        ctx.save();
        ctx.translate(pt.x, pt.y);
        ctx.rotate(pt.angle);

        if (!this.forceFallback && typeof ctx.fillTextCluster === 'function') {
          ctx.fillTextCluster(cluster, -cluster.x, -cluster.y);
        } else {
          // Fallback rendering
          ctx.fillText(this.text.slice(cluster.start, cluster.end), 0, 0);
        }
        ctx.restore();
      }
    } else if (this.text.length > 0) {
      // Fallback if getTextClusters is unsupported: render char by char
      let s = 0;
      for (let i = 0; i < this.text.length; i++) {
        const char = this.text[i];
        const pt = this.path.getPointAtLength(s, this.textAlign, metrics.width || 0);
        ctx.save();
        ctx.translate(pt.x, pt.y);
        ctx.rotate(pt.angle);
        ctx.fillText(char, 0, 0);
        ctx.restore();
        s += ctx.measureText(char).width;
      }
    }

    // 4. Draw Caret
    if (this.isFocused) {
      const elapsed = (Date.now() - this.lastInputTime) % 1000;
      if (elapsed < 550) {
        const sCaret = this.getArcLengthForIndex(ctx, this.cursorIndex);
        const pt = this.path.getPointAtLength(sCaret, this.textAlign, metrics.width || 0);
        const vm = this.getBaselineVerticalMetrics();

        ctx.save();
        ctx.translate(pt.x, pt.y);
        ctx.rotate(pt.angle);
        ctx.beginPath();
        ctx.moveTo(0, vm.top);
        ctx.lineTo(0, vm.bottom);
        ctx.strokeStyle = '#38bdf8';
        ctx.lineWidth = 2.5;
        ctx.lineCap = 'round';
        ctx.shadowColor = '#0284c7';
        ctx.shadowBlur = 8;
        ctx.stroke();
        ctx.restore();
      }
    }

    ctx.restore();
  }

  /**
   * Helper to draw curved arc selection highlight between arc lengths s1 and s2.
   */
  drawCurvedHighlight(ctx, s1, s2, yOffset = -24, height = 36) {
    if (Math.abs(s2 - s1) < 0.5) return;
    const minS = Math.min(s1, s2);
    const maxS = Math.max(s1, s2);
    const steps = Math.max(2, Math.ceil((maxS - minS) / 8));
    const ds = (maxS - minS) / steps;

    const topPoints = [];
    const bottomPoints = [];

    const totalWidth = ctx.measureText(this.text || '').width || 0;

    for (let i = 0; i <= steps; i++) {
      const s = minS + i * ds;
      const pt = this.path.getPointAtLength(s, this.textAlign, totalWidth);
      const cosA = Math.cos(pt.angle);
      const sinA = Math.sin(pt.angle);

      // Transform local (0, yOffset) and (0, yOffset + height) by pt.angle
      topPoints.push({
        x: pt.x - sinA * yOffset,
        y: pt.y + cosA * yOffset
      });
      bottomPoints.push({
        x: pt.x - sinA * (yOffset + height),
        y: pt.y + cosA * (yOffset + height)
      });
    }

    ctx.save();
    ctx.beginPath();
    ctx.moveTo(topPoints[0].x, topPoints[0].y);
    for (let i = 1; i < topPoints.length; i++) {
      ctx.lineTo(topPoints[i].x, topPoints[i].y);
    }
    for (let i = bottomPoints.length - 1; i >= 0; i--) {
      ctx.lineTo(bottomPoints[i].x, bottomPoints[i].y);
    }
    ctx.closePath();
    ctx.fillStyle = 'rgba(56, 189, 248, 0.35)';
    ctx.fill();
    ctx.restore();
  }
}
