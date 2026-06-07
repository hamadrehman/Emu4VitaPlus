#pragma once

#include <stdint.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SwrContext SwrContext;

int swr_alloc_set_opts2(struct SwrContext **ps,
                        const AVChannelLayout *out_ch_layout,
                        enum AVSampleFormat out_sample_fmt,
                        int out_sample_rate,
                        const AVChannelLayout *in_ch_layout,
                        enum AVSampleFormat in_sample_fmt,
                        int in_sample_rate,
                        int log_offset,
                        void *log_ctx);

int swr_init(struct SwrContext *s);
void swr_free(struct SwrContext **s);
int swr_convert(struct SwrContext *s,
                uint8_t **out,
                int out_count,
                const uint8_t **in,
                int in_count);
int swr_get_out_samples(struct SwrContext *s, int in_samples);

#ifdef __cplusplus
}
#endif
