/******************************************************************************
*
* Project:  CodeBlocks
* Purpose:  Class for handling the inputs required for surface fire behavior 
*           in the Rothermel Model
* Author:   William Chatham <wchatham@fs.fed.us>
*
******************************************************************************
*
* THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
* MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
* IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
* OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
* PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
* LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
* PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
* RELIABILITY, OR ANY OTHER CHARACTERISTIC.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
******************************************************************************/

#include "surfaceInputs.h"

#include <cmath>

// Default Ctor
SurfaceInputs::SurfaceInputs()
{
    initializeMembers();
}

SurfaceInputs & SurfaceInputs::operator=(const SurfaceInputs & rhs)
{
    if (this != &rhs)
    {
        memberwiseCopyAssignment(rhs);
    }
    return *this;
}

void SurfaceInputs::initializeMembers()
{
    airTemperature_ = -500.0; // impossible temperature below absolute zero, indicates hasn't been set by user

    fuelModelNumber_ = 0;
    secondFuelModelNumber_ = 0;
    moistureOneHour_ = 0.0;
    moistureTenHour_ = 0.0;
    moistureHundredHour_ = 0.0;
    moistureDeadAggregate_ = -1.0;
    moistureLiveHerbaceous_ = 0.0;
    moistureLiveWoody_ = 0.0;
    moistureLiveAggregate_ = -1.0;
    slope_ = 0.0;
    aspect_ = 0.0;
    windSpeed_ = 0.0;
    windDirection_ = 0.0;

    moistureInputMode_ = MoistureInputMode::BySizeClass;

    isCalculatingScorchHeight_ = false;
    isUsingTwoFuelModels_ = false;
    isUsingPalmettoGallberry_ = false;
    isUsingWesternAspen_ = false;
    isUsingChaparral_ = false;

    windAndSpreadOrientationMode_ = WindAndSpreadOrientationMode::RelativeToUpslope;
    windHeightInputMode_ = WindHeightInputMode::DirectMidflame;
    twoFuelModelsMethod_ = TwoFuelModelsMethod::NoMethod;
    windAdjustmentFactorCalculationMethod_ = WindAdjustmentFactorCalculationMethod::UseCrownRatio;
    surfaceFireSpreadDirectionMode_ = SurfaceFireSpreadDirectionMode::FromIgnitionPoint;

    firstFuelModelCoverage_ = 0.0;

    ageOfRough_ = 0.0;
    heightOfUnderstory_ = 0.0;
    palmettoCoverage_ = 0.0;
    overstoryBasalArea_ = 0.0;

    canopyCover_ = 0.0;
    canopyHeight_ = 0.0;
    crownRatio_ = 0.0;

    aspenFuelModelNumber_ = -1;
    aspenCuringLevel_ = 0.0;
    aspenFireSeverity_ = AspenFireSeverity::Low;
    dbh_ = 0.0;

    elapsedTime_ = TimeUnits::toBaseUnits(1, TimeUnits::Hours);

    userProvidedWindAdjustmentFactor_ = -1.0;

    moistureScenarios = nullptr;
    currentMoistureScenarioName_ = "";
    currentMoistureScenarioIndex_ = -1;
    moistureValuesBySizeClass_ = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
}

void SurfaceInputs::updateSurfaceInputs(int fuelModelNumber, double moistureOneHour, double moistureTenHour,
    double moistureHundredHour, double moistureLiveHerbaceous, double moistureLiveWoody, 
    MoistureUnits::MoistureUnitsEnum moistureUnits, double windSpeed, SpeedUnits::SpeedUnitsEnum windSpeedUnits, 
    WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode, double windDirection, 
    WindAndSpreadOrientationMode::WindAndSpreadOrientationModeEnum windAndSpreadOrientationMode, double slope,
    SlopeUnits::SlopeUnitsEnum slopeUnits, double aspect, double canopyCover, CoverUnits::CoverUnitsEnum coverUnits,
    double canopyHeight, LengthUnits::LengthUnitsEnum canopyHeightUnits, double crownRatio)
{
    setSlope(slope, slopeUnits);
    aspect_ = aspect;

    setFuelModelNumber(fuelModelNumber);
   
    setMoistureOneHour(moistureOneHour, moistureUnits);
    setMoistureTenHour(moistureTenHour, moistureUnits);
    setMoistureHundredHour(moistureHundredHour, moistureUnits);
    setMoistureLiveHerbaceous(moistureLiveHerbaceous, moistureUnits);
    setMoistureLiveWoody(moistureLiveWoody, moistureUnits);

    setWindSpeed(windSpeed, windSpeedUnits, windHeightInputMode);
    setWindHeightInputMode(windHeightInputMode);

    if (windDirection < 0.0)
    {
        windDirection += 360.0;
    }
    while (windDirection >= 360.0)
    {
        windDirection -= 360.0;
    }

    setWindDirection(windDirection);
    setWindAndSpreadOrientationMode(windAndSpreadOrientationMode);
    isUsingTwoFuelModels_ = false;
    setTwoFuelModelsMethod(TwoFuelModelsMethod::NoMethod);

    setCanopyCover(canopyCover, coverUnits);
    setCanopyHeight(canopyHeight, canopyHeightUnits);
    setCrownRatio(crownRatio);
}

void  SurfaceInputs::updateSurfaceInputsForTwoFuelModels(int firstfuelModelNumber, int secondFuelModelNumber,
    double moistureOneHour, double moistureTenHour, double moistureHundredHour, double moistureLiveHerbaceous,
    double moistureLiveWoody, MoistureUnits::MoistureUnitsEnum moistureUnits, double windSpeed, SpeedUnits::SpeedUnitsEnum windSpeedUnits, 
    WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode, double windDirection,
    WindAndSpreadOrientationMode::WindAndSpreadOrientationModeEnum windAndSpreadOrientationMode, double firstFuelModelCoverage, 
    CoverUnits::CoverUnitsEnum firstFuelModelCoverageUnits, TwoFuelModelsMethod::TwoFuelModelsMethodEnum twoFuelModelsMethod, double slope, SlopeUnits::SlopeUnitsEnum slopeUnits,
    double aspect, double canopyCover, CoverUnits::CoverUnitsEnum canopyCoverUnits, double canopyHeight, LengthUnits::LengthUnitsEnum canopyHeightUnits, double crownRatio)
{
    int fuelModelNumber = firstfuelModelNumber;
    updateSurfaceInputs(fuelModelNumber, moistureOneHour, moistureTenHour, moistureHundredHour, moistureLiveHerbaceous, 
        moistureLiveWoody, moistureUnits, windSpeed, windSpeedUnits, windHeightInputMode, windDirection, windAndSpreadOrientationMode,
        slope, slopeUnits,
        aspect, canopyCover, canopyCoverUnits, canopyHeight, canopyHeightUnits, crownRatio);
    setSecondFuelModelNumber(secondFuelModelNumber);
    setTwoFuelModelsFirstFuelModelCoverage(firstFuelModelCoverage, firstFuelModelCoverageUnits);
    isUsingTwoFuelModels_ = true;
    setTwoFuelModelsMethod(twoFuelModelsMethod);
}

void  SurfaceInputs::updateSurfaceInputsForPalmettoGallbery(double moistureOneHour, double moistureTenHour,
    double moistureHundredHour, double moistureLiveHerbaceous, double moistureLiveWoody, MoistureUnits::MoistureUnitsEnum moistureUnits, 
    double windSpeed, SpeedUnits::SpeedUnitsEnum windSpeedUnits, WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode, 
    double windDirection, WindAndSpreadOrientationMode::WindAndSpreadOrientationModeEnum windAndSpreadOrientationMode,
    double ageOfRough, double heightOfUnderstory, double palmettoCoverage, double overstoryBasalArea, BasalAreaUnits::BasalAreaUnitsEnum basalAreaUnits,
    double slope, SlopeUnits::SlopeUnitsEnum slopeUnits, double aspect, double canopyCover, CoverUnits::CoverUnitsEnum coverUnits, double canopyHeight,
    LengthUnits::LengthUnitsEnum canopyHeightUnits, double crownRatio)
{
    updateSurfaceInputs(0, moistureOneHour, moistureTenHour, moistureHundredHour, moistureLiveHerbaceous,
        moistureLiveWoody, moistureUnits, windSpeed, windSpeedUnits, windHeightInputMode, windDirection,
        windAndSpreadOrientationMode, slope, slopeUnits, aspect, canopyCover, coverUnits, canopyHeight, canopyHeightUnits,
        crownRatio);

    setPalmettoGallberryAgeOfRough(ageOfRough);
    setPalmettoGallberryHeightOfUnderstory(heightOfUnderstory, canopyHeightUnits);
    setPalmettoGallberryPalmettoCoverage(palmettoCoverage, coverUnits);
    setPalmettoGallberryOverstoryBasalArea(overstoryBasalArea, basalAreaUnits);
}

void SurfaceInputs::updateSurfaceInputsForWesternAspen(int aspenFuelModelNumber, double aspenCuringLevel, CuringLevelUnits::CuringLevelEnum curingLevelUnits,
    AspenFireSeverity::AspenFireSeverityEnum aspenFireSeverity, double dbh, LengthUnits::LengthUnitsEnum dbhUnits, double moistureOneHour, double moistureTenHour,
    double moistureHundredHour, double moistureLiveHerbaceous, double moistureLiveWoody, MoistureUnits::MoistureUnitsEnum moistureUnits, 
    double windSpeed, SpeedUnits::SpeedUnitsEnum windSpeedUnits, WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode, 
    double windDirection, WindAndSpreadOrientationMode::WindAndSpreadOrientationModeEnum windAndSpreadOrientationMode,
    double slope, SlopeUnits::SlopeUnitsEnum slopeUnits, double aspect, double canopyCover, CoverUnits::CoverUnitsEnum coverUnits, double canopyHeight,
    LengthUnits::LengthUnitsEnum canopyHeightUnits, double crownRatio)
{
    updateSurfaceInputs(0, moistureOneHour, moistureTenHour, moistureHundredHour, moistureLiveHerbaceous,
        moistureLiveWoody, moistureUnits, windSpeed, windSpeedUnits, windHeightInputMode, windDirection,
        windAndSpreadOrientationMode, slope, slopeUnits, aspect, canopyCover, coverUnits, canopyHeight, canopyHeightUnits,
        crownRatio);

    setAspenFuelModelNumber(aspenFuelModelNumber);
    setAspenCuringLevel(aspenCuringLevel, curingLevelUnits);
    setAspenFireSeverity(aspenFireSeverity);
    setAspenDBH(dbh, dbhUnits);
}

void SurfaceInputs::setAspenFuelModelNumber(int aspenFuelModelNumber)
{
    aspenFuelModelNumber_ = aspenFuelModelNumber;
}

void SurfaceInputs::setAspenCuringLevel(double aspenCuringLevel, CuringLevelUnits::CuringLevelEnum curingLevelUnits)
{
    aspenCuringLevel_ = CuringLevelUnits::toBaseUnits(aspenCuringLevel, curingLevelUnits);
}

void SurfaceInputs::setAspenDBH(double dbh, LengthUnits::LengthUnitsEnum dbhUnits)
{
    dbh_ = LengthUnits::toBaseUnits(dbh, dbhUnits);
}

void SurfaceInputs::setAspenFireSeverity(AspenFireSeverity::AspenFireSeverityEnum aspenFireSeverity)
{
    aspenFireSeverity_ = aspenFireSeverity;
}

void SurfaceInputs::setIsUsingWesternAspen(bool isUsingWesternAspen)
{
    isUsingWesternAspen_ = isUsingWesternAspen;
    if (isUsingWesternAspen_)
    {
        // Special case fuel models are mutually exclusive
        isUsingChaparral_ = false;
        isUsingPalmettoGallberry_ = false;
    }
}

void SurfaceInputs::setCanopyCover(double canopyCover, CoverUnits::CoverUnitsEnum coverUnits)
{
    canopyCover_ = CoverUnits::toBaseUnits(canopyCover, coverUnits);
}

void SurfaceInputs::setCanopyHeight(double canopyHeight, LengthUnits::LengthUnitsEnum canopyHeightUnits)
{
    canopyHeight_ = LengthUnits::toBaseUnits(canopyHeight, canopyHeightUnits);
}

void SurfaceInputs::setCrownRatio(double crownRatio)
{
    crownRatio_ = crownRatio;
}

void SurfaceInputs::setWindAndSpreadOrientationMode(WindAndSpreadOrientationMode::WindAndSpreadOrientationModeEnum windAndSpreadOrientationMode)
{
    windAndSpreadOrientationMode_ = windAndSpreadOrientationMode;
}

void SurfaceInputs::setWindHeightInputMode(WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode)
{
    windHeightInputMode_ = windHeightInputMode;
}

void SurfaceInputs::setFuelModelNumber(int fuelModelNumber)
{
    fuelModelNumber_ = fuelModelNumber;
}

void SurfaceInputs::setMoistureOneHour(double moistureOneHour, MoistureUnits::MoistureUnitsEnum moistureUnits)
{
    moistureOneHour_ = MoistureUnits::toBaseUnits(moistureOneHour, moistureUnits);
    updateMoisturesBasedOnInputMode();
}

void SurfaceInputs::setMoistureTenHour(double moistureTenHour, MoistureUnits::MoistureUnitsEnum moistureUnits)
{
    moistureTenHour_ = MoistureUnits::toBaseUnits(moistureTenHour, moistureUnits);
    updateMoisturesBasedOnInputMode();
}

void SurfaceInputs::setMoistureHundredHour(double moistureHundredHour, MoistureUnits::MoistureUnitsEnum moistureUnits)
{
    moistureHundredHour_ = MoistureUnits::toBaseUnits(moistureHundredHour, moistureUnits);
    updateMoisturesBasedOnInputMode();
}

void SurfaceInputs::setMoistureLiveHerbaceous(double moistureLiveHerbaceous, MoistureUnits::MoistureUnitsEnum moistureUnits)
{
    moistureLiveHerbaceous_ = MoistureUnits::toBaseUnits(moistureLiveHerbaceous, moistureUnits);
    updateMoisturesBasedOnInputMode();
}

void SurfaceInputs::setMoistureLiveWoody(double moistureLiveWoody, MoistureUnits::MoistureUnitsEnum moistureUnits)
{
    moistureLiveWoody_ = MoistureUnits::toBaseUnits(moistureLiveWoody, moistureUnits);
    updateMoisturesBasedOnInputMode();
}

void SurfaceInputs::setMoistureDeadAggregate(double moistureDeadAggregate, MoistureUnits::MoistureUnitsEnum moistureUnits)
{
    moistureDeadAggregate_ = MoistureUnits::toBaseUnits(moistureDeadAggregate, moistureUnits);
    updateMoisturesBasedOnInputMode();
}

void SurfaceInputs::setMoistureLiveAggregate(double moistureLiveAggregate, MoistureUnits::MoistureUnitsEnum moistureUnits)
{
    moistureLiveAggregate_ = MoistureUnits::toBaseUnits(moistureLiveAggregate, moistureUnits);
    updateMoisturesBasedOnInputMode();
}

bool SurfaceInputs::setMoistureScenarioByName(std::string moistureScenarioName)
{
    bool isMoistureScenarioDefined = moistureScenarios->getIsMoistureScenarioDefinedByName(moistureScenarioName);
    currentMoistureScenarioName_ = "";
    if(isMoistureScenarioDefined)
    {
        currentMoistureScenarioName_ = moistureScenarioName;
        currentMoistureScenarioIndex_ = moistureScenarios->getMoistureScenarioIndexByName(moistureScenarioName);
        updateMoisturesBasedOnInputMode();
    }
    return isMoistureScenarioDefined;
}

bool SurfaceInputs::setMoistureScenarioByIndex(int moistureScenarioIndex)
{
    bool isMoistureScenarioDefined = moistureScenarios->getIsMoistureScenarioDefinedByIndex(moistureScenarioIndex);
    currentMoistureScenarioIndex_ = -1;
    if(isMoistureScenarioDefined)
    {
        currentMoistureScenarioIndex_ = moistureScenarioIndex;
        currentMoistureScenarioName_ = moistureScenarios->getMoistureScenarioNameByIndex(moistureScenarioIndex);
        updateMoisturesBasedOnInputMode();
    }
    return isMoistureScenarioDefined;
}

void SurfaceInputs::setMoistureInputMode(MoistureInputMode::MoistureInputModeEnum moistureInputMode)
{
    moistureInputMode_ = moistureInputMode;
    updateMoisturesBasedOnInputMode();
}

void SurfaceInputs::setSlope(double slope, SlopeUnits::SlopeUnitsEnum slopeUnits)
{
    slope_ = SlopeUnits::toBaseUnits(slope, slopeUnits);
}

void SurfaceInputs::setAspect(double aspect)
{
    aspect_ = aspect;
}

void SurfaceInputs::setTwoFuelModelsMethod(TwoFuelModelsMethod::TwoFuelModelsMethodEnum  twoFuelModelsMethod)
{
    twoFuelModelsMethod_ = twoFuelModelsMethod;
}

void SurfaceInputs::setTwoFuelModelsFirstFuelModelCoverage(double firstFuelModelCoverage, CoverUnits::CoverUnitsEnum coverUnits)
{
    firstFuelModelCoverage_ = CoverUnits::toBaseUnits(firstFuelModelCoverage, coverUnits);
}

void  SurfaceInputs::setWindSpeed(double windSpeed, SpeedUnits::SpeedUnitsEnum windSpeedUnits, WindHeightInputMode::WindHeightInputModeEnum windHeightInputMode)
{
    windHeightInputMode_ = windHeightInputMode;
    windSpeed_ = SpeedUnits::toBaseUnits(windSpeed, windSpeedUnits);
}

void  SurfaceInputs::setWindDirection(double windDirection)
{
    windDirection_ = windDirection;
}

void  SurfaceInputs::setFirstFuelModelNumber(int firstFuelModelNumber)
{
    fuelModelNumber_ = firstFuelModelNumber;
}

int  SurfaceInputs::getFirstFuelModelNumber() const
{
    return fuelModelNumber_;
}

int  SurfaceInputs::getSecondFuelModelNumber() const
{
    return secondFuelModelNumber_;
}

void  SurfaceInputs::setSecondFuelModelNumber(int secondFuelModelNumber)
{
    secondFuelModelNumber_ = secondFuelModelNumber;
}

int SurfaceInputs::getFuelModelNumber() const
{
    return fuelModelNumber_;
}

double SurfaceInputs::getSlope(SlopeUnits::SlopeUnitsEnum slopeUnits) const
{
    return SlopeUnits::fromBaseUnits(slope_, slopeUnits);
}

double SurfaceInputs::getAspect() const
{
    return aspect_;
}

double SurfaceInputs::getFirstFuelModelCoverage() const
{
    return firstFuelModelCoverage_;
}

TwoFuelModelsMethod::TwoFuelModelsMethodEnum SurfaceInputs::getTwoFuelModelsMethod() const
{
    return twoFuelModelsMethod_;
}

bool SurfaceInputs::isUsingTwoFuelModels() const
{
    return isUsingTwoFuelModels_;
}

bool SurfaceInputs::getIsUsingPalmettoGallberry() const
{
    return isUsingPalmettoGallberry_;
}

WindHeightInputMode::WindHeightInputModeEnum SurfaceInputs::getWindHeightInputMode() const
{
    return windHeightInputMode_;
}

WindAndSpreadOrientationMode::WindAndSpreadOrientationModeEnum SurfaceInputs::getWindAndSpreadOrientationMode() const
{
    return windAndSpreadOrientationMode_;
}

double SurfaceInputs::getWindDirection() const
{
    return windDirection_;
}

double SurfaceInputs::getWindSpeed(SpeedUnits::SpeedUnitsEnum windSpeedUnits) const
{
    return SpeedUnits::fromBaseUnits(windSpeed_, windSpeedUnits);
}

double SurfaceInputs::getMoistureOneHour(MoistureUnits::MoistureUnitsEnum moistureUnits) const
{
    return MoistureUnits::fromBaseUnits(moistureValuesBySizeClass_[MoistureClassInput::OneHour], moistureUnits);
}

double SurfaceInputs::getMoistureTenHour(MoistureUnits::MoistureUnitsEnum moistureUnits) const
{
     return MoistureUnits::fromBaseUnits(moistureValuesBySizeClass_[MoistureClassInput::TenHour], moistureUnits);
}

double SurfaceInputs::getMoistureHundredHour(MoistureUnits::MoistureUnitsEnum moistureUnits) const
{
    return MoistureUnits::fromBaseUnits(moistureValuesBySizeClass_[MoistureClassInput::HundredHour], moistureUnits);
}

double SurfaceInputs::getMoistureDeadAggregateValue(MoistureUnits::MoistureUnitsEnum moistureUnits) const
{
    return MoistureUnits::fromBaseUnits(moistureValuesBySizeClass_[MoistureClassInput::DeadAggregate], moistureUnits);
}

double SurfaceInputs::getMoistureLiveHerbaceous(MoistureUnits::MoistureUnitsEnum moistureUnits) const
{
    return MoistureUnits::fromBaseUnits(moistureValuesBySizeClass_[MoistureClassInput::LiveHerbaceous], moistureUnits);
}

double SurfaceInputs::getMoistureLiveWoody(MoistureUnits::MoistureUnitsEnum moistureUnits) const
{
    return MoistureUnits::fromBaseUnits(moistureValuesBySizeClass_[MoistureClassInput::LiveWoody], moistureUnits);
}

double SurfaceInputs::getMoistureLiveAggregateValue(MoistureUnits::MoistureUnitsEnum moistureUnits) const
{
    return MoistureUnits::fromBaseUnits(moistureValuesBySizeClass_[MoistureClassInput::LiveAggregate], moistureUnits);
}

void SurfaceInputs::setPalmettoGallberryAgeOfRough(double ageOfRough)
{
    ageOfRough_ = ageOfRough;
}

double SurfaceInputs::getPalmettoGallberryAgeOfRough() const
{
    return ageOfRough_;
}

void SurfaceInputs::setPalmettoGallberryHeightOfUnderstory(double heightOfUnderstory, LengthUnits::LengthUnitsEnum heightUnits)
{
    heightOfUnderstory_ = LengthUnits::toBaseUnits(heightOfUnderstory, heightUnits);
}

double SurfaceInputs::getPalmettoGallberryHeightOfUnderstory(LengthUnits::LengthUnitsEnum heightUnits) const
{
    return  LengthUnits::fromBaseUnits(heightOfUnderstory_, heightUnits);
}
void SurfaceInputs::setPalmettoGallberryPalmettoCoverage(double palmettoCoverage, CoverUnits::CoverUnitsEnum coverUnits)
{
    palmettoCoverage_ = CoverUnits::toBaseUnits(palmettoCoverage, coverUnits);
}

double SurfaceInputs::getPalmettoGallberryPalmettoCoverage(CoverUnits::CoverUnitsEnum coverUnits) const
{
    return CoverUnits::fromBaseUnits(palmettoCoverage_, coverUnits);
}

void SurfaceInputs::setPalmettoGallberryOverstoryBasalArea(double overstoryBasalArea, BasalAreaUnits::BasalAreaUnitsEnum basalAreaUnits)
{
    overstoryBasalArea_ = BasalAreaUnits::toBaseUnits(overstoryBasalArea, basalAreaUnits);
}

void SurfaceInputs::setIsUsingPalmettoGallberry(bool isUsingPalmettoGallberry)
{
    isUsingPalmettoGallberry_ = isUsingPalmettoGallberry;
    if (isUsingPalmettoGallberry_)
    {
        // Special case fuel models are mutually exclusive
        isUsingChaparral_ = false;
        isUsingWesternAspen_ = false;
    }
}

double SurfaceInputs::getPalmettoGallberryOverstoryBasalArea(BasalAreaUnits::BasalAreaUnitsEnum basalAreaUnits) const
{
    return BasalAreaUnits::fromBaseUnits(overstoryBasalArea_, basalAreaUnits);
}

double SurfaceInputs::getCanopyCover(CoverUnits::CoverUnitsEnum coverUnits) const
{
    return CoverUnits::fromBaseUnits(canopyCover_, coverUnits);
}

double SurfaceInputs::getCanopyHeight(LengthUnits::LengthUnitsEnum canopyHeightUnits) const
{
    return canopyHeight_;
}

double SurfaceInputs::getCrownRatio() const
{
    return crownRatio_;
}

bool SurfaceInputs::getIsUsingWesternAspen() const
{
    return isUsingWesternAspen_;
}

int SurfaceInputs::getAspenFuelModelNumber() const
{
    return aspenFuelModelNumber_;
}

double SurfaceInputs::getAspenCuringLevel(CuringLevelUnits::CuringLevelEnum curingLevelUnits) const
{
    return CuringLevelUnits::fromBaseUnits(aspenCuringLevel_, curingLevelUnits);
}

double SurfaceInputs::getAspenDBH(LengthUnits::LengthUnitsEnum dbhUnits) const
{
    return LengthUnits::fromBaseUnits(dbh_, dbhUnits);
}

AspenFireSeverity::AspenFireSeverityEnum SurfaceInputs::getAspenFireSeverity() const
{
    return aspenFireSeverity_;
}

void SurfaceInputs::setChaparralFuelType(ChaparralFuelType::ChaparralFuelTypeEnum chaparralFuelType)
{
    chaparralFuelType_ = chaparralFuelType;
}

void SurfaceInputs::setChaparralFuelBedDepth(double chaparralFuelBedDepth, LengthUnits::LengthUnitsEnum depthUnits)
{
    chaparralFuelBedDepth_ = LengthUnits::toBaseUnits(chaparralFuelBedDepth, depthUnits);
}

void SurfaceInputs::setChaparralFuelDeadLoadFraction(double chaparralFuelDeadLoadFraction)
{
    chaparralFuelDeadLoadFraction_ = chaparralFuelDeadLoadFraction;
}

void SurfaceInputs::setChaparralTotalFuelLoad(double chaparralTotalFuelLoad, LoadingUnits::LoadingUnitsEnum fuelLoadUnits)
{
    chaparralTotalFuelLoad_ = LoadingUnits::toBaseUnits(chaparralTotalFuelLoad, fuelLoadUnits);
}

void SurfaceInputs::setIsUsingChaparral(bool isUsingChaparral)
{
    isUsingChaparral_ = isUsingChaparral;
    if (isUsingChaparral_)
    {
        // Special case fuel models are mutually exclusive
        isUsingPalmettoGallberry_ = false;
        isUsingWesternAspen_ = false;
    }
}

ChaparralFuelType::ChaparralFuelTypeEnum SurfaceInputs::getChaparralFuelType() const
{
    return chaparralFuelType_;
}

double SurfaceInputs::getChaparralFuelBedDepth(LengthUnits::LengthUnitsEnum depthUnits) const
{
    return LengthUnits::fromBaseUnits(chaparralFuelBedDepth_, depthUnits);
}

double SurfaceInputs::getChaparralFuelDeadLoadFraction() const
{
    return chaparralFuelDeadLoadFraction_;
}

double SurfaceInputs::getChaparralTotalFuelLoad(LoadingUnits::LoadingUnitsEnum fuelLoadUnits) const
{
    return LoadingUnits::fromBaseUnits(chaparralTotalFuelLoad_, fuelLoadUnits);
}

bool SurfaceInputs::getIsUsingChaparral() const
{
    return isUsingChaparral_;
}

void SurfaceInputs::memberwiseCopyAssignment(const SurfaceInputs & rhs)
{
    airTemperature_ = rhs.airTemperature_;

    fuelModelNumber_ = rhs.fuelModelNumber_;
    moistureOneHour_ = rhs.moistureOneHour_;
    moistureTenHour_ = rhs.moistureTenHour_;
    moistureHundredHour_ = rhs.moistureHundredHour_;
    moistureLiveHerbaceous_ = rhs.moistureLiveHerbaceous_;
    moistureLiveWoody_ = rhs.moistureLiveWoody_;
    windSpeed_ = rhs.windSpeed_;
    windDirection_ = rhs.windDirection_;
    slope_ = rhs.slope_;
    aspect_ = rhs.aspect_;

    moistureDeadAggregate_ = rhs.moistureDeadAggregate_;
    moistureLiveAggregate_ = rhs.moistureLiveAggregate_;
    moistureInputMode_ = rhs.moistureInputMode_;

    isCalculatingScorchHeight_ = rhs.isCalculatingScorchHeight_;
    isUsingTwoFuelModels_ = rhs.isUsingTwoFuelModels_;
    secondFuelModelNumber_ = rhs.secondFuelModelNumber_;
    firstFuelModelCoverage_ = rhs.firstFuelModelCoverage_;

    isUsingPalmettoGallberry_ = rhs.isUsingPalmettoGallberry_;
    ageOfRough_ = rhs.ageOfRough_;
    heightOfUnderstory_ = rhs.heightOfUnderstory_;
    palmettoCoverage_ = rhs.palmettoCoverage_;
    overstoryBasalArea_ = rhs.overstoryBasalArea_;

    isUsingWesternAspen_ = rhs.isUsingWesternAspen_;
    aspenFuelModelNumber_ = rhs.aspenFuelModelNumber_;
    aspenCuringLevel_ = rhs.aspenCuringLevel_;
    dbh_ = rhs.dbh_;
    aspenFireSeverity_ = rhs.aspenFireSeverity_;

    isUsingChaparral_ = rhs.isUsingChaparral_;

    elapsedTime_ = rhs.elapsedTime_;

    canopyCover_ = rhs.canopyCover_;
    canopyHeight_ = rhs.canopyHeight_;
    crownRatio_ = rhs.crownRatio_;
    userProvidedWindAdjustmentFactor_ = rhs.userProvidedWindAdjustmentFactor_;

    twoFuelModelsMethod_ = rhs.twoFuelModelsMethod_;
    windHeightInputMode_ = rhs.windHeightInputMode_;
    windAndSpreadOrientationMode_ = rhs.windAndSpreadOrientationMode_;
    windAdjustmentFactorCalculationMethod_ = rhs.windAdjustmentFactorCalculationMethod_;
    surfaceFireSpreadDirectionMode_ = rhs.surfaceFireSpreadDirectionMode_;

    moistureScenarios = rhs.moistureScenarios;
    currentMoistureScenarioName_ = rhs.currentMoistureScenarioName_;
    currentMoistureScenarioIndex_ = rhs.currentMoistureScenarioIndex_;
    moistureValuesBySizeClass_ = rhs.moistureValuesBySizeClass_;
}

void SurfaceInputs::updateMoisturesBasedOnInputMode()
{
    if(moistureInputMode_ == MoistureInputMode::BySizeClass)
    {
        moistureValuesBySizeClass_[MoistureClassInput::OneHour] = moistureOneHour_;
        moistureValuesBySizeClass_[MoistureClassInput::TenHour] = moistureTenHour_;
        moistureValuesBySizeClass_[MoistureClassInput::HundredHour] = moistureHundredHour_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveHerbaceous] = moistureLiveHerbaceous_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveWoody] = moistureLiveWoody_;
        moistureValuesBySizeClass_[MoistureClassInput::DeadAggregate] = -1.0;
        moistureValuesBySizeClass_[MoistureClassInput::LiveAggregate] = -1.0;
    }
    else if(moistureInputMode_ == MoistureInputMode::AllAggregate)
    {
        moistureValuesBySizeClass_[MoistureClassInput::OneHour] = moistureDeadAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::TenHour] = moistureDeadAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::HundredHour] = moistureDeadAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveHerbaceous] = moistureLiveAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveWoody] = moistureLiveAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::DeadAggregate] = moistureDeadAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveAggregate] = moistureLiveAggregate_;
    }
    else if(moistureInputMode_ == MoistureInputMode::DeadAggregateAndLiveSizeClass)
    {
        moistureValuesBySizeClass_[MoistureClassInput::OneHour] = moistureDeadAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::TenHour] = moistureDeadAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::HundredHour] = moistureDeadAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveHerbaceous] = moistureLiveHerbaceous_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveWoody] = moistureLiveWoody_;
        moistureValuesBySizeClass_[MoistureClassInput::DeadAggregate] = moistureDeadAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveAggregate] = -1.0;
    }
    else if(moistureInputMode_ == MoistureInputMode::LiveAggregateAndDeadSizeClass)
    {
        moistureValuesBySizeClass_[MoistureClassInput::OneHour] = moistureOneHour_;
        moistureValuesBySizeClass_[MoistureClassInput::TenHour] = moistureTenHour_;
        moistureValuesBySizeClass_[MoistureClassInput::HundredHour] = moistureHundredHour_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveHerbaceous] = moistureLiveAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::LiveWoody] = moistureLiveAggregate_;
        moistureValuesBySizeClass_[MoistureClassInput::DeadAggregate] = -1.0;
        moistureValuesBySizeClass_[MoistureClassInput::LiveAggregate] = moistureLiveAggregate_;
    }
    else if(moistureInputMode_ == MoistureInputMode::MoistureScenario)
    {
        moistureValuesBySizeClass_[MoistureClassInput::OneHour] = moistureScenarios->getMoistureScenarioOneHourByIndex(currentMoistureScenarioIndex_);
        moistureValuesBySizeClass_[MoistureClassInput::TenHour] = moistureScenarios->getMoistureScenarioTenHourByIndex(currentMoistureScenarioIndex_);
        moistureValuesBySizeClass_[MoistureClassInput::HundredHour] = moistureScenarios->getMoistureScenarioHundredHourByIndex(currentMoistureScenarioIndex_);
        moistureValuesBySizeClass_[MoistureClassInput::LiveHerbaceous] = moistureScenarios->getMoistureScenarioLiveHerbaceousByIndex(currentMoistureScenarioIndex_);
        moistureValuesBySizeClass_[MoistureClassInput::LiveWoody] = moistureScenarios->getMoistureScenarioLiveWoodyByIndex(currentMoistureScenarioIndex_);
        moistureValuesBySizeClass_[MoistureClassInput::DeadAggregate] = -1.0;
        moistureValuesBySizeClass_[MoistureClassInput::LiveAggregate] = -1.0;
    }
}

void SurfaceInputs::setUserProvidedWindAdjustmentFactor(double userProvidedWindAdjustmentFactor)
{
    userProvidedWindAdjustmentFactor_ = userProvidedWindAdjustmentFactor;
}

void SurfaceInputs::setWindAdjustmentFactorCalculationMethod(WindAdjustmentFactorCalculationMethod::WindAdjustmentFactorCalculationMethodEnum windAdjustmentFactorCalculationMethod)
{
    windAdjustmentFactorCalculationMethod_ = windAdjustmentFactorCalculationMethod;
}

void SurfaceInputs::setElapsedTime(double elapsedTime, TimeUnits::TimeUnitsEnum timeUnits)
{
    elapsedTime_ = TimeUnits::toBaseUnits(elapsedTime, timeUnits);
}

void SurfaceInputs::setAirTemperature(double airTemperature, TemperatureUnits::TemperatureUnitsEnum temperatureUnits)
{
    airTemperature_ = TemperatureUnits::toBaseUnits(airTemperature, temperatureUnits);
}

void SurfaceInputs::setIsCalculatingScorchHeight(bool IsCalculatingScorchHeight)
{
    isCalculatingScorchHeight_ = IsCalculatingScorchHeight;
}

double SurfaceInputs::getUserProvidedWindAdjustmentFactor() const
{
    return userProvidedWindAdjustmentFactor_;
}

WindAdjustmentFactorCalculationMethod::WindAdjustmentFactorCalculationMethodEnum SurfaceInputs::getWindAdjustmentFactorCalculationMethod() const
{
    return windAdjustmentFactorCalculationMethod_;
}

double SurfaceInputs::getElapsedTime(TimeUnits::TimeUnitsEnum timeUnits) const
{
    return TimeUnits::fromBaseUnits(elapsedTime_, timeUnits);
}

double SurfaceInputs::getAirTemperature(TemperatureUnits::TemperatureUnitsEnum temperatureUnits) const
{
    return TemperatureUnits::fromBaseUnits(airTemperature_, temperatureUnits);
}

bool SurfaceInputs::getIsCalculatingScorchHeight() const
{
    return isCalculatingScorchHeight_;
}

bool SurfaceInputs::isMoistureClassInputNeeded(MoistureClassInput::MoistureClassInputEnum moistureClass) const
{
    bool isMoistureClassNeeded = false;

    if((moistureClass == MoistureClassInput::OneHour ||
        moistureClass == MoistureClassInput::TenHour ||
        moistureClass == MoistureClassInput::HundredHour) &&
        (moistureInputMode_ == MoistureInputMode::BySizeClass ||
            moistureInputMode_ == MoistureInputMode::LiveAggregateAndDeadSizeClass))
    {
        isMoistureClassNeeded = true;
    }
    else if(moistureClass == MoistureClassInput::DeadAggregate &&
        (moistureInputMode_ == MoistureInputMode::AllAggregate ||
            moistureInputMode_ == MoistureInputMode::DeadAggregateAndLiveSizeClass))
    {
        isMoistureClassNeeded = true;
    }
    else if((moistureClass == MoistureClassInput::LiveHerbaceous ||
        moistureClass == MoistureClassInput::LiveWoody) &&
        (moistureInputMode_ == MoistureInputMode::BySizeClass ||
            moistureInputMode_ == MoistureInputMode::DeadAggregateAndLiveSizeClass))
    {
        isMoistureClassNeeded = true;
    }
    else if(moistureClass == MoistureClassInput::LiveAggregate &&
        (moistureInputMode_ == MoistureInputMode::AllAggregate ||
            moistureInputMode_ == MoistureInputMode::LiveAggregateAndDeadSizeClass))
    {
        isMoistureClassNeeded = true;
    }

    return isMoistureClassNeeded;
}

MoistureInputMode::MoistureInputModeEnum SurfaceInputs::getMoistureInputMode() const
{
    return moistureInputMode_;
}

std::string SurfaceInputs::getCurrentMoistureScenarioName() const
{
    return currentMoistureScenarioName_;
}

int SurfaceInputs::getCurrentMoistureScenarioIndex() const
{
    return currentMoistureScenarioIndex_;
}
