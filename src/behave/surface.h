#ifndef SURFACE_HEADER
#define SURFACE_HEADER

// The SURFACE module of BehavePlus
#include "surfaceFireSpread.h"

class SurfaceInputs;

class Surface
{
public:
    Surface(const FuelModelSet& fuelModelSet, SurfaceInputs& surfaceInputs);
    Surface(const Surface &rhs);
    Surface& operator= (const Surface& rhs);

    double calculateSurfaceFireForwardSpreadRate(double directionOfinterest = -1.0);
    double calculateSpreadRateAtVector(double directionOfinterest);
    double getSpreadRate() const;
    double getDirectionOfMaxSpread() const;
    double getFlameLength() const;
    double getFireLengthToWidthRatio() const;
    double getFireEccentricity() const;
    double getFirelineIntensity() const;
    double getHeatPerUnitArea() const;
    double getMidflameWindspeed() const;
    double getEllipticalA() const;
    double getEllipticalB() const;
    double getEllipticalC() const;
   

private:
    bool isUsingTwoFuelModels() const;

    // SURFACE Module component objects
    const FuelModelSet*	fuelModelSet_;
    SurfaceFireSpread surfaceFireSpread_;
    SurfaceInputs* surfaceInputs_;
};

#endif //SURFACEFIRE_HEADER
