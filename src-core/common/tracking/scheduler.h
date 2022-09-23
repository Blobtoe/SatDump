#pragma once

#include <thread>
#include "tracking.h"

namespace satdump
{
    class Scheduler
    {
    private:
        std::shared_ptr<SatelliteTracker> next_satellite_tracker; //could also store this as the index of the tracker in satellite_trackers
        bool has_next_transit = false;
        std::vector<std::shared_ptr<SatelliteTracker>> satellite_trackers;
        std::thread d_thread;
        bool should_run;

    private:
        void work();
        void run()
        {
            while (should_run)
                work();
        }

    public:
        std::string error;

    public:
        ~Scheduler();
        void start();
        void stop();
        void add_satellite_tracker(std::shared_ptr<SatelliteTracker> satellite_tracker);
        void remove_satellite_tracker(std::shared_ptr<SatelliteTracker> satellite_tracker);
        std::vector<std::shared_ptr<SatelliteTracker>> get_satellite_trackers();
        void calculate_next_satellite_tracker();
        std::shared_ptr<SatelliteTracker> get_next_satellite_tracker();
    };
}