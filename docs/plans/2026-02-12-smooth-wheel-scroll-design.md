# Smooth Wheel Scroll Design

## Problem
Mouse wheel scrolling in the `.root` container feels jerky — "뚝뚝 끊김" across all tabs.

### Root Causes
1. **Lerp 0.25 "jump-and-crawl"**: First frame moves 25% of remaining distance (big jump), then exponentially decelerates (crawl).
2. **Excessive delta snap**: Tiny Ultralight deltas (<12px) snap to 180*uiScale, causing 360px jumps at 2x scale.
3. **No velocity/momentum**: Each wheel event is independent; continuous scrolling has no acceleration feel.

## Solution: Hybrid Velocity + Tuned Deltas

### 1. Delta Normalization (tuning)
| Condition | Before | After |
|-----------|--------|-------|
| tiny delta (<12, gap >24ms) | 180 * uiScale | 80 * uiScale |
| small delta (<40) | delta * 3.0 * uiScale | delta * 2.0 * uiScale |
| normal delta | delta * uiScale | delta * uiScale |
| max step | 0.9 * viewport | 0.6 * viewport |

### 2. Animation Loop (velocity + friction)
Replace lerp model with:
- **velocity accumulation**: `velocity += deltaPx * 0.15`
- **per-frame friction**: `velocity *= 0.88` (~300ms to stop at 60fps)
- **stop threshold**: `abs(velocity) < 0.5` → snap to 0, stop animation
- No more `scrollTarget`; velocity drives `scrollTop` directly.

### 3. Edge Cases
- Tab switch → reset velocity to 0
- Keyboard nav (`scrollQuickIndexIntoView`) → unchanged (instant scroll)
- Virtual table sync → existing `scroll` event listener handles it

### Files Changed
- `PrismaUI/views/codexofpowerng/index.html` — `installSmoothWheelScroll()` only
