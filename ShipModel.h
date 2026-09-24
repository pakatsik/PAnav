#pragma once

struct ShipModel
{
    double Lpp;
    double beam;
    double draft;
    double Cb;
    double serviceSpeed;

    double Le;
    double Lr;
    double kyyRatio;
    double frontalWindArea;
};

struct WeatherData
{
    double Hs;
    double Tp;
    double waveDirection;
    double windSpeed;
    double windDirection;
};
