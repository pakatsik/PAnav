#include "Wind.h"
#include <cmath>

using namespace std;

const double PI_WIND = 3.141592653589793;
const double RHO_AIR = 1.225;

ApparentWind calculateApparentWind(
    double trueWindSpeed,
    double trueWindAngle,
    double shipSpeed)
{
    double beta = trueWindAngle * PI_WIND / 180.0;

    double x = shipSpeed + trueWindSpeed * cos(beta);
    double y = trueWindSpeed * sin(beta);

    ApparentWind result;
    result.speed = sqrt(x * x + y * y);
    result.angle = atan2(abs(y), x) * 180.0 / PI_WIND;

    return result;
}

// ITTC coefficient data with linear interpolation between tabulated angles
double windCoefficientCDA(double apparentWindAngle)
{
    const int N = 19;

    double angles[N] =
    {
        0, 10, 20, 30, 40,
        50, 60, 70, 80, 90,
        100, 110, 120, 130, 140,
        150, 160, 170, 180
    };

    double Cx[N] =
    {
        -0.871, -0.782, -0.825, -0.733, -0.641,
        -0.493, -0.457, -0.309, -0.130, -0.003,
         0.110,  0.238,  0.420,  0.551,  0.666,
         0.705,  0.706,  0.695,  0.705
    };

    if (apparentWindAngle <= 0.0)
        return -Cx[0];

    if (apparentWindAngle >= 180.0)
        return -Cx[N - 1];

    for (int i = 0; i < N - 1; i++)
    {
        if (apparentWindAngle >= angles[i]
            && apparentWindAngle <= angles[i + 1])
        {
            double fraction =
                (apparentWindAngle - angles[i])
                / (angles[i + 1] - angles[i]);

            double interpolatedCx =
                Cx[i] + fraction * (Cx[i + 1] - Cx[i]);

            return -interpolatedCx;
        }
    }

    return 0.0;
}

double addedWindResistance(
    double trueWindSpeed,
    double trueWindAngle,
    double shipSpeed,
    double frontalArea)
{
    ApparentWind apparent = calculateApparentWind(
        trueWindSpeed, trueWindAngle, shipSpeed);

    double CDA = windCoefficientCDA(apparent.angle);
    double CDA0 = windCoefficientCDA(0.0);

    double windForce =
        0.5 * RHO_AIR * frontalArea * CDA
        * apparent.speed * apparent.speed;

    double calmAirForce =
        0.5 * RHO_AIR * frontalArea * CDA0
        * shipSpeed * shipSpeed;

    return windForce - calmAirForce;
}
