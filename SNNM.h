#pragma once

double knotsToMps(double speedKnots);
double degreesToRadians(double degrees);
double relativeAngle(double direction, double shipHeading);

double entranceAngle(double beam, double Le);
double runAngle(double beam, double Lr);

double meanAddedWaveResistanceJONSWAP(
    double Hs,
    double Tp,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double kyyRatio,
    double E1,
    double E2);
