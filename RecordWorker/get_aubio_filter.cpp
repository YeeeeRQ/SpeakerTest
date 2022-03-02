#include "get_aubio_filter.h"

aubio_filter_t* get_aubio_filter(filter_type f_type){
    lsmp_t a0 = .0;
    lsmp_t a1 = .0;
    lsmp_t a2 = .0;
    lsmp_t b1 = .0;
    lsmp_t b2 = .0;

    if(f_type == filter_type::F_HIGHPASS_400Hz){
        // 400Hz highpass Q=0.8
        a0 = 0.9648402352214867;
        a1 = -1.9296804704429733;
        a2 = 0.9648402352214867;
        b1 = -1.9281127699822234;
        b2 = 0.9312481709037234;
    }else  if(f_type == filter_type::F_LOWPASS_8000Hz){
        // 8000Hz lowpass Q=0.8
        a0 = 0.1856783730608666;
        a1 = 0.3713567461217332;
        a2 = 0.1856783730608666;
        b1 = -0.5329223919126902;
        b2 = 0.27563588415615664;
    }else  if(f_type == filter_type::F_PEAK_400Hz){
        // 400Hz peak q8 gain_10dB
        a0 = 1.0076703344943496;
        a1 = -1.9896698176497059;
        a2 = 0.9852349850807944;
        b1 = -1.9896698176497059;
        b2 = 0.992905319575144;
    }else  if(f_type == filter_type::F_PEAK_1000Hz){
        // 1000Hz peak q8 gain_3dB
        a0 = 1.0036289188390717;
        a1 = -1.9623200519727308;
        a2 = 0.9787779244343572;
        b1 = -1.9623200519727308;
        b2 = 0.9824068432734288;
    }else  if(f_type == filter_type::F_PEAK_2000Hz){
        // 2000Hz peak q8 gain_3dB
        a0 = 1.0071229099569896;
        a1 = -1.8862107789184968;
        a2 = 0.9583449120638954;
        b1 = -1.8862107789184968;
        b2 = 0.9654678220208853;
    }else  if(f_type == filter_type::F_PEAK_3000Hz){
        // 3000Hz peak q8 gain_3dB
        a0 = 1.010418188126155;
        a1 = -1.774106323463592;
        a2 = 0.9390739816801984;
        b1 = -1.774106323463592;
        b2 = 0.9494921698063533;
    }else  if(f_type == filter_type::F_PEAK_4000Hz){
        // 4000Hz peak q8 gain_3dB
        a0 = 1.013457737894429;
        a1 = -1.6289740334471616;
        a2 = 0.9212985621328299;
        b1 = -1.6289740334471616;
        b2 = 0.9347563000272592;
    }else  if(f_type == filter_type::F_PEAK_5000Hz){
        // 5000Hz peak q8 gain_3dB
        a0 = 1.0161916294601814;
        a1 = -1.454209891309759;
        a2 = 0.9053106450783083;
        b1 = -1.454209891309759;
        b2 = 0.9215022745384897;
    }else{
        // do nothing
    }

    aubio_filter_t*  f= new_aubio_filter(3);
    aubio_filter_set_biquad(f, a0, a1, a2, b1, b2);
    return  f;
}


