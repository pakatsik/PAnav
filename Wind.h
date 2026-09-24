#pragma once

struct ApparentWind
{
    double speed;
    double angle;
};

ApparentWind calculateApparentWind(
    double trueWindSpeed,
    double trueWindAngle,
    double shipSpeed);

double windCoefficientCDA(double apparentWindAngle);

double addedWindResistance(
    double trueWindSpeed,
    double trueWindAngle,
    double shipSpeed,
    double frontalArea);