#include "SNNM.h"
#include <cmath>

// Added wave resistance is calculated using the SNNM method.
// Regular-wave resistance is evaluated for each wave frequency
// then integrated over a JONSWAP spectrum for irregular seas.

using namespace std;

const double PI = 3.141592653589793;
const double G = 9.80665;
const double RHO_WATER = 1025.0;
const double JONSWAP_GAMMA = 3.3;

// Basic calculations

double knotsToMps(double speedKnots)
{
    return speedKnots * 0.514444;
}

double froudeNumber(double speedMps, double Lpp)
{
    return speedMps / sqrt(G * Lpp);
}

double degreesToRadians(double degrees)
{
    return degrees * PI / 180.0;
}

// Angles

double normalizeAngle180(double angle)
{
    while (angle > 180.0)
        angle -= 360.0;

    while (angle < -180.0)
        angle += 360.0;

    return angle;
}

double relativeAngle(double direction, double shipHeading)
{
    return abs(normalizeAngle180(direction - shipHeading));
}

// Approximate waterline geometry

double entranceAngle(double beam, double Le)
{
    return atan(0.495 * beam / Le);
}

double runAngle(double beam, double Lr)
{
    return atan(0.495 * beam / Lr);
}

// SNNM motion-induced added resistance

double groupVelocity(double omega)
{
    return G / (2.0 * omega);
}

double relativeFroudeNumber(double speedMps,
    double Vg,
    double Lpp)
{
    return (speedMps - Vg) / sqrt(G * Lpp);
}

double normalizedWaveFrequency(double omega,
    double alpha,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double kyyRatio,
    double Fr)
{
    double term1 = 2.142 * cbrt(kyyRatio);

    double term2 =
        sqrt(Lpp / (2.0 * PI * G));

    double term3 =
        pow(Cb / 0.65, 0.17);

    double term4 =
        1.0 - (0.111 / Cb)
        * (log(beam / draft) - log(2.75));

    double headingTerm =
        (-1.377 * Fr * Fr + 1.157 * Fr)
        * abs(cos(alpha))
        + 0.618
        * (13.0 + cos(2.0 * alpha))
        / 14.0;

    return term1 * term2 * term3
        * term4 * headingTerm * omega;
}

// a1 for head-to-beam waves
double a1HeadBeam(double alpha,
    double Cb,
    double beam,
    double draft,
    double Fr)
{
    return
        pow(0.87 / Cb,
            (1.0 + Fr) * cos(alpha))
        * pow(log(beam / draft), -1.0)
        * (1.0 + 2.0 * cos(alpha)) / 3.0;
}

// a2 for head-to-beam waves
double a2HeadBeam(double Fr)
{
    if (Fr < 0.12)
        return 0.0072 + 0.1676 * Fr;

    return pow(Fr, 1.5) * exp(-3.5 * Fr);
}

// a1 for following waves
double a1Following(double Cb,
    double beam,
    double draft,
    double speedMps,
    double Vg,
    double FrRel)
{
    if (speedMps > Vg && FrRel >= 0.12)
    {
        return
            pow(0.87 / Cb, 1.0 + FrRel)
            * pow(log(beam / draft), -1.0);
    }

    return
        (0.87 / Cb)
        * pow(log(beam / draft), -1.0);
}

// a2 for following waves
double a2Following(double speedMps,
    double Vg,
    double FrRel)
{
    if (speedMps <= Vg)
        return 0.0072 * (2.0 * speedMps / Vg - 1.0);

    if (FrRel < 0.12)
        return 0.0072 + 0.1676 * FrRel;

    return pow(FrRel, 1.5) * exp(-3.5 * FrRel);
}

double a3ZeroTrim()
{
    return 1.0;
}

double b1Coefficient(double omegaBar)
{
    if (omegaBar < 1.0)
        return 11.0;

    return -8.5;
}

double d1Coefficient(double omegaBar,
    double Lpp,
    double beam,
    double Cb)
{
    if (omegaBar < 1.0)
    {
        return
            566.0
            * pow((Lpp * Cb) / beam, -2.66);
    }

    return
        -566.0
        * pow(Lpp / beam, -2.66)
        * 4.0;
}

// Common RAWM equation
double calculateRAWM(double beam,
    double Lpp,
    double Cb,
    double kyyRatio,
    double a1,
    double a2,
    double a3,
    double omegaBar,
    double b1,
    double d1)
{
    return
        3859.2 * RHO_WATER * G
        * (beam * beam / Lpp)
        * pow(Cb, 1.34)
        * pow(kyyRatio, 2.0)
        * a1 * a2 * a3
        * pow(omegaBar, b1)
        * exp(
            (b1 / d1)
            * (1.0 - pow(omegaBar, d1))
        );
}

// RAWM for head-to-beam waves
double motionResistanceTransferHeadBeam(double omega,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double kyyRatio)
{
    double Fr = froudeNumber(speedMps, Lpp);

    double omegaBar =
        normalizedWaveFrequency(
            omega, alpha, Lpp, beam,
            draft, Cb, kyyRatio, Fr);

    double a1 =
        a1HeadBeam(alpha, Cb, beam, draft, Fr);

    double a2 = a2HeadBeam(Fr);
    double a3 = a3ZeroTrim();
    double b1 = b1Coefficient(omegaBar);
    double d1 = d1Coefficient(omegaBar, Lpp, beam, Cb);

    return calculateRAWM(
        beam, Lpp, Cb, kyyRatio,
        a1, a2, a3, omegaBar, b1, d1);
}

// RAWM for following waves
double motionResistanceTransferFollowing(double omega,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double kyyRatio)
{
    double alpha = PI;
    double Fr = froudeNumber(speedMps, Lpp);
    double Vg = groupVelocity(omega);

    double FrRel =
        relativeFroudeNumber(speedMps, Vg, Lpp);

    double omegaBar =
        normalizedWaveFrequency(
            omega, alpha, Lpp, beam,
            draft, Cb, kyyRatio, Fr);

    double a1 =
        a1Following(
            Cb, beam, draft,
            speedMps, Vg, FrRel);

    double a2 =
        a2Following(speedMps, Vg, FrRel);

    double a3 = a3ZeroTrim();
    double b1 = b1Coefficient(omegaBar);
    double d1 = d1Coefficient(omegaBar, Lpp, beam, Cb);

    return calculateRAWM(
        beam, Lpp, Cb, kyyRatio,
        a1, a2, a3, omegaBar, b1, d1);
}

// RAWM for any wave angle
double motionResistanceTransfer(double omega,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double kyyRatio)
{
    if (alpha <= PI / 2.0)
    {
        return motionResistanceTransferHeadBeam(
            omega, alpha, speedMps,
            Lpp, beam, draft, Cb, kyyRatio);
    }

    // Stern-oblique waves: interpolate between beam and following waves
    double beamValue =
        motionResistanceTransferHeadBeam(
            omega, PI / 2.0, speedMps,
            Lpp, beam, draft, Cb, kyyRatio);

    double followingValue =
        motionResistanceTransferFollowing(
            omega, speedMps,
            Lpp, beam, draft, Cb, kyyRatio);

    double interpolationFactor =
        (alpha - PI / 2.0) / (PI / 2.0);

    return
        beamValue
        + interpolationFactor
        * (followingValue - beamValue);
}

// SNNM wave-reflection component

// Draft coefficient alpha_T*
double draftCoefficient(double omega,
    double Lpp,
    double Tstar)
{
    double wavelength =
        2.0 * PI * G / (omega * omega);

    double wavelengthRatio =
        wavelength / Lpp;

    if (wavelengthRatio > 2.5)
        return 0.0;

    return
        1.0 -
        exp(
            -4.0 * PI
            * (
                Tstar / wavelength
                - Tstar / (2.5 * Lpp)
                )
        );
}

double reflectionHeadingFunction(double alpha,
    double E1)
{
    if (alpha <= E1)
        return cos(alpha);

    return 0.0;
}

// T* used for stern reflection components
double sternEffectiveDraft(double draft,
    double Cb,
    double alpha)
{
    if (Cb <= 0.75)
    {
        return
            draft
            * (4.0 + sqrt(abs(cos(alpha))))
            / 5.0;
    }

    return
        draft
        * (2.0 + sqrt(abs(cos(alpha))))
        / 3.0;
}

double reflectionComponent1(double omega,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double Fr,
    double E1)
{
    if (alpha > PI - E1)
        return 0.0;

    double alphaT =
        draftCoefficient(omega, Lpp, draft);

    double fAlpha =
        reflectionHeadingFunction(alpha, E1);

    double speedFactor =
        2.0 * omega * speedMps / G;

    double geometryTerm =
        pow(sin(E1 + alpha), 2.0)
        + speedFactor
        * (
            cos(alpha)
            - cos(E1) * cos(E1 + alpha)
            );

    double CbFactor =
        pow(
            0.87 / Cb,
            (1.0 + 4.0 * sqrt(Fr)) * fAlpha
        );

    return
        (2.25 / 4.0)
        * RHO_WATER * G * beam
        * alphaT * geometryTerm * CbFactor;
}

double reflectionComponent2(double omega,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double Fr,
    double E1)
{
    if (alpha > E1)
        return 0.0;

    double alphaT =
        draftCoefficient(omega, Lpp, draft);

    double fAlpha =
        reflectionHeadingFunction(alpha, E1);

    double speedFactor =
        2.0 * omega * speedMps / G;

    double geometryTerm =
        pow(sin(E1 - alpha), 2.0)
        + speedFactor
        * (
            cos(alpha)
            - cos(E1) * cos(E1 - alpha)
            );

    double CbFactor =
        pow(
            0.87 / Cb,
            (1.0 + 4.0 * sqrt(Fr)) * fAlpha
        );

    return
        (2.25 / 4.0)
        * RHO_WATER * G * beam
        * alphaT * geometryTerm * CbFactor;
}

double reflectionComponent3(double omega,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double E2)
{
    if (alpha < E2)
        return 0.0;

    double Tstar =
        sternEffectiveDraft(draft, Cb, alpha);

    double alphaT =
        draftCoefficient(omega, Lpp, Tstar);

    double speedFactor =
        2.0 * omega * speedMps / G;

    double geometryTerm =
        pow(sin(E2 - alpha), 2.0)
        + speedFactor
        * (
            cos(alpha)
            - cos(E2) * cos(E2 - alpha)
            );

    return
        -(2.25 / 4.0)
        * RHO_WATER * G * beam
        * alphaT * geometryTerm;
}

double reflectionComponent4(double omega,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double E2)
{
    if (alpha < PI - E2)
        return 0.0;

    double Tstar =
        sternEffectiveDraft(draft, Cb, alpha);

    double alphaT =
        draftCoefficient(omega, Lpp, Tstar);

    double speedFactor =
        2.0 * omega * speedMps / G;

    double geometryTerm =
        pow(sin(E2 + alpha), 2.0)
        + speedFactor
        * (
            cos(alpha)
            - cos(E2) * cos(E2 + alpha)
            );

    return
        -(2.25 / 4.0)
        * RHO_WATER * G * beam
        * alphaT * geometryTerm;
}

double reflectionResistanceTransfer(double omega,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double E1,
    double E2)
{
    double Fr = froudeNumber(speedMps, Lpp);

    double rawr1 =
        reflectionComponent1(
            omega, alpha, speedMps,
            Lpp, beam, draft, Cb, Fr, E1);

    double rawr2 =
        reflectionComponent2(
            omega, alpha, speedMps,
            Lpp, beam, draft, Cb, Fr, E1);

    double rawr3 =
        reflectionComponent3(
            omega, alpha, speedMps,
            Lpp, beam, draft, Cb, E2);

    double rawr4 =
        reflectionComponent4(
            omega, alpha, speedMps,
            Lpp, beam, draft, Cb, E2);

    return rawr1 + rawr2 + rawr3 + rawr4;
}

// JONSWAP irregular-wave spectrum

// JONSWAP spectral density S(omega)
double jonswapSpectrum(double omega,
    double Hs,
    double Tp)
{
    if (omega <= 0.0 || Hs <= 0.0 || Tp <= 0.0)
        return 0.0;

    double omegaP = 2.0 * PI / Tp;

    // Spectral width parameter
    double sigma;

    if (omega <= omegaP)
        sigma = 0.07;
    else
        sigma = 0.09;

    // Peak enhancement exponent
    double peakExponent =
        exp(
            -pow(omega - omegaP, 2.0)
            / (2.0 * sigma * sigma * omegaP * omegaP)
        );

    double baseSpectrum =
        (5.0 / 16.0)
        * Hs * Hs
        * pow(omegaP, 4.0)
        / pow(omega, 5.0)
        * exp(
            -1.25
            * pow(omegaP / omega, 4.0)
        );

    double gammaCorrection =
        1.0 - 0.287 * log(JONSWAP_GAMMA);

    return
        baseSpectrum
        * pow(JONSWAP_GAMMA, peakExponent)
        * gammaCorrection;
}

// Total regular-wave transfer function
double totalWaveResistanceTransfer(double omega,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double kyyRatio,
    double E1,
    double E2)
{
    double rawm =
        motionResistanceTransfer(
            omega, alpha, speedMps,
            Lpp, beam, draft,
            Cb, kyyRatio);

    double rawr =
        reflectionResistanceTransfer(
            omega, alpha, speedMps,
            Lpp, beam, draft,
            Cb, E1, E2);

    return rawm + rawr;
}

// Mean added resistance in irregular waves
double meanAddedWaveResistanceJONSWAP(double Hs,
    double Tp,
    double alpha,
    double speedMps,
    double Lpp,
    double beam,
    double draft,
    double Cb,
    double kyyRatio,
    double E1,
    double E2)
{
    if (Hs <= 0.0)
        return 0.0;

    double omegaP = 2.0 * PI / Tp;

    // Numerical integration range
    double omegaMin = 0.2 * omegaP;
    double omegaMax = 4.0 * omegaP;

    const int N = 1000;

    double dOmega =
        (omegaMax - omegaMin) / N;

    double integral = 0.0;

    for (int i = 0; i <= N; i++)
    {
        double omega =
            omegaMin + i * dOmega;

        double spectrum =
            jonswapSpectrum(
                omega,
                Hs,
                Tp);

        double transfer =
            totalWaveResistanceTransfer(
                omega,
                alpha,
                speedMps,
                Lpp,
                beam,
                draft,
                Cb,
                kyyRatio,
                E1,
                E2);

        double value =
            transfer * spectrum;

        // Trapezoidal rule: first and last points have half weight
        if (i == 0 || i == N)
            value *= 0.5;

        integral += value;
    }

    integral *= dOmega;

    // Convert wave-elevation variance to amplitude-squared contribution
    return 2.0 * integral;
}
