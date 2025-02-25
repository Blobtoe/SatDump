#pragma once

#include "block.h"
#include <fftw3.h>

namespace dsp
{
    class FFTPanBlock : public Block<complex_t, float>
    {
    private:
        std::mutex fft_mutex;
        float *fft_taps = nullptr;
        int fft_size;
        void work();

        void destroy_fft();

        float *fft_output_buffer;

        int in_main_buffer = 0;
        complex_t *fft_main_buffer;

        fftwf_complex *fftw_in;
        fftwf_complex *fftw_out;
        fftwf_plan fftw_plan;

    public:
        FFTPanBlock(std::shared_ptr<dsp::stream<complex_t>> input);
        ~FFTPanBlock();
        void set_fft_settings(int size);

        float avg_rate = 0.01;
    };
}