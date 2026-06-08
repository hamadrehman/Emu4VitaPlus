#pragma once

#include <stdint.h>
#include <psp2/ctrl.h>

namespace Emu4VitaPlus
{
    static constexpr uint32_t INPUT_MAX_CTRL_PORTS = 5;

    class ControllerRouting
    {
    public:
        ControllerRouting();

        void Reset();
        void Refresh(bool force_log = false);
        void RefreshPeriodic();
        uint32_t GetCtrlPortForPlayer(uint32_t player_port) const;
        uint32_t GetPrimaryUiPort() const;

    private:
        void _LogControllerPorts(const SceCtrlPortInfo &info, int supported);
        void _RebuildPlayerPortMap(const SceCtrlPortInfo &info);

        uint8_t _player_port_map[INPUT_MAX_CTRL_PORTS];
        SceCtrlPortInfo _last_info;
        int _last_supported;
        bool _has_last_info;
        uint32_t _refresh_counter;
    };
}
