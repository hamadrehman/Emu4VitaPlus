#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum AVSampleFormat
{
    AV_SAMPLE_FMT_NONE = -1,
    AV_SAMPLE_FMT_U8 = 0,
    AV_SAMPLE_FMT_S16 = 1,
    AV_SAMPLE_FMT_S32 = 2,
    AV_SAMPLE_FMT_FLT = 3,
    AV_SAMPLE_FMT_DBL = 4,
    AV_SAMPLE_FMT_U8P = 5,
    AV_SAMPLE_FMT_S16P = 6,
    AV_SAMPLE_FMT_S32P = 7,
    AV_SAMPLE_FMT_FLTP = 8,
    AV_SAMPLE_FMT_DBLP = 9
} AVSampleFormat;

#ifdef __cplusplus
}
#endif
