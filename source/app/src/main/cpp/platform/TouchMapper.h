// TouchMapper.h — touch-first RTS gesture translation (Master Prompt §6).
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
//
// NOT a mouse emulation layer. Maps phone gestures to engine-level intents
// (select, command, camera) as defined in docs/CONTROLS.md. The engine seam
// consumes intents; thresholds are calibrated from Settings (§8).
#pragma once

#include <cstdint>

namespace generals {

enum class Gesture : uint8_t {
    None = 0,
    TapSelect,        // tap on unit/building
    TapCommand,       // tap on ground with selection -> contextual command
    DragSelect,       // box selection
    LongPress,        // advanced command menu / hold-to-preview
    TwoFingerPan,     // camera movement
    PinchZoom,
    DoubleTapSimilar, // select all similar on screen
    EdgeScroll
};

struct TouchPoint { float x; float y; };

struct GestureConfig {
    float dragThresholdPx   = 18.0f;   // scaled by touch sensitivity (Settings)
    float pinchThresholdPx  = 24.0f;
    long  longPressMs       = 450;
    long  doubleTapMs       = 280;
    float pinchZoomFactor   = 2.5f;    // max zoom-in factor
};

/// Stateful recognizer. Feed MotionEvents (converted to TouchPoint + action)
/// from the game surface; receives Gesture intents with payload coordinates.
class TouchMapper {
public:
    explicit TouchMapper(const GestureConfig& config);

    /// action: 0=down 1=pointer-down 2=move 3=up 4=cancel (Android constants mapped)
    Gesture feed(int action, int pointerCount,
                 float x0, float y0, float x1, float y1,
                 float* outX, float* outY, float* outAmount);

    void applySensitivity(float touchSens /*0..100*/);

private:
    GestureConfig m_cfg;
    bool m_down = false;
    bool m_multi = false;
    float m_startX = 0, m_startY = 0;
    float m_lastX = 0, m_lastY = 0;
    long  m_downMs = 0;
    long  m_lastTapMs = 0;
};

} // namespace generals
