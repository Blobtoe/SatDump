#pragma once

#include "../app.h"
#include "common/dsp_source_sink/dsp_sample_source.h"
#include "pipeline_selector.h"
#include "common/tracking/scheduler.h"

namespace satdump
{
    class AutomaticApplication : public Application
    {
        protected:
            void drawUI();

            bool is_started = false;
            int sdr_select_id = 0;
            std::string sdr_select_string;

            std::shared_ptr<Scheduler> scheduler;

            std::vector<dsp::SourceDescriptor> sources;
            std::shared_ptr<dsp::DSPSampleSource> source_ptr;

            std::vector<std::string> satellite_names;
            int satellite_name_id = 0;

            PipelineUISelector pipeline_selector;
            int pipeline_preset_id = 0;

            std::string sdr_error, error;

        public:
            AutomaticApplication();
            ~AutomaticApplication();

        public:
            static std::string getID() { return "automatic"; }
            static std::shared_ptr<Application> getInstance() { return std::make_shared<AutomaticApplication>(); }
    };
};