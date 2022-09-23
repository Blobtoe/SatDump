#include "automatic.h"
#include "main_ui.h"
#include "core/style.h"
#include "logger.h"

namespace satdump
{
    AutomaticApplication::AutomaticApplication() : Application("automatic"), pipeline_selector(true)
    {
        // get all available sources
        dsp::registerAllSources();
        sources = dsp::getAllAvailableSources();
        for (dsp::SourceDescriptor src : sources)
        {
            logger->debug("Device " + src.name);
            sdr_select_string += src.name + '\0';
        }
        for (int i = 0; i < (int)sources.size(); i++)
        {
            try
            {
                source_ptr = dsp::getSourceFromDescriptor(sources[i]);
                source_ptr->open();
                sdr_select_id = i;
                break;
            }
            catch (std::runtime_error &e)
            {
                logger->error(e.what());
            }
        }

        // create list of available satellites
        std::vector<int> norads_to_fetch = config::main_cfg["tle_settings"]["tles_to_fetch"].get<std::vector<int>>();
        for (std::size_t i = 0; i < norads_to_fetch.size(); i++)
        {
            std::optional<TLE> tle = satdump::general_tle_registry.get_from_norad(norads_to_fetch[i]);
            if (tle.has_value())
                satellite_names.push_back(tle.value().name);
        }

        scheduler = std::make_shared<Scheduler>();
    }

    AutomaticApplication::~AutomaticApplication()
    {
        source_ptr->stop();
        source_ptr->close();
        scheduler->stop();
    }

    void AutomaticApplication::drawUI()
    {
        ImGui::BeginGroup();

        if (ImGui::CollapsingHeader("Device", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();
            if (is_started)
                style::beginDisabled();
            if (ImGui::Combo("##Source", &sdr_select_id, sdr_select_string.c_str()))
            {
                source_ptr = getSourceFromDescriptor(sources[sdr_select_id]);

                // Try to open a device, if it doesn't work, we re-open a device we can
                try
                {
                    source_ptr->open();
                }
                catch (std::runtime_error &e)
                {
                    logger->error(e.what());

                    for (int i = 0; i < (int)sources.size(); i++)
                    {
                        try
                        {
                            source_ptr = dsp::getSourceFromDescriptor(sources[i]);
                            source_ptr->open();
                            sdr_select_id = i;
                            break;
                        }
                        catch (std::runtime_error &e)
                        {
                            logger->error(e.what());
                        }
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(u8" \uead2 "))
            {
                sources = dsp::getAllAvailableSources();

                sdr_select_string.clear();
                for (dsp::SourceDescriptor src : sources)
                {
                    logger->debug("Device " + src.name);
                    sdr_select_string += src.name + '\0';
                }

                while (sdr_select_id >= (int)sources.size())
                    sdr_select_id--;

                source_ptr = getSourceFromDescriptor(sources[sdr_select_id]);
                source_ptr->open();
                source_ptr->set_frequency(100e6);
            }
            if (is_started)
                style::endDisabled();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            source_ptr->drawControlUI();

            if (!is_started)
            {
                if (ImGui::Button("Start"))
                {
                    scheduler->start();
                    is_started = true;
                }
            }
            else
            {
                if (ImGui::Button("Stop"))
                {
                    scheduler->stop();
                    is_started = false;
                }
            }
            ImGui::SameLine();
            ImGui::TextColored(ImColor(255, 0, 0), "%s", scheduler->error.c_str());
        }
        if (ImGui::CollapsingHeader("Satellites", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::BeginTable("##satellitelist", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                for (std::shared_ptr<SatelliteTracker> satellite_tracker : scheduler->get_satellite_trackers())
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", satellite_tracker->get_tle().name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", satellite_tracker->get_pipeline().first.readable_name.c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", std::to_string(satellite_tracker->get_pipeline().second).c_str());
                    ImGui::TableSetColumnIndex(3);
                    if (ImGui::Button("Remove"))
                        scheduler->remove_satellite_tracker(satellite_tracker);
                }
            }
            ImGui::EndTable();

            ImGui::Separator();
            
            if (satellite_names.size() > 0)
            {
                if (ImGui::BeginCombo("Satellite###satellitescombo", satellite_names[satellite_name_id].c_str()))
                {
                    for (int n = 0; n < (int)satellite_names.size(); n++)
                    {
                        const bool is_selected = (satellite_name_id == n);
                        if (ImGui::Selectable(satellite_names[n].c_str(), is_selected))
                            satellite_name_id = n;

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            pipeline_selector.renderSelectionBox(ImGui::GetContentRegionAvail().x);
            pipeline_selector.renderParamTable();

            Pipeline selected_pipeline = pipelines[pipeline_selector.pipeline_id];
            if (pipeline_preset_id >= (int)selected_pipeline.preset.frequencies.size())
                pipeline_preset_id = 0;
            if (selected_pipeline.preset.frequencies.size() > 0)
            {
                if (ImGui::BeginCombo("Freq###presetscombo", selected_pipeline.preset.frequencies[pipeline_preset_id].first.c_str()))
                {
                    for (int n = 0; n < (int)selected_pipeline.preset.frequencies.size(); n++)
                    {
                        const bool is_selected = (pipeline_preset_id == n);
                        if (ImGui::Selectable(selected_pipeline.preset.frequencies[n].first.c_str(), is_selected))
                            pipeline_preset_id = n;

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            if (ImGui::Button("Add"))
            {
                std::optional<TLE> tle = satdump::general_tle_registry.get_from_name(satellite_names[satellite_name_id]);
                if (selected_pipeline.preset.frequencies[pipeline_preset_id].second == 0)
                {
                    error = "Invalid frequency";
                }
                else if (!tle.has_value())
                {
                    error = "Could not find TLE";
                }
                else
                {
                    error = "";
                    std::shared_ptr<SatelliteTracker> satellite_tracker = std::make_shared<SatelliteTracker>(tle.value());
                    satellite_tracker->set_pipeline(std::pair<Pipeline,double>(selected_pipeline,double(selected_pipeline.preset.frequencies[pipeline_preset_id].second) / 1e6));
                    scheduler->add_satellite_tracker(satellite_tracker);
                }
            }
        }
        ImGui::SameLine();
        ImGui::TextColored(ImColor(255, 0, 0), "%s", error.c_str());
        ImGui::EndGroup();
    };
};