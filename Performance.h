#pragma once

#include "ShipModel.h"

struct PerformanceResult
{
    double weatherSpeedMps;
    double weatherSpeedKnots;
    double speedLossKnots;

    double waveResistance;
    double windResistance;
    double totalWeatherResistance;

    double calmWaterPower;
    double weatherPower;
    double totalRequiredPower;
    double availablePower;

    bool withinSNNMRange;
};

PerformanceResult calculateWeatherPerformance(
    ShipModel ship,
    WeatherData weather,
    double shipHeading);
