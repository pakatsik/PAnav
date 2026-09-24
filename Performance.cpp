#include "Performance.h"
#include "SNNM.h"
#include "Wind.h"
#include <cmath>

// The calm-water power curve is approximated with P = k * V^3.
// The weather resistance is converted to additional power using R * V.
// Ship speed is reduced iteratively until the required power does not
// exceed the available propulsion power.

using namespace std;

const double G_PERFORMANCE = 9.80665;
const double AVAILABLE_POWER_KW = 7300.0;
const double SEA_MARGIN = 0.15;
const double REFERENCE_CALM_POWER_KW =
AVAILABLE_POWER_KW / (1.0 + SEA_MARGIN);

struct ResistancePoint
{
    double wave;
    double wind;
    double total;
};

struct PowerPoint
{
    double calmPower;
    double weatherPower;
    double totalPower;
    ResistancePoint resistance;
};

double mpsToKnots(double speedMps)
{
    return speedMps / 0.514444;
}

// Simplified cubic speed-power relation around the reference condition
double calmWaterPower(double speedMps, double referenceSpeedMps)
{
    return REFERENCE_CALM_POWER_KW
        * pow(speedMps / referenceSpeedMps, 3.0);
}

ResistancePoint resistanceAtSpeed(
    ShipModel ship,
    WeatherData weather,
    double shipHeading,
    double speedMps)
{
    double relativeWaveAngle =
        relativeAngle(weather.waveDirection, shipHeading);

    double relativeWindAngle =
        relativeAngle(weather.windDirection, shipHeading);

    double alpha = degreesToRadians(relativeWaveAngle);
    double E1 = entranceAngle(ship.beam, ship.Le);
    double E2 = runAngle(ship.beam, ship.Lr);

    ResistancePoint result;

    result.wave = meanAddedWaveResistanceJONSWAP(
        weather.Hs,
        weather.Tp,
        alpha,
        speedMps,
        ship.Lpp,
        ship.beam,
        ship.draft,
        ship.Cb,
        ship.kyyRatio,
        E1,
        E2);

    result.wind = addedWindResistance(
        weather.windSpeed,
        relativeWindAngle,
        speedMps,
        ship.frontalWindArea);

    result.total = result.wave + result.wind;

    return result;
}

PowerPoint requiredPowerAtSpeed(
    ShipModel ship,
    WeatherData weather,
    double shipHeading,
    double speedMps,
    double referenceSpeedMps)
{
    PowerPoint result;

    result.resistance = resistanceAtSpeed(
        ship, weather, shipHeading, speedMps);

    result.calmPower = calmWaterPower(speedMps, referenceSpeedMps);

    // Added resistance times speed, converted from W to kW
    result.weatherPower =
        result.resistance.total * speedMps / 1000.0;

    result.totalPower = result.calmPower + result.weatherPower;

    return result;
}

PerformanceResult calculateWeatherPerformance(
    ShipModel ship,
    WeatherData weather,
    double shipHeading)
{
    double referenceSpeedMps = knotsToMps(ship.serviceSpeed);

    // Fr = 0.09 is the lower limit used for the SNNM speed range
    double minimumSNNMSpeed =
        0.09 * sqrt(G_PERFORMANCE * ship.Lpp);

    double weatherSpeedMps = referenceSpeedMps;
    bool withinSNNMRange = true;

    PowerPoint referencePoint = requiredPowerAtSpeed(
        ship,
        weather,
        shipHeading,
        referenceSpeedMps,
        referenceSpeedMps);

    if (referencePoint.totalPower > AVAILABLE_POWER_KW)
    {
        PowerPoint minimumPoint = requiredPowerAtSpeed(
            ship,
            weather,
            shipHeading,
            minimumSNNMSpeed,
            referenceSpeedMps);

        if (minimumPoint.totalPower > AVAILABLE_POWER_KW)
        {
            weatherSpeedMps = minimumSNNMSpeed;
            withinSNNMRange = false;
        }
        else
        {
            // Bisection between the minimum valid speed and service speed
            double low = minimumSNNMSpeed;
            double high = referenceSpeedMps;

            for (int i = 0; i < 40; i++)
            {
                double middle = (low + high) / 2.0;

                PowerPoint middlePoint = requiredPowerAtSpeed(
                    ship,
                    weather,
                    shipHeading,
                    middle,
                    referenceSpeedMps);

                if (middlePoint.totalPower > AVAILABLE_POWER_KW)
                    high = middle;
                else
                    low = middle;
            }

            weatherSpeedMps = (low + high) / 2.0;
        }
    }

    PowerPoint finalPoint = requiredPowerAtSpeed(
        ship,
        weather,
        shipHeading,
        weatherSpeedMps,
        referenceSpeedMps);

    PerformanceResult result;

    result.weatherSpeedMps = weatherSpeedMps;
    result.weatherSpeedKnots = mpsToKnots(weatherSpeedMps);
    result.speedLossKnots = ship.serviceSpeed - result.weatherSpeedKnots;

    result.waveResistance = finalPoint.resistance.wave;
    result.windResistance = finalPoint.resistance.wind;
    result.totalWeatherResistance = finalPoint.resistance.total;

    result.calmWaterPower = finalPoint.calmPower;
    result.weatherPower = finalPoint.weatherPower;
    result.totalRequiredPower = finalPoint.totalPower;
    result.availablePower = AVAILABLE_POWER_KW;
    result.withinSNNMRange = withinSNNMRange;

    return result;
}
