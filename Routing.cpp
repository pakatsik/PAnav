#include "Routing.h"
#include "Performance.h"
#include <cmath>

// Each route leg is treated as a direct great-circle connection.
// The initial bearing is used as the ship heading for the leg.
// This is considered adequate for the relatively short routes
// used in the simulation.

using namespace std;

const double PI_ROUTE = 3.141592653589793;
const double EARTH_RADIUS_NM = 3440.065;

double routeDegreesToRadians(double degrees)
{
    return degrees * PI_ROUTE / 180.0;
}

double routeRadiansToDegrees(double radians)
{
    return radians * 180.0 / PI_ROUTE;
}

// Haversine distance between two coordinates
double calculateDistanceNm(Waypoint point1, Waypoint point2)
{
    double lat1 = routeDegreesToRadians(point1.latitude);
    double lat2 = routeDegreesToRadians(point2.latitude);
    double deltaLat = routeDegreesToRadians(point2.latitude - point1.latitude);
    double deltaLon = routeDegreesToRadians(point2.longitude - point1.longitude);

    double a =
        sin(deltaLat / 2.0) * sin(deltaLat / 2.0)
        + cos(lat1) * cos(lat2)
        * sin(deltaLon / 2.0) * sin(deltaLon / 2.0);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return EARTH_RADIUS_NM * c;
}

// Initial great-circle bearing from point1 to point2
double calculateBearing(Waypoint point1, Waypoint point2)
{
    double lat1 = routeDegreesToRadians(point1.latitude);
    double lat2 = routeDegreesToRadians(point2.latitude);
    double deltaLon = routeDegreesToRadians(point2.longitude - point1.longitude);

    double y = sin(deltaLon) * cos(lat2);
    double x =
        cos(lat1) * sin(lat2)
        - sin(lat1) * cos(lat2) * cos(deltaLon);

    double bearing = routeRadiansToDegrees(atan2(y, x));

    if (bearing < 0.0)
        bearing += 360.0;

    return bearing;
}

RouteLeg calculateRouteLeg(
    Waypoint start,
    Waypoint end,
    ShipModel ship,
    WeatherData weather)
{
    RouteLeg leg;

    leg.start = start;
    leg.end = end;
    leg.distanceNm = calculateDistanceNm(start, end);
    leg.headingDeg = calculateBearing(start, end);

    PerformanceResult performance =
        calculateWeatherPerformance(ship, weather, leg.headingDeg);

    leg.weatherSpeedKnots = performance.weatherSpeedKnots;

    if (leg.weatherSpeedKnots > 0.0)
        leg.travelTimeHours = leg.distanceNm / leg.weatherSpeedKnots;
    else
        leg.travelTimeHours = 0.0;

    return leg;
}
