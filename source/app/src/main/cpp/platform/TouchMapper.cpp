// TouchMapper.cpp — gesture recognizer (docs/CONTROLS.md is the spec).
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
#include "TouchMapper.h"

#include <chrono>
#include <cmath>

namespace generals {

namespace {
long nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
} // namespace

TouchMapper::TouchMapper(const GestureConfig& config) : m_cfg(config) {}

void TouchMapper::applySensitivity(float touchSens /*0..100*/) {
    // Higher sensitivity -> lower distance thresholds. Documented calibration
    // hook (§35): thresholds re-derivable, no magic permanence.
    const float k = 0.5f + (100.0f - touchSens) / 100.0f; // 0.5x..1.5x
    m_cfg.dragThresholdPx = 18.0f * k;
    m_cfg.pinchThresholdPx = 24.0f * k;
}

Gesture TouchMapper::feed(int action, int pointerCount,
                          float x0, float y0, float x1, float y1,
                          float* outX, float* outY, float* outAmount) {
    if (outX) *outX = 0; if (outY) *outY = 0; if (outAmount) *outAmount = 0;

    switch (action) {
        case 0: { // ACTION_DOWN
            m_down = true; m_multi = pointerCount > 1;
            m_startX = m_lastX = x0; m_startY = m_lastY = y0;
            m_downMs = nowMs();
            return Gesture::None;
        }
        case 1: { // ACTION_POINTER_DOWN
            m_multi = true;
            return Gesture::None;
        }
        case 2: { // ACTION_MOVE
            if (!m_down) return Gesture::None;
            const float dx = x0 - m_lastX, dy = y0 - m_lastY;
            m_lastX = x0; m_lastY = y0;
            if (m_multi) {
                if (outX) *outX = dx; if (outY) *outY = dy;
                return Gesture::TwoFingerPan;
            }
            const float dist = std::hypot(x0 - m_startX, y0 - m_startY);
            if (dist > m_cfg.dragThresholdPx) {
                if (outX) *outX = m_startX; if (outY) *outY = m_startY;
                return Gesture::DragSelect;
            }
            return Gesture::None;
        }
        case 3: { // ACTION_UP
            if (!m_down) return Gesture::None;
            m_down = false;
            const long held = nowMs() - m_downMs;
            const float dist = std::hypot(x0 - m_startX, y0 - m_startY);
            if (m_multi) { m_multi = false; return Gesture::None; }
            if (held >= m_cfg.longPressMs && dist <= m_cfg.dragThresholdPx) {
                if (outX) *outX = x0; if (outY) *outY = y0;
                return Gesture::LongPress;
            }
            if (dist <= m_cfg.dragThresholdPx) {
                const long sinceLast = nowMs() - m_lastTapMs;
                m_lastTapMs = nowMs();
                if (sinceLast <= m_cfg.doubleTapMs) {
                    if (outX) *outX = x0; if (outY) *outY = y0;
                    return Gesture::DoubleTapSimilar;
                }
                if (outX) *outX = x0; if (outY) *outY = y0;
                // Semantics (select vs command) resolved by the engine seam
                // using current selection state — intent only here (§6).
                return Gesture::TapSelect;
            }
            return Gesture::None;
        }
        case 4: { // ACTION_CANCEL
            m_down = false; m_multi = false;
            return Gesture::None;
        }
        default: return Gesture::None;
    }
}

} // namespace generals
