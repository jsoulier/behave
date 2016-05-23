#include "crown.h"

#include <cmath>
#include "fuelModelSet.h"
#include "surfaceEnums.h"
#include "windSpeedUtility.h"

Crown::Crown(const FuelModelSet& fuelModelSet, const CrownInputs& crownInputs, const SurfaceInputs& surfaceInputs, 
    const Surface& surface)
    : crownFireSpread_(fuelModelSet, crownDeepCopyOfSurfaceInputs_)
{
    crownInputs_ = &crownInputs;  // point to the same location as crownInputs
    fuelModelSet_ = &fuelModelSet; // point to the same location as fuelModels
    surfaceInputs_ = &surfaceInputs; // point to the same location as surfaceInputs
    surface_ = &surface; // point to the same location as surface

    crownDeepCopyOfSurfaceInputs_ = *surfaceInputs_; // copy the actual data surfaceInputs is pointing to
}

Crown::~Crown()
{

}

Crown::Crown(const Crown &rhs)
{
    fuelModelSet_ = rhs.fuelModelSet_;
    surface_ = rhs.surface_;
    surfaceInputs_ = rhs.surfaceInputs_;
    crownInputs_ = rhs.crownInputs_;
}

Crown& Crown::operator= (const Crown& rhs)
{
    if (this != &rhs)
    {
        fuelModelSet_ = rhs.fuelModelSet_;
        surface_ = rhs.surface_;
        surfaceInputs_ = rhs.surfaceInputs_;
        crownInputs_ = rhs.crownInputs_;
    }
    return *this;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire spread rate.
*
*  This uses Rothermel's 1991 crown fire correlation.
*
*  \return Crown fire average spread rate (ft/min).
*
*/
double Crown::calculateCrownFireSpreadRate()
{
    // Step 1: Create the crown fuel model (fire behavior fuel model 10)
    crownDeepCopyOfSurfaceInputs_.setFuelModelNumber(10);    // set the fuel model used to fuel model 10
    crownDeepCopyOfSurfaceInputs_.setSlope(0.0);             // slope is always assumed to be zero in crown ROS
    crownDeepCopyOfSurfaceInputs_.setWindDirection(0.0);     // wind direction is assumed to be upslope in crown ROS
    double windAdjustmentFactor = 0.4;      // wind adjustment factor is assumed to be 0.4 for crown ROS

    windSpeedAtTwentyFeet_ = calculateWindSpeedAtTwentyFeet();
    double midflameWindSpeed = windAdjustmentFactor * windSpeedAtTwentyFeet_;
    crownDeepCopyOfSurfaceInputs_.setWindSpeed(midflameWindSpeed);

    // Step 2: Determine fire behavior.
    crownFireSpreadRate_ = 3.34 * crownFireSpread_.calculateForwardSpreadRate();  // Rothermel 1991

    //  Step 3:  Get values from Surface needed for further calculations 
    crownCopyOfSurfaceHeatPerUnitArea_ = surface_->getHeatPerUnitArea();
    crownCopyOfSurfaceFirelineIntensity_ = surface_->getFirelineIntensity();

    //  Step 4: Calculate remaining crown fire characteristics
    calculateCrownFuelLoad();
    calculateCanopyHeatPerUnitArea();
    calculateCrownFireHeatPerUnitArea();
    calculateCrownFirelineIntensity();
    calculateCrownFlameLength();

    calculateCrownCriticalFireSpreadRate();
    calculateCrownCriticalSurfaceFireIntensity();
    calculateCrownCriticalSurfaceFlameLength();

    calculateCrownPowerOfFire();
    calcuateCrownPowerOfWind();

    return crownFireSpreadRate_;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the canopy portion of the crown fire heat per unit area
*  given the crown fire fuel load and low heat of combustion.
*
*/
void Crown::calculateCanopyHeatPerUnitArea()
{
    const double LOW_HEAT_OF_COMBUSTION = 8000.0; // Low heat of combustion (hard coded to 8000 Btu/lbs)
    canopyHeatPerUnitArea_ = crownFuelLoad_ * LOW_HEAT_OF_COMBUSTION;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the total crown fire heat per unit area
*  by summing surface HPUA and canopy HPUA.
*
*/
void Crown::calculateCrownFireHeatPerUnitArea()
{
    crownFireHeatPerUnitArea_ = crownCopyOfSurfaceHeatPerUnitArea_ + canopyHeatPerUnitArea_;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire fuel load
*  given the canopy bulk density and canopy height.
*
*  \return Crown fire fuel load (lb/ft2).
*/
void Crown::calculateCrownFuelLoad()
{
    double canopyBulkDensity = crownInputs_->getCanopyBulkDensity();
    double canopyBaseHeight = crownInputs_->getCanopyBaseHeight();
    double canopyHeight = surfaceInputs_->getCanopyHeight();
    crownFuelLoad_ = canopyBulkDensity * (canopyHeight - canopyBaseHeight);
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire transition ratio.
*
*  given the surface fireline intensity (Btu/ft/s) and the
*  critical crown fire fireline intensity (Btu/ft/s).
*
*  \return Transition ratio.
*/
double Crown::calculateCrownFireTransitionRatio()
{
    double crownFireTransitionRatio = ((crownCriticalSurfaceFireIntensity_ < 1.0e-7)
        ? (0.00)
        : (crownCopyOfSurfaceFirelineIntensity_ / crownCriticalSurfaceFireIntensity_));

    return crownFireTransitionRatio;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire fireline intensity
*  given the surface fire and crown fire heats per unit area
*  and the crown fire spread rate.
*
*  \param crownFireHpua Crown fire (surface + canopy) heat per unit area (Btu/ft2).
*  \param crownFireSpreadRate Crown fire rate of spread (ft/min).
*
*  \return Crown fire fireline intensity (Btu/ft/s).
*/
void Crown::calculateCrownFirelineIntensity()
{
    crownFirelineIntensity_ = (crownFireSpreadRate_ / 60.0) * crownFireHeatPerUnitArea_;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the critical surface fire intensity for a surface fire
*  to transition to a crown fire given the foliar moisture and crown base height
*
*  \return Critical surface fire intensity (Btu/ft/s).
*/
double Crown::calculateCrownCriticalSurfaceFireIntensity()
{
    const double KILOWATTS_PER_METER_TO_BTUS_PER_FOOT_PER_SECOND = 0.288672;
    const double FEET_TO_METERS = 0.3048;

    double foliarMoisture = crownInputs_->getFoliarMoisture();

    // Convert foliar moisture content to percent and constrain lower limit
    // double foliarMoisture *= 100.0;
    foliarMoisture = (foliarMoisture < 30.0) ? 30.0 : foliarMoisture;

    double crownBaseHeight = crownInputs_->getCanopyBaseHeight();
    // Convert crown base heigt to meters and constrain lower limit
    crownBaseHeight *= FEET_TO_METERS;
    crownBaseHeight = (crownBaseHeight < 0.1) ? 0.1 : crownBaseHeight;
    // Critical surface fireline intensity (kW/m)
    // Need to changed value in calculation below from 450 to 460 at some point
    crownCriticalSurfaceFireIntensity_ = pow((0.010 * crownBaseHeight * (450.0 + 25.9 * foliarMoisture)), 1.5);

    // Return as Btu/ft/s
    crownCriticalSurfaceFireIntensity_ *= KILOWATTS_PER_METER_TO_BTUS_PER_FOOT_PER_SECOND;

    return crownCriticalSurfaceFireIntensity_;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the critical surface fire flame length for a surface fire
*  to transition to a crown fire given the critical fireline intensity.
*
*  \return Critical surface fire flame length (ft).
*/
double Crown::calculateCrownCriticalSurfaceFlameLength()
{
    double criticalSurfaceFlameLength = crownFireSpread_.calculateFlameLength(crownCriticalSurfaceFireIntensity_);
    return criticalSurfaceFlameLength;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire flame length
*  given the crown fireline intensity.
*
*  \return Crown fire flame length (ft).
*/
double Crown::calculateCrownFlameLength()
{
    crownFlameLength_ = 0.2 * pow(crownFirelineIntensity_, (2.0 / 3.0));
    return crownFlameLength_;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire 'power of the fire'
*  given the crown fireline intensity.
*
*  \return Crown fire 'power of the fire' (ft-lb/s/ft2).
*/
void Crown::calculateCrownPowerOfFire()
{
    crownPowerOfFire_ = crownFirelineIntensity_ / 129.0;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire 'power of the wind'
*  given the 20-ft wind speed and crown fire spread rate.
*
*  \return Crown fire 'power of the wind' (ft-lb/s/ft2).
*/
void Crown::calcuateCrownPowerOfWind()
{
    const double MILES_PER_HOUR_TO_FEET_PER_MINUTE = 5280.0 / 60.0;
    const double SECONDS_PER_MINUTE = 60.0;
    
    windSpeedAtTwentyFeet_ = calculateWindSpeedAtTwentyFeet();

    double windSpeedInFeetPerMinute = windSpeedAtTwentyFeet_ * MILES_PER_HOUR_TO_FEET_PER_MINUTE;

    double WindspeedMinusCrownROS = 0.0; // Eq. 7, Rothermel 1991

    WindspeedMinusCrownROS = (windSpeedInFeetPerMinute - crownFireSpreadRate_) / SECONDS_PER_MINUTE;
    WindspeedMinusCrownROS = (WindspeedMinusCrownROS < 1e-07) ? 0.0 : WindspeedMinusCrownROS;
    crownPowerOfWind_ = 0.00106 * (WindspeedMinusCrownROS * WindspeedMinusCrownROS * WindspeedMinusCrownROS);
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire 'power-of-fire to power-of-wind' ratio
*
*  \return Ratio of the crown fire 'power-of-the-fire' to 'power-of-the-wind).
*/
double Crown::calcualteCrownFirePowerRatio()
{
    crownFirePowerRatio_ = (crownPowerOfWind_ > 1e-07) ? (crownPowerOfFire_ / crownPowerOfWind_) : 0.0;
    return crownFirePowerRatio_;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the critical crown fire spread rate to achieve active
*  crowning given the canopy crown bulk density
*
*  \return Critical crown fire spread rate (ft/min).
*/
double Crown::calculateCrownCriticalFireSpreadRate()
{
    double canopyBulkDensity = crownInputs_->getCanopyBulkDensity();
    const double LBS_PER_CUBIC_FOOT_TO_KG_PER_CUBIC_METER = 16.0185;
    // Convert to Kg/m3
    double convertedBulkDensity = LBS_PER_CUBIC_FOOT_TO_KG_PER_CUBIC_METER * canopyBulkDensity;
    crownCriticalFireSpreadRate_ = (convertedBulkDensity < 1e-07) ? 0.00 : (3.0 / convertedBulkDensity);
    const double METERS_PER_MIN_TO_FEET_PER_MIN = 3.28084;
    // Convert to ft/min
    crownCriticalFireSpreadRate_ *= METERS_PER_MIN_TO_FEET_PER_MIN;
    return crownCriticalFireSpreadRate_;
}

//------------------------------------------------------------------------------
/*! \brief Calculates the crown fire active ratio given the crown fire spread 
*  rate and the critical crown fire spread rate.
*
*  \return Crown fire active ratio.
*/
double Crown::calculateCrownFireActiveRatio()
{
    crownFireActiveRatio_ = (crownCriticalFireSpreadRate_ < 1e-07)
        ? (0.00)
        : (crownFireSpreadRate_ / crownCriticalFireSpreadRate_);
    return crownFireActiveRatio_;
}

double Crown::calculateWindSpeedAtTwentyFeet()
{
    windSpeedAtTwentyFeet_ = -1; // If negative 1 is returned, there is an error
    WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode;
    windHeightInputMode = surfaceInputs_->getWindHeightInputMode();

    if (windHeightInputMode == WindHeightInputMode::TWENTY_FOOT)
    {
        windSpeedAtTwentyFeet_ = surfaceInputs_->getWindSpeed();
    }
    else if (windHeightInputMode == WindHeightInputMode::TEN_METER)
    {
        WindSpeedUtility windSpeedUtility;
        double windSpeedAtTenMeters = surfaceInputs_->getWindSpeed();
        windSpeedAtTwentyFeet_ = windSpeedUtility.windSpeedAtTwentyFeetFromTenMeter(windSpeedAtTenMeters);
    }
    return windSpeedAtTwentyFeet_;
}
