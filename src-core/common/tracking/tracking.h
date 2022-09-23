#pragma once

#include "tle.h"
#include "libs/predict/predict.h"
#include "common/geodetic/geodetic_coordinates.h"
#include "core/pipeline.h"

namespace satdump
{
    struct Transit
    {
        double aos;
        double los;
        double peak_elevation;

        Transit(double aos, double los, double peak_elevation);
    };

    class SatelliteTracker
    {
    private:
        predict_orbital_elements_t *satellite_object;
        predict_position satellite_orbit;
        predict_observer_t *obs;
        TLE tle;
        std::shared_ptr<Transit> next_transit;
        std::pair<Pipeline, double> pipeline; // pipeline, frequency_mhz
        // TEMPORARY
        std::tuple<double, double, double> location = std::tuple<double, double, double>(0, 0, 0); // lat, lon, alt

    public:
        SatelliteTracker(TLE tle);
        ~SatelliteTracker();

        geodetic::geodetic_coords_t get_sat_position_at(double utc_time);
        void calculate_next_transit(double min_elevation);
        std::shared_ptr<Transit> get_next_transit();
        TLE get_tle();
        void set_pipeline(std::pair<Pipeline, double> pipeline);
        std::pair<Pipeline, double> get_pipeline();
    };
}