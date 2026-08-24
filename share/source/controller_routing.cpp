#include <string.h>
#include "controller_routing.h"
#include "log.h"

namespace Emu4VitaPlus
{
    ControllerRouting::ControllerRouting() : _last_supported(-1),
                                             _has_last_info(false),
                                             _refresh_counter(0)
    {
        Reset();
    }

    void ControllerRouting::Reset()
    {
        for (uint32_t i = 0; i < INPUT_MAX_CTRL_PORTS; ++i)
        {
            _player_port_map[i] = i;
        }
        memset(&_last_info, 0, sizeof(_last_info));
        _last_supported = -1;
        _has_last_info = false;
        _refresh_counter = 0;
    }

    void ControllerRouting::_LogControllerPorts(const SceCtrlPortInfo &info, int supported)
    {
        LogInfo("multi-controller support: %d", supported);
        LogInfo("controller ports: [0]=%u [1]=%u [2]=%u [3]=%u [4]=%u",
                info.port[0], info.port[1], info.port[2], info.port[3], info.port[4]);
    }

    void ControllerRouting::_RebuildPlayerPortMap(const SceCtrlPortInfo &info)
    {
        if (info.port[0] != SCE_CTRL_TYPE_VIRT)
        {
            for (uint32_t i = 0; i < INPUT_MAX_CTRL_PORTS; ++i)
            {
                _player_port_map[i] = i;
            }
            return;
        }

        memset(_player_port_map, INPUT_INVALID_CTRL_PORT, sizeof(_player_port_map));

        uint32_t player = 0;
        for (uint32_t ctrl_port = 1; ctrl_port < INPUT_MAX_CTRL_PORTS && player < INPUT_MAX_CTRL_PORTS; ++ctrl_port)
        {
            if (info.port[ctrl_port] != SCE_CTRL_TYPE_UNPAIRED)
            {
                _player_port_map[player++] = ctrl_port;
            }
        }

        if (player > 0)
        {
            LogInfo("player-port map: P1->%u P2->%u P3->%u P4->%u P5->%u",
                    _player_port_map[0], _player_port_map[1], _player_port_map[2],
                    _player_port_map[3], _player_port_map[4]);
        }
    }

    void ControllerRouting::Refresh(bool force_log)
    {
        SceCtrlPortInfo info{};
        int supported = sceCtrlIsMultiControllerSupported();
        int ret = sceCtrlGetControllerPortInfo(&info);
        if (ret < 0)
        {
            if (force_log || !_has_last_info)
            {
                LogWarn("sceCtrlGetControllerPortInfo failed: 0x%08X multi:%d", ret, supported);
            }
            return;
        }

        bool changed = !_has_last_info ||
                       supported != _last_supported ||
                       memcmp(&info, &_last_info, sizeof(info)) != 0;
        if (force_log || changed)
        {
            _LogControllerPorts(info, supported);
        }

        _RebuildPlayerPortMap(info);
        _last_info = info;
        _last_supported = supported;
        _has_last_info = true;
    }

    void ControllerRouting::RefreshPeriodic()
    {
        if ((++_refresh_counter % 240) == 0)
        {
            Refresh();
        }
    }

    uint32_t ControllerRouting::GetCtrlPortForPlayer(uint32_t player_port) const
    {
        if (player_port >= INPUT_MAX_CTRL_PORTS)
        {
            return INPUT_INVALID_CTRL_PORT;
        }
        uint32_t mapped = _player_port_map[player_port];
        return mapped < INPUT_MAX_CTRL_PORTS ? mapped : INPUT_INVALID_CTRL_PORT;
    }

    uint32_t ControllerRouting::GetPrimaryUiPort() const
    {
        uint32_t primary = GetCtrlPortForPlayer(0);
        if (!_has_last_info)
        {
            return primary;
        }

        if (primary < INPUT_MAX_CTRL_PORTS &&
            _last_info.port[primary] != SCE_CTRL_TYPE_UNPAIRED &&
            _last_info.port[primary] != SCE_CTRL_TYPE_VIRT)
        {
            return primary;
        }

        for (uint32_t player = 0; player < INPUT_MAX_CTRL_PORTS; ++player)
        {
            uint32_t mapped = GetCtrlPortForPlayer(player);
            if (mapped < INPUT_MAX_CTRL_PORTS &&
                _last_info.port[mapped] != SCE_CTRL_TYPE_UNPAIRED &&
                _last_info.port[mapped] != SCE_CTRL_TYPE_VIRT)
            {
                return mapped;
            }
        }

        if (_last_info.port[0] != SCE_CTRL_TYPE_UNPAIRED)
        {
            return 0;
        }

        return 0;
    }

    bool ControllerRouting::IsVirtualPort(uint32_t ctrl_port) const
    {
        return _has_last_info &&
               ctrl_port < INPUT_MAX_CTRL_PORTS &&
               _last_info.port[ctrl_port] == SCE_CTRL_TYPE_VIRT;
    }

    bool ControllerRouting::IsUnpairedPort(uint32_t ctrl_port) const
    {
        return _has_last_info &&
               ctrl_port < INPUT_MAX_CTRL_PORTS &&
               _last_info.port[ctrl_port] == SCE_CTRL_TYPE_UNPAIRED;
    }
}
