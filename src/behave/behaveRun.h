// Interface for Behave application based on the Facade OOP Design Pattern
// It is used to tie together the modules and objects used by Behave - WMC 10/2015

// TODO: Need to revisit how the Two Fuel Models module is organized and handled - WMC 02/2016

#ifndef BEHAVERUN_HEADER
#define BEHAVERUN_HEADER

// Surface
#include "surface.h"
#include "surfaceInputs.h"

// Crown
#include "crown.h"
#include "crownInputs.h"

class FuelModels;

class BehaveRun
{
public:
    BehaveRun() = delete; // There is no default constructor
    explicit BehaveRun(FuelModelSet &fuelModelSet);
    
    BehaveRun(const BehaveRun &rhs);
    BehaveRun& operator= (const BehaveRun& rhs);
    ~BehaveRun();

    // SURFACE Module Inputs
    void setFuelModelSet(FuelModelSet& fuelModelSet);
    void setFuelModelNumber(int fuelModelNumber);
    void setMoistureOneHour(double moistureOneHour);
    void setMoistureTenHour(double moistureTenHour);
    void setMoistureHundredHour(double moistureHundredHour);
    void setMoistureLiveHerbaceous(double moistureLiveHerbaceous);
    void setMoistureLiveWoody(double moistureLiveWoody);
    void setSlope(double slope);
    void setAspect(double aspect);
    void setWindSpeed(double windSpeed);
    void setWindDirection(double windDirection);
    void setWindHeightInputMode(WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode);
    void setWindAndSpreadAngleOrientationMode(WindAndSpreadAngleMode::WindAndSpreadAngleModeEnum windAndSpreadAngleMode);
    void setTwoFuelModelsMethod(TwoFuelModels::TwoFuelModelsEnum twoFuelModelsMethod);
    void setSlopeInputMode(SlopeInputMode::SlopeInputModeEnum slopeInputMode);
    void setFirstFuelModelNumber(int firstFuelModelNumber);
    void setSecondFuelModelNumber(int secondFuelModelNumber);

    void updateSurfaceInputs(int fuelModelNumber, double moistureOneHour, double moistureTenHour, double moistureHundredHour,
        double moistureLiveHerbaceous, double moistureLiveWoody, WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode,
        double windSpeed, double windDirection, double slope, double aspect, double canopyCover, double canopyHeight, double crownRatio);
    void updateSurfaceInputsForTwoFuelModels(int firstfuelModelNumber, int secondFuelModelNumber, double moistureOneHour,
        double moistureTenHour, double moistureHundredHour, double moistureLiveHerbaceous, double moistureLiveWoody,
        WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode, double windSpeed, double windDirection,
        double firstFuelModelCoverage, TwoFuelModels::TwoFuelModelsEnum twoFuelModelsMethod, double slope, double aspect,
        double canopyCover, double canopyHeight, double crownRatio);
    void updateSurfaceInputsForPalmettoGallbery(double moistureOneHour, double moistureTenHour, double moistureHundredHour,
        double moistureLiveHerbaceous, double moistureLiveWoody, WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode,
        double windSpeed, double windDirection, double ageOfRough, double heightOfUnderstory, double palmettoCoverage,
        double overstoryBasalArea, double slope, double aspect, double canopyCover, double canopyHeight, double crownRatio);
    void updateSurfaceInputsForPalmettoGallberyModelsWithMoistureByLifeState(double moistureDead, double moistureLive, 
        WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode, double windSpeed, double windDirection, double ageOfRough,
        double heightOfUnderstory, double palmettoCoverage, double overstoryBasalArea, double slope, double aspect, double canopyCover,
        double canopyHeight, double crownRatio);
    void updateSurfaceInputsForWesternAspen(int aspenFuelModelNumber, double aspenCuringLevel, 
        AspenFireSeverity::AspenFireSeverityEnum aspenFireSeverity, double DBH, double moistureOneHour, double moistureTenHour, 
        double moistureHundredHour, double moistureLiveHerbaceous, double moistureLiveWoody,
        WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode, double windSpeed, double windDirection, double slope,
        double aspect, double canopyCover, double canopyHeight, double crownRatio);
    double calculateSurfaceFireForwardSpreadRate(double directionOfInterest = -1.0);

    // SURFACE Module Getters
    double getSpreadRate() const;
    double getDirectionOfMaxSpread() const;
    double getFlameLength() const;
    double getFireLengthToWidthRatio() const;
    double getFireEccentricity() const;
    double getSurfaceFirelineIntensity() const;
    double getMidflameWindspeed() const;
    double getEllipticalA() const;
    double getEllipticalB() const;
    double getEllipticalC() const;

    bool isFuelModelDefined(int fuelModelNumber) const;
    bool isWindAndSpreadAngleRelativeToNorth() const;
    bool isWindAndSpreadAngleRelativeToUpslope() const;
    bool isSlopeInDegrees() const;
    bool isSlopeInPercent() const;
    
private:
    FuelModelSet* fuelModelSet_;    // This must point to a valid reference passed to the constructor

    // SURFACE Module Components
    Surface surface_;               // SURFACE Module object
    SurfaceInputs surfaceInputs_;   // Object that manages user input to SURFACE Module

    // CROWN Module Components
    Crown crown_;
    CrownInputs crownInputs_;
};

#endif //BehaveRun_HEADER
