#pragma once
#include <stdint.h>
#include <functional>
#include <vector>
#include <stack>
#include <psp2/ctrl.h>
#include "rect.h"
#include "touch.h"
#include "controller_routing.h"

const uint32_t SCE_CTRL_LSTICK_UP = 0x00400000;
const uint32_t SCE_CTRL_LSTICK_RIGHT = 0x00800000;
const uint32_t SCE_CTRL_LSTICK_DOWN = 0x01000000;
const uint32_t SCE_CTRL_LSTICK_LEFT = 0x02000000;
const uint32_t SCE_CTRL_RSTICK_UP = 0x04000000;
const uint32_t SCE_CTRL_RSTICK_RIGHT = 0x08000000;
const uint32_t SCE_CTRL_RSTICK_DOWN = 0x10000000;
const uint32_t SCE_CTRL_RSTICK_LEFT = 0x20000000;

// from https://shinmera.github.io/promptfont/
#define BUTTON_LEFT "↞"
#define BUTTON_UP "↟"
#define BUTTON_RIGHT "↠"
#define BUTTON_DOWN "↡"
#define BUTTON_A "⇓"
#define BUTTON_B "⇒"
#define BUTTON_X "⇐"
#define BUTTON_Y "⇑"
#define BUTTON_L "↜"
#define BUTTON_R "↝"
#define BUTTON_L1 "↰"
#define BUTTON_R1 "↱"
#define BUTTON_L2 "↲"
#define BUTTON_R2 "↳"
#define BUTTON_L3 "⇋"
#define BUTTON_R3 "⇌"
#define BUTTON_SELECT "⇷"
#define BUTTON_START "⇸"
#define BUTTON_HOME ""
#define BUTTON_LEFT_ANALOG_LEFT "↼"
#define BUTTON_LEFT_ANALOG_UP "↾"
#define BUTTON_LEFT_ANALOG_RIGHT "⇀"
#define BUTTON_LEFT_ANALOG_DOWN "⇂"
#define BUTTON_RIGHT_ANALOG_LEFT "↽"
#define BUTTON_RIGHT_ANALOG_UP "↿"
#define BUTTON_RIGHT_ANALOG_RIGHT "⇁"
#define BUTTON_RIGHT_ANALOG_DOWN "⇃"
#define BUTTON_CROSS "⇣"
#define BUTTON_CIRCLE "⇢"
#define BUTTON_SQUARE "⇠"
#define BUTTON_TRIANGLE "⇡"
#define BUTTON_RIGHT_ANALOG_LEFT_RIGHT "⇆"
#define BUTTON_LEFT_DOWN "↴"
#define BUTTON_RIGHT_UP "↵"
#define BUTTON_RIGHT_DOWN "⇞"
#define BUTTON_LEFT_UP "⇟"
#define BUTTON_EMPTY_DPAD "⇎"

#define DEFAULT_TURBO_START_TIME 100000ull
#define DEFAULT_TURBO_INTERVAL 50000ull

#define TEST_KEY(KEY, KEYS) (((KEY) & (KEYS)) == (KEY))
#define ANALOG_PSV_TO_RETRO(X) (-32767 + ((int32_t)(X) * 65534) / 255)

extern uint32_t EnterButton;
#define CancelButton (EnterButton == SCE_CTRL_CIRCLE ? SCE_CTRL_CROSS : SCE_CTRL_CIRCLE)

extern void SwapEnterButton(bool swap);

namespace Emu4VitaPlus
{
    class Input;
}

using InputFunc = std::function<void(Emu4VitaPlus::Input *)>;

struct KeyBinding
{
    uint32_t key;
    InputFunc func;
};

struct AnalogAxis
{
    uint8_t x;
    uint8_t y;
};

struct TurboKeyState
{
    bool down;
    uint64_t next_change_state_time;
};

namespace Emu4VitaPlus
{
    class Input
    {
    public:
        Input();
        virtual ~Input();

        void SetKeyUpCallback(uint32_t key, InputFunc func);
        void SetKeyDownCallback(uint32_t key, InputFunc func, bool turbo = false);

        void UnsetKeyUpCallback(uint32_t key);
        void UnsetKeyDownCallback(uint32_t key);

        void SetTurbo(uint32_t key);
        void UnsetTurbo(uint32_t key);

        void SetTurboInterval(uint64_t turbo_start = DEFAULT_TURBO_START_TIME, uint64_t turbo_interval = DEFAULT_TURBO_INTERVAL);

        void Poll(bool waiting = false);

        void Reset();

        uint32_t GetCtrlPortForPlayer(uint32_t port) const;
        uint32_t GetKeyStates(uint32_t port = 0) const
        {
            uint32_t ctrl_port = GetCtrlPortForPlayer(port);
            return ctrl_port < INPUT_MAX_CTRL_PORTS ? _key_states[ctrl_port] : 0;
        };

        AnalogAxis GetLeftAnalogAxis(uint32_t port = 0) const
        {
            uint32_t ctrl_port = GetCtrlPortForPlayer(port);
            return ctrl_port < INPUT_MAX_CTRL_PORTS ? _left_analog[ctrl_port] : AnalogAxis{128, 128};
        };
        AnalogAxis GetRightAnalogAxis(uint32_t port = 0) const
        {
            uint32_t ctrl_port = GetCtrlPortForPlayer(port);
            return ctrl_port < INPUT_MAX_CTRL_PORTS ? _right_analog[ctrl_port] : AnalogAxis{128, 128};
        };

        const int16_t GetMapedLeftAnalogX(uint32_t port = 0) const { return _analog_map_table[GetLeftAnalogAxis(port).x]; };
        const int16_t GetMapedLeftAnalogY(uint32_t port = 0) const { return _analog_map_table[GetLeftAnalogAxis(port).y]; };
        const int16_t GetReverseMapedLeftAnalogY(uint32_t port = 0) const { return _analog_map_table[0xff - GetLeftAnalogAxis(port).y]; };

        const int16_t GetMapedRightAnalogX(uint32_t port = 0) const { return _analog_map_table[GetRightAnalogAxis(port).x]; };
        const int16_t GetMapedRightAnalogY(uint32_t port = 0) const { return _analog_map_table[GetRightAnalogAxis(port).y]; };
        const int16_t GetReverseMapedRightAnalogY(uint32_t port = 0) const { return _analog_map_table[0xff - GetRightAnalogAxis(port).y]; };

        void PushCallbacks();
        void PopCallbacks();

        Touch *GetFrontTouch() { return &_front_touch; };
        Touch *GetRearTouch() { return &_rear_touch; };

    protected:
        std::vector<KeyBinding> _key_up_callbacks;
        std::vector<KeyBinding> _key_down_callbacks;

        TurboKeyState _turbo_key_states[32];
        uint32_t _last_key;
        uint32_t _key_states[INPUT_MAX_CTRL_PORTS];
        uint32_t _current_hotkey;
        uint32_t _turbo_key;
        uint64_t _turbo_start_ms;
        uint64_t _turbo_interval_ms;

        bool _enable_key_up;
        ControllerRouting _controller_routing;
        AnalogAxis _left_analog[INPUT_MAX_CTRL_PORTS];
        AnalogAxis _right_analog[INPUT_MAX_CTRL_PORTS];
        // map to retro's analog
        // -0x7fff to 0x7fff
        static const int16_t _analog_map_table[0x100];

        Touch _front_touch{SCE_TOUCH_PORT_FRONT};
        Touch _rear_touch{SCE_TOUCH_PORT_BACK};

        std::stack<std::vector<KeyBinding>> _callback_stack;

        uint32_t _ProcTurbo(uint32_t key);
        uint32_t _PollPort(uint32_t port, bool waiting);
        virtual void _ProcCallbacks(uint32_t key);
    };
}
