#pragma once

#include <string>
#include "ShipModel.h"

struct Waypoint
{
    std::string name;
    double latitude;
    double longitude;
};

struct RouteLeg
{
    Waypoint start;
    Waypoint end;
    double distanceNm;
    double headingDeg;
    double weatherSpeedKnots;
    double travelTimeHours;
};

double calculateDistanceNm(Waypoint point1, Waypoint point2);
double calculateBearing(Waypoint point1, Waypoint point2);

RouteLeg calculateRouteLeg(
    Waypoint start,
    Waypoint end,
    ShipModel ship,
    WeatherData weather);
