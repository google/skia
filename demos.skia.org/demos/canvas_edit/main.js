import { CirclePath, WavePath, SpiralPath } from './path.js';
import { CanvasPathEditor } from './editor.js';

const canvas = document.getElementById('editor-canvas');
const ctx = canvas.getContext('2d');
const hiddenInput = document.getElementById('hidden-input');
const warningBanner = document.getElementById('api-warning-banner');

// UI Controls
const shapeBtns = document.querySelectorAll('.shape-btn:not(.mode-btn)');
const modeBtns = document.querySelectorAll('.mode-btn');
const fontFamilySelect = document.getElementById('font-family-select');
const fontSizeSlider = document.getElementById('font-size-slider');
const fontSizeVal = document.getElementById('font-size-val');
const radiusSlider = document.getElementById('radius-slider');
const radiusVal = document.getElementById('radius-val');
const radiusLabelText = document.getElementById('radius-label-text');
const textAlignSelect = document.getElementById('text-align-select');
const textBaselineSelect = document.getElementById('text-baseline-select');

// Initial setup
const initialText = "✨ Hello عالم 👨‍👩‍👧‍👦 ffi AVATAR! ✨";
let currentShape = 'circle';
let currentRadius = parseFloat(radiusSlider.value);

function createPath(shape, radius) {
  const cx = canvas.width / 2;
  const cy = canvas.height / 2 + 10;
  if (shape === 'wave') {
    return new WavePath(cx, cy, radius * 0.55, 600, 1200);
  } else if (shape === 'spiral') {
    return new SpiralPath(cx, cy - 20, radius * 0.25, radius * 0.08);
  }
  return new CirclePath(cx, cy, radius, -Math.PI / 2);
}

const initialPath = createPath(currentShape, currentRadius);
const editor = new CanvasPathEditor(initialText, initialPath);
editor.fontSize = parseInt(fontSizeSlider.value, 10);

// Feature detection check
if (!editor.checkExperimentalApis(ctx)) {
  if (warningBanner) warningBanner.style.display = 'block';
  modeBtns.forEach((btn) => {
    if (btn.dataset.mode === 'auto') {
      btn.classList.remove('active');
      btn.disabled = true;
      btn.style.opacity = '0.4';
      btn.style.cursor = 'not-allowed';
      btn.title = 'Experimental Canvas 2D TextMetrics APIs are not detected in this browser.';
    } else if (btn.dataset.mode === 'fallback') {
      btn.classList.add('active');
    }
  });
  editor.forceFallback = true;
}

// Synchronize hidden textarea with editor state
function syncTextareaToEditor() {
  if (hiddenInput.value !== editor.text) {
    hiddenInput.value = editor.text;
  }
  const minSel = Math.min(editor.selectionStart, editor.selectionEnd);
  const maxSel = Math.max(editor.selectionStart, editor.selectionEnd);
  try {
    hiddenInput.setSelectionRange(editor.selectionEnd, editor.selectionEnd);
    if (editor.hasSelection()) {
      hiddenInput.setSelectionRange(minSel, maxSel);
    }
  } catch (e) {
    // Ignore if textarea not focusable
  }
}

function syncEditorToTextarea() {
  const candidate = hiddenInput.value;
  if (candidate !== editor.text && candidate.length > editor.text.length && !editor.canFitText(ctx, candidate)) {
    // Reject update if text overflows path length limit
    syncTextareaToEditor();
    return;
  }
  editor.text = candidate;
  editor.selectionStart = hiddenInput.selectionStart;
  editor.selectionEnd = hiddenInput.selectionEnd;
  editor.cursorIndex = hiddenInput.selectionDirection === 'backward'
    ? hiddenInput.selectionStart
    : hiddenInput.selectionEnd;
  editor.lastInputTime = Date.now();
}

syncTextareaToEditor();

// Pointer & Mouse Interaction
let isPointerDown = false;

function getCanvasPoint(e) {
  const rect = canvas.getBoundingClientRect();
  const scaleX = canvas.width / rect.width;
  const scaleY = canvas.height / rect.height;
  return {
    x: (e.clientX - rect.left) * scaleX,
    y: (e.clientY - rect.top) * scaleY
  };
}

canvas.addEventListener('pointerdown', (e) => {
  isPointerDown = true;
  canvas.setPointerCapture(e.pointerId);
  hiddenInput.focus({ preventScroll: true });

  const pt = getCanvasPoint(e);
  editor.setCursorAtPoint(ctx, pt.x, pt.y, e.shiftKey);
  syncTextareaToEditor();
});

canvas.addEventListener('pointermove', (e) => {
  if (!isPointerDown) return;
  const pt = getCanvasPoint(e);
  editor.setCursorAtPoint(ctx, pt.x, pt.y, true);
  syncTextareaToEditor();
});

canvas.addEventListener('pointerup', (e) => {
  if (!isPointerDown) return;
  isPointerDown = false;
  canvas.releasePointerCapture(e.pointerId);
  syncTextareaToEditor();
});

// Keyboard & Input Synchronization
hiddenInput.addEventListener('input', () => {
  syncEditorToTextarea();
});

hiddenInput.addEventListener('keydown', (e) => {
  if (e.key === 'ArrowUp' || e.key === 'ArrowDown') {
    e.preventDefault();
    const step = e.key === 'ArrowUp' ? -6 : 6;
    editor.moveCursor(step, e.shiftKey);
    syncTextareaToEditor();
    return;
  }
  if (e.key === 'Tab') {
    e.preventDefault();
    editor.insertText(ctx, '  ');
    syncTextareaToEditor();
    return;
  }
  // Allow default ArrowLeft/Right/Backspace/Delete to update textarea, then sync
  requestAnimationFrame(() => {
    syncEditorToTextarea();
  });
});

hiddenInput.addEventListener('keyup', () => {
  syncEditorToTextarea();
});

document.addEventListener('selectionchange', () => {
  if (document.activeElement === hiddenInput) {
    syncEditorToTextarea();
  }
});

// Focus handling
canvas.addEventListener('click', () => {
  hiddenInput.focus({ preventScroll: true });
  editor.isFocused = true;
});

hiddenInput.addEventListener('focus', () => {
  editor.isFocused = true;
});

hiddenInput.addEventListener('blur', () => {
  editor.isFocused = false;
});

// UI Control Listeners
shapeBtns.forEach((btn) => {
  btn.addEventListener('click', () => {
    shapeBtns.forEach((b) => b.classList.remove('active'));
    btn.classList.add('active');
    currentShape = btn.dataset.shape;
    if (radiusLabelText) {
      if (currentShape === 'wave') radiusLabelText.textContent = 'Amplitude';
      else if (currentShape === 'spiral') radiusLabelText.textContent = 'Scale';
      else radiusLabelText.textContent = 'Radius';
    }
    editor.setPath(createPath(currentShape, currentRadius));
    hiddenInput.focus();
  });
});

modeBtns.forEach((btn) => {
  btn.addEventListener('click', () => {
    if (btn.disabled) return;
    modeBtns.forEach((b) => b.classList.remove('active'));
    btn.classList.add('active');
    editor.forceFallback = btn.dataset.mode === 'fallback';
    const hasApi = editor.checkExperimentalApis(ctx);
    if (warningBanner) {
      warningBanner.style.display = (!hasApi && !editor.forceFallback) ? 'block' : 'none';
    }
    hiddenInput.focus();
  });
});

fontFamilySelect.addEventListener('change', (e) => {
  editor.fontFamily = e.target.value;
  hiddenInput.focus();
});

fontSizeSlider.addEventListener('input', (e) => {
  editor.fontSize = parseInt(e.target.value, 10);
  fontSizeVal.textContent = `${editor.fontSize}px`;
});

radiusSlider.addEventListener('input', (e) => {
  currentRadius = parseFloat(e.target.value);
  radiusVal.textContent = `${Math.round(currentRadius)}px`;
  editor.setPath(createPath(currentShape, currentRadius));
});

textAlignSelect.addEventListener('change', (e) => {
  editor.textAlign = e.target.value;
  hiddenInput.focus();
});

textBaselineSelect.addEventListener('change', (e) => {
  editor.textBaseline = e.target.value;
  hiddenInput.focus();
});

// Animation Loop
function render() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  editor.draw(ctx);
  requestAnimationFrame(render);
}

// Grab focus by default on startup
syncTextareaToEditor();
hiddenInput.focus({ preventScroll: true });
editor.isFocused = true;

requestAnimationFrame(render);
