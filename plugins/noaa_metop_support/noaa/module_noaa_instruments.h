#pragma once

#include "core/module.h"
#include "common/repack.h"
#include "instruments/avhrr/avhrr_reader.h"
#include "../instruments/mhs/mhs_reader.h"
#include "instruments/hirs/hirs_reader.h"
#include "instruments/amsu/amsu_reader.h"

namespace noaa
{
    namespace instruments
    {
        class NOAAInstrumentsDecoderModule : public ProcessingModule
        {
        protected:
            std::atomic<size_t> filesize;
            std::atomic<size_t> progress;
            const bool is_gac;
            const bool is_dsb;

            // Readers
            avhrr::AVHRRReader avhrr_reader;
            noaa_metop::mhs::MHSReader mhs_reader;
            hirs::HIRSReader hirs_reader;
            amsu::AMSUReader amsu_reader;

            // Statuses
            instrument_status_t avhrr_status = DECODING;
            instrument_status_t mhs_status = DECODING;
            instrument_status_t amsu_status = DECODING;
            instrument_status_t hirs_status = DECODING;

        public:
            NOAAInstrumentsDecoderModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
            void process();
            void drawUI(bool window);

        public:
            static std::string getID();
            virtual std::string getIDM() { return getID(); };
            static std::vector<std::string> getParameters();
            static std::shared_ptr<ProcessingModule> getInstance(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
        };
    } // namespace amsu
} // namespace noaa