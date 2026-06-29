/**
 * Path geometry abstractions for laying out canvas text along curved arcs.
 * Supports CirclePath, WavePath, and SpiralPath with accurate arc-length
 * parameterization and point-to-length projection.
 */

export class PathGuide {
  constructor(name) {
    this.name = name;
  }

  /**
   * Get point and tangent angle at arc length s.
   * @param {number} s - Arc length along the path.
   * @returns {{x: number, y: number, angle: number}}
   */
  getPointAtLength(s) {
    throw new Error('Not implemented');
  }

  /**
   * Find nearest arc length s for given canvas point (px, py).
   * @param {number} px
   * @param {number} py
   * @returns {number}
   */
  getLengthAtPoint(px, py) {
    throw new Error('Not implemented');
  }

  /**
   * Draw the visual path guide on canvas.
   * @param {CanvasRenderingContext2D} ctx
   */
  drawGuide(ctx) {
    throw new Error('Not implemented');
  }

  getMaxLength() {
    return Infinity;
  }
}

export class CirclePath extends PathGuide {
  constructor(cx, cy, radius, startAngle = -Math.PI / 2) {
    super('circle');
    this.cx = cx;
    this.cy = cy;
    this.radius = radius;
    this.startAngle = startAngle;
  }

  getMaxLength() {
    return Math.PI * 2 * this.radius - 16;
  }

  getAlignOffset(textAlign, totalWidth = 0) {
    if (textAlign === 'center') return -totalWidth * 0.5;
    if (textAlign === 'right') return -totalWidth;
    return 0;
  }

  getPointAtLength(s, textAlign = 'left', totalWidth = 0) {
    const targetS = s + this.getAlignOffset(textAlign, totalWidth);
    const theta = this.startAngle + targetS / this.radius;
    return {
      x: this.cx + this.radius * Math.cos(theta),
      y: this.cy + this.radius * Math.sin(theta),
      angle: theta + Math.PI / 2
    };
  }

  getLengthAtPoint(px, py, textAlign = 'left', totalWidth = 0) {
    let theta = Math.atan2(py - this.cy, px - this.cx);
    let delta = theta - this.startAngle;
    if (textAlign === 'center') {
      while (delta <= -Math.PI) delta += Math.PI * 2;
      while (delta > Math.PI) delta -= Math.PI * 2;
    } else if (textAlign === 'right') {
      while (delta > 0) delta -= Math.PI * 2;
      while (delta <= -Math.PI * 2) delta += Math.PI * 2;
    } else {
      while (delta < 0) delta += Math.PI * 2;
      while (delta >= Math.PI * 2) delta -= Math.PI * 2;
    }
    return delta * this.radius - this.getAlignOffset(textAlign, totalWidth);
  }

  drawGuide(ctx) {
    ctx.save();
    ctx.beginPath();
    ctx.arc(this.cx, this.cy, this.radius, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(168, 85, 247, 0.3)';
    ctx.lineWidth = 2;
    ctx.setLineDash([6, 6]);
    ctx.stroke();
    ctx.restore();
  }
}

/**
 * Base class for paths using discrete arc-length sampling and lookup tables.
 */
class SampledPath extends PathGuide {
  constructor(name, samples) {
    super(name);
    this.samples = samples; // Array of { s, x, y, angle }
  }

  getMaxLength() {
    if (!this.samples || this.samples.length < 2) return Infinity;
    return this.samples[this.samples.length - 1].s - this.samples[0].s - 16;
  }

  getAlignOffset(textAlign, totalWidth = 0) {
    if (!this.samples || this.samples.length === 0) return 0;
    const totalLength = this.samples[this.samples.length - 1].s;
    if (textAlign === 'center') return totalLength * 0.5 - totalWidth * 0.5;
    if (textAlign === 'right') return totalLength - totalWidth;
    return 0;
  }

  getPointAtLength(s, textAlign = 'left', totalWidth = 0) {
    const targetS = s + this.getAlignOffset(textAlign, totalWidth);
    if (this.samples.length === 0) return { x: 0, y: 0, angle: 0 };
    if (targetS <= this.samples[0].s) return this.samples[0];
    if (targetS >= this.samples[this.samples.length - 1].s) return this.samples[this.samples.length - 1];

    // Binary search
    let low = 0;
    let high = this.samples.length - 1;
    while (low <= high) {
      const mid = (low + high) >> 1;
      if (this.samples[mid].s < targetS) {
        low = mid + 1;
      } else if (this.samples[mid].s > targetS) {
        high = mid - 1;
      } else {
        return this.samples[mid];
      }
    }

    // Interpolate between high and low
    const p1 = this.samples[high];
    const p2 = this.samples[low];
    const t = (targetS - p1.s) / (p2.s - p1.s);
    return {
      x: p1.x + (p2.x - p1.x) * t,
      y: p1.y + (p2.y - p1.y) * t,
      angle: p1.angle + (p2.angle - p1.angle) * t
    };
  }

  getLengthAtPoint(px, py, textAlign = 'left', totalWidth = 0) {
    let bestS = 0;
    let minDistSq = Infinity;
    for (let i = 0; i < this.samples.length; i++) {
      const dx = px - this.samples[i].x;
      const dy = py - this.samples[i].y;
      const distSq = dx * dx + dy * dy;
      if (distSq < minDistSq) {
        minDistSq = distSq;
        bestS = this.samples[i].s;
      }
    }
    return bestS - this.getAlignOffset(textAlign, totalWidth);
  }

  drawGuide(ctx) {
    if (this.samples.length < 2) return;
    ctx.save();
    ctx.beginPath();
    ctx.moveTo(this.samples[0].x, this.samples[0].y);
    for (let i = 1; i < this.samples.length; i++) {
      ctx.lineTo(this.samples[i].x, this.samples[i].y);
    }
    ctx.strokeStyle = 'rgba(168, 85, 247, 0.3)';
    ctx.lineWidth = 2;
    ctx.setLineDash([6, 6]);
    ctx.stroke();
    ctx.restore();
  }
}

export class WavePath extends SampledPath {
  constructor(cx, cy, amplitude = 140, wavelength = 600, span = 1200) {
    const samples = [];
    const k = (Math.PI * 2) / wavelength;
    const startX = cx - span / 2;
    const endX = cx + span / 2;
    const steps = 2000;
    const dx = (endX - startX) / steps;

    let totalS = 0; // Start arc length s=0 at beginning of wave
    let prevX = startX;
    let prevY = cy - amplitude * Math.cos(k * (startX - cx));

    for (let i = 0; i <= steps; i++) {
      const x = startX + i * dx;
      const y = cy - amplitude * Math.cos(k * (x - cx));
      if (i > 0) {
        const dX = x - prevX;
        const dY = y - prevY;
        totalS += Math.hypot(dX, dY);
      }
      const dyDx = amplitude * k * Math.sin(k * (x - cx));
      const angle = Math.atan2(dyDx, 1);
      samples.push({ s: totalS, x, y, angle });
      prevX = x;
      prevY = y;
    }
    super('wave', samples);
    this.cx = cx;
    this.cy = cy;
    this.amplitude = amplitude;
    this.wavelength = wavelength;
  }
}

export class SpiralPath extends SampledPath {
  constructor(cx, cy, r0 = 40, b = 15) {
    const samples = [];
    const minTheta = -Math.PI;
    const maxTheta = Math.PI * 5;
    const steps = 2500;
    const dTheta = (maxTheta - minTheta) / steps;

    let totalS = 0;
    let prevX = cx + (r0 + b * minTheta) * Math.cos(minTheta);
    let prevY = cy + (r0 + b * minTheta) * Math.sin(minTheta);

    for (let i = 0; i <= steps; i++) {
      const theta = minTheta + i * dTheta;
      const r = r0 + b * theta;
      const x = cx + r * Math.cos(theta);
      const y = cy + r * Math.sin(theta);
      if (i > 0) {
        totalS += Math.hypot(x - prevX, y - prevY);
      }
      // Tangent vector for Archimedean spiral: dr/dtheta = b
      // dx/dtheta = b cos(theta) - r sin(theta)
      // dy/dtheta = b sin(theta) + r cos(theta)
      const dxdt = b * Math.cos(theta) - r * Math.sin(theta);
      const dydt = b * Math.sin(theta) + r * Math.cos(theta);
      const angle = Math.atan2(dydt, dxdt);
      samples.push({ s: totalS, x, y, angle });
      prevX = x;
      prevY = y;
    }
    super('spiral', samples);
    this.cx = cx;
    this.cy = cy;
  }
}
