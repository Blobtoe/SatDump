#include "scheduler.h"
#include "logger.h"
#include <chrono>

namespace satdump
{
    Scheduler::~Scheduler()
    {
        stop();
    }

    void Scheduler::work()
    {
        // TODO: handle if there is no next transit
        int now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (now >= next_satellite_tracker->get_next_transit()->aos)
        {
            logger->info("Processing transit");
            // TODO: process transit
            stop();
        }
    }

    void Scheduler::start()
    {
        calculate_next_satellite_tracker();
        should_run = true;
        d_thread = std::thread(&Scheduler::run, this);
    }
    void Scheduler::stop()
    {
        should_run = false;
        if (d_thread.joinable())
            d_thread.join();
    }
    void Scheduler::add_satellite_tracker(std::shared_ptr<SatelliteTracker> satellite_tracker)
    {
        satellite_trackers.push_back(satellite_tracker);
    }

    void Scheduler::remove_satellite_tracker(std::shared_ptr<SatelliteTracker> satellite_tracker)
    {
        for (std::size_t i = 0; i < satellite_trackers.size(); i++)
        {
            if (satellite_trackers[i]->get_tle().norad == satellite_tracker->get_tle().norad)
            {
                satellite_trackers.erase(satellite_trackers.begin() + i);
                break;
            }
        }
    }

    std::vector<std::shared_ptr<SatelliteTracker>> Scheduler::get_satellite_trackers()
    {
        return satellite_trackers;
    }

    void Scheduler::calculate_next_satellite_tracker()
    {
        // loop over every tracker and find the one with the smallest aos
        for (std::shared_ptr<SatelliteTracker> satellite_tracker : satellite_trackers)
        {
            // TODO: move min_elevation paramater to settings. Using 16 as placeholder
            satellite_tracker->calculate_next_transit(16);
            if (!has_next_transit || satellite_tracker->get_next_transit()->aos < next_satellite_tracker->get_next_transit()->aos)
            {
                has_next_transit = true;
                next_satellite_tracker = satellite_tracker;
            }
        }
        if (!has_next_transit)
            error = "No transits found!";
        else
        {
            error = "";
            logger->info("found transit for " + next_satellite_tracker->get_tle().name + " from " + std::to_string(next_satellite_tracker->get_next_transit()->aos) + " to " + std::to_string(next_satellite_tracker->get_next_transit()->los) + " at " + std::to_string(next_satellite_tracker->get_next_transit()->peak_elevation) + "deg");
        }
    }

    std::shared_ptr<SatelliteTracker> Scheduler::get_next_satellite_tracker()
    {
        return next_satellite_tracker;
    }
}