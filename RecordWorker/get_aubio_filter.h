#ifndef GET_AUBIO_FILTER_H
#define GET_AUBIO_FILTER_H

#include <aubio/aubio.h>


enum class filter_type{
    F_HIGHPASS_400Hz,
    F_LOWPASS_8000Hz,

    F_PEAK_400Hz,
    F_PEAK_1000Hz,
    F_PEAK_2000Hz,
    F_PEAK_3000Hz,
    F_PEAK_4000Hz,
    F_PEAK_5000Hz,
    None,
};

aubio_filter_t* get_aubio_filter(filter_type f_type);

#endif
