#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum AVChannelOrder
{
    AV_CHANNEL_ORDER_UNSPEC,
    AV_CHANNEL_ORDER_NATIVE,
    AV_CHANNEL_ORDER_CUSTOM,
    AV_CHANNEL_ORDER_AMBISONIC
} AVChannelOrder;

typedef struct AVChannelCustom
{
    uint64_t id;
    char name[16];
    void *opaque;
} AVChannelCustom;

typedef struct AVChannelLayout
{
    enum AVChannelOrder order;
    int nb_channels;
    union
    {
        uint64_t mask;
        AVChannelCustom *map;
    } u;
    void *opaque;
} AVChannelLayout;

#define AV_CH_FRONT_LEFT  (1ULL << 0)
#define AV_CH_FRONT_RIGHT (1ULL << 1)
#define AV_CH_LAYOUT_STEREO (AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT)

void av_channel_layout_default(AVChannelLayout *ch_layout, int nb_channels);

#ifdef __cplusplus
}
#endif
