#include "tracking.h"

namespace satdump
{
    SatelliteTracker::SatelliteTracker(TLE tle) : tle(tle)
    {
        satellite_object = predict_parse_tle(tle.line1.c_str(), tle.line2.c_str());
        obs = predict_create_observer("Me", std::get<0>(location)*M_PI/180.0, std::get<1>(location)*M_PI/180.0, std::get<2>(location));
    }
    SatelliteTracker::~SatelliteTracker()
    {
        predict_destroy_orbital_elements(satellite_object);
    }

    geodetic::geodetic_coords_t SatelliteTracker::get_sat_position_at(double utc_time)
    {
        predict_orbit(satellite_object, &satellite_orbit, predict_to_julian_double(utc_time));
        return geodetic::geodetic_coords_t(satellite_orbit.latitude, satellite_orbit.longitude, satellite_orbit.altitude, true).toDegs();
    }

    void SatelliteTracker::calculate_next_transit(double min_elevation)
    {
        int now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        predict_observation observation_aos = predict_next_aos(obs, satellite_object, predict_to_julian_double(now));
        predict_observation observation_los = predict_next_los(obs, satellite_object, observation_aos.time);
        predict_observation observation_tca = predict_at_max_elevation(obs, satellite_object, observation_aos.time);
        int i = 0;
        // loop until we find transit over min_elevation
        while (observation_tca.elevation*180/M_PI < min_elevation)
        {
            if (i > 1000)
            {
                //handle runaway loop
                break;
            }
            observation_aos = predict_next_aos(obs, satellite_object, observation_los.time);
            observation_los = predict_next_los(obs, satellite_object, observation_aos.time);
            observation_tca = predict_at_max_elevation(obs, satellite_object, observation_aos.time);
            i++;
        }
        next_transit = std::make_shared<Transit>(predict_from_julian(observation_aos.time), predict_from_julian(observation_los.time), observation_tca.elevation*180/M_PI);
    }

    std::shared_ptr<Transit> SatelliteTracker::get_next_transit()
    {
        return next_transit;
    }

    TLE SatelliteTracker::get_tle()
    {
        return tle;
    }

    void SatelliteTracker::set_pipeline(std::pair<Pipeline, double> pipeline)
    {
        this->pipeline = pipeline;
    }

    std::pair<Pipeline, double> SatelliteTracker::get_pipeline()
    {
        return pipeline;
    }

    Transit::Transit(double aos,
                     double los,
                     double peak_elevation) : aos(aos),
                                              los(los),
                                              peak_elevation(peak_elevation) {};
}