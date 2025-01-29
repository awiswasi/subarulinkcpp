#pragma once
#ifndef SUBARULINK_API_CONSTANTS_HPP
#define SUBARULINK_API_CONSTANTS_HPP

#include <string>
#include <map>
#include <vector>

namespace subarulink {
  namespace api {

    const std::string API_VERSION = "/g2v30";

    const std::map<std::string, std::string> API_SERVER = {
        {"USA", "mobileapi.prod.subarucs.com"},
        {"CAN", "mobileapi.ca.prod.subarucs.com"}
    };

    const std::map<std::string, std::string> API_MOBILE_APP = {
        {"USA", "com.subaru.telematics.app.remote"},
        {"CAN", "ca.subaru.telematics.remote"}
    };

    // selectVehicle.json keys
    const std::string API_VEHICLE_ATTRIBUTES = "attributes";
    const std::string API_VEHICLE_ID = "id";
    const std::string API_VEHICLE_MODEL_NAME = "modelName";
    const std::string API_VEHICLE_MODEL_YEAR = "modelYear";
    const std::string API_VEHICLE_NAME = "nickname";
    const std::string API_VEHICLE_API_GEN = "api_gen";
    const std::string API_VEHICLE_FEATURES = "features";
    const std::string API_VEHICLE_SUBSCRIPTION_FEATURES = "subscriptionFeatures";
    const std::string API_VEHICLE_SUBSCRIPTION_STATUS = "subscriptionStatus";

// vehicleHealth.json fields
    const std::string API_HEALTH_TROUBLE = "isTrouble";
    const std::string API_HEALTH_ONDATES = "onDates";
    const std::string API_HEALTH_FEATURE = "featureCode";

// condition/execute.json fields
    const std::string API_DOOR_BOOT_POSITION = "doorBootPosition";
    const std::string API_DOOR_ENGINE_HOOD_POSITION = "doorEngineHoodPosition";
    const std::string API_DOOR_FRONT_LEFT_POSITION = "doorFrontLeftPosition";
    const std::string API_DOOR_FRONT_RIGHT_POSITION = "doorFrontRightPosition";
    const std::string API_DOOR_REAR_LEFT_POSITION = "doorRearLeftPosition";
    const std::string API_DOOR_REAR_RIGHT_POSITION = "doorRearRightPosition";
    const std::string API_EV_CHARGER_STATE_TYPE = "evChargerStateType";
    const std::string API_EV_DISTANCE_TO_EMPTY = "evDistanceToEmpty";
    const std::string API_EV_IS_PLUGGED_IN = "evIsPluggedIn";
    const std::string API_EV_STATE_OF_CHARGE_MODE = "evStateOfChargeMode";
    const std::string API_EV_STATE_OF_CHARGE_PERCENT = "evStateOfChargePercent";
    const std::string API_EV_TIME_TO_FULLY_CHARGED = "evTimeToFullyCharged";
    const std::string API_EV_TIME_TO_FULLY_CHARGED_UTC = "evTimeToFullyChargedUTC";
    const std::string API_REMAINING_FUEL_PERCENT = "remainingFuelPercent";
    const std::string API_LAST_UPDATED_DATE = "lastUpdatedTime";
    const std::string API_LOCK_BOOT_STATUS = "doorBootLockStatus";
    const std::string API_LOCK_FRONT_LEFT_STATUS = "doorFrontLeftLockStatus";
    const std::string API_LOCK_FRONT_RIGHT_STATUS = "doorFrontRightLockStatus";
    const std::string API_LOCK_REAR_LEFT_STATUS = "doorRearLeftLockStatus";
    const std::string API_LOCK_REAR_RIGHT_STATUS = "doorRearRightStatus";
    const std::string API_WINDOW_FRONT_LEFT_STATUS = "windowFrontLeftStatus";
    const std::string API_WINDOW_FRONT_RIGHT_STATUS = "windowFrontRightStatus";
    const std::string API_WINDOW_REAR_LEFT_STATUS = "windowRearLeftStatus";
    const std::string API_WINDOW_REAR_RIGHT_STATUS = "windowRearRightStatus";
    const std::string API_WINDOW_SUNROOF_STATUS = "windowSunroofStatus";

// Additional error codes
    const std::string API_ERROR_NO_VEHICLES = "noVehiclesOnAccount";
    const std::string API_ERROR_NO_ACCOUNT = "accountNotFound";
    const std::string API_ERROR_TOO_MANY_ATTEMPTS = "tooManyAttempts";
    const std::string API_ERROR_VEHICLE_NOT_IN_ACCOUNT = "vehicleNotInAccount";

// Timestamp formats
    const std::string API_TIMESTAMP_FMT = "%Y-%m-%dT%H:%M:%S.%f%z";     // "2020-04-25T23:35:55.000+0000"
    const std::string API_TIMESTAMP_FMT_OLD = "%Y-%m-%dT%H:%M:%S%z";    // "2020-04-25T23:35:55+0000"
    const std::string API_VS_TIMESTAMP_FMT = "%Y-%m-%dT%H:%M%z";        // "2020-04-25T23:35+0000"
    const std::string API_POSITION_TIMESTAMP_FMT = "%Y-%m-%dT%H:%M:%SZ"; // "2020-04-25T23:35:55Z"

// Tire pressure prefix constants
    const std::string API_FEATURE_FRONT_TIRE_RECOMMENDED_PRESSURE_PREFIX = "TIF_";
    const std::string API_FEATURE_REAR_TIRE_RECOMMENDED_PRESSURE_PREFIX = "TIR_";

// API Endpoints
    const std::string API_LOGIN = "/login.json";
    const std::string API_2FA_CONTACT = "/twoStepAuthContacts.json";
    const std::string API_2FA_SEND_VERIFICATION = "/twoStepAuthSendVerification.json";
    const std::string API_2FA_AUTH_VERIFY = "/twoStepAuthVerify.json";
    const std::string API_REFRESH_VEHICLES = "/refreshVehicles.json";
    const std::string API_SELECT_VEHICLE = "/selectVehicle.json";
    const std::string API_VALIDATE_SESSION = "/validateSession.json";
    const std::string API_VEHICLE_STATUS = "/vehicleStatus.json";
    const std::string API_AUTHORIZE_DEVICE = "/authenticateDevice.json";
    const std::string API_NAME_DEVICE = "/nameThisDevice.json";
    const std::string API_VEHICLE_HEALTH = "/vehicleHealth.json";

// API Templates (replace api_gen with g1 or g2)
    const std::string API_LOCK = "/service/api_gen/lock/execute.json";
    const std::string API_LOCK_CANCEL = "/service/api_gen/lock/cancel.json";
    const std::string API_UNLOCK = "/service/api_gen/unlock/execute.json";
    const std::string API_UNLOCK_CANCEL = "/service/api_gen/unlock/cancel.json";
    const std::string API_HORN_LIGHTS = "/service/api_gen/hornLights/execute.json";
    const std::string API_HORN_LIGHTS_CANCEL = "/service/api_gen/hornLights/cancel.json";
    const std::string API_HORN_LIGHTS_STOP = "/service/api_gen/hornLights/stop.json";
    const std::string API_LIGHTS = "/service/api_gen/lightsOnly/execute.json";
    const std::string API_LIGHTS_CANCEL = "/service/api_gen/lightsOnly/cancel.json";
    const std::string API_LIGHTS_STOP = "/service/api_gen/lightsOnly/stop.json";
    const std::string API_CONDITION = "/service/api_gen/condition/execute.json";
    const std::string API_LOCATE = "/service/api_gen/locate/execute.json";
    const std::string API_REMOTE_SVC_STATUS = "/service/api_gen/remoteService/status.json";

// Generation-specific APIs
    const std::string API_G1_LOCATE_UPDATE = "/service/g1/vehicleLocate/execute.json";
    const std::string API_G1_LOCATE_STATUS = "/service/g1/vehicleLocate/status.json";
    const std::string API_G2_LOCATE_UPDATE = "/service/g2/vehicleStatus/execute.json";
    const std::string API_G2_LOCATE_STATUS = "/service/g2/vehicleStatus/locationStatus.json";
    const std::string API_G1_HORN_LIGHTS_STATUS = "/service/g1/hornLights/status.json";

// G2-Only APIs
    const std::string API_G2_SEND_POI = "/service/g2/sendPoi/execute.json";
    const std::string API_G2_SPEEDFENCE = "/service/g2/speedFence/execute.json";
    const std::string API_G2_GEOFENCE = "/service/g2/geoFence/execute.json";
    const std::string API_G2_CURFEW = "/service/g2/curfew/execute.json";
    const std::string API_G2_REMOTE_ENGINE_START = "/service/g2/engineStart/execute.json";
    const std::string API_G2_REMOTE_ENGINE_START_CANCEL = "/service/g2/engineStart/cancel.json";
    const std::string API_G2_REMOTE_ENGINE_STOP = "/service/g2/engineStop/execute.json";
    const std::string API_G2_FETCH_RES_QUICK_START_SETTINGS = "/service/g2/remoteEngineQuickStartSettings/fetch.json";
    const std::string API_G2_FETCH_RES_USER_PRESETS = "/service/g2/remoteEngineStartSettings/fetch.json";
    const std::string API_G2_FETCH_RES_SUBARU_PRESETS = "/service/g2/climatePresetSettings/fetch.json";
    const std::string API_G2_SAVE_RES_SETTINGS = "/service/g2/remoteEngineStartSettings/save.json";
    const std::string API_G2_SAVE_RES_QUICK_START_SETTINGS = "/service/g2/remoteEngineQuickStartSettings/save.json";

// EV-Only APIs
    const std::string API_EV_CHARGE_NOW = "/service/g2/phevChargeNow/execute.json";
    const std::string API_EV_FETCH_CHARGE_SETTINGS = "/service/g2/phevGetTimerSettings/execute.json";
    const std::string API_EV_SAVE_CHARGE_SETTINGS = "/service/g2/phevSendTimerSetting/execute.json";
    const std::string API_EV_DELETE_CHARGE_SCHEDULE = "/service/g2/phevDeleteTimerSetting/execute.json";

// API Field Names
    const std::string API_AVG_FUEL_CONSUMPTION = "avgFuelConsumptionMpg";
    const std::string API_DIST_TO_EMPTY = "distanceToEmptyFuelMiles10s";
    const std::string API_TIMESTAMP = "eventDateStr";
    const std::string API_LATITUDE = "latitude";
    const std::string API_LONGITUDE = "longitude";
    const std::string API_ODOMETER = "odometerValue";
    const std::string API_VEHICLE_STATE = "vehicleStateType";
    const std::string API_TIRE_PRESSURE_FL = "tirePressureFrontLeftPsi";
    const std::string API_TIRE_PRESSURE_FR = "tirePressureFrontRightPsi";
    const std::string API_TIRE_PRESSURE_RL = "tirePressureRearLeftPsi";
    const std::string API_TIRE_PRESSURE_RR = "tirePressureRearRightPsi";

// Vehicle Features
    const std::string API_FEATURE_PHEV = "PHEV";
    const std::string API_FEATURE_REMOTE_START = "RES";
    const std::string API_FEATURE_REMOTE = "REMOTE";
    const std::string API_FEATURE_SAFETY = "SAFETY";
    const std::string API_FEATURE_ACTIVE = "ACTIVE";
    const std::string API_FEATURE_G1_TELEMATICS = "g1";
    const std::string API_FEATURE_G2_TELEMATICS = "g2";
    const std::string API_FEATURE_G3_TELEMATICS = "g3";
    const std::string API_FEATURE_TPMS = "TPMS_MIL";
    const std::string API_FEATURE_LOCK_STATUS = "DOOR_LU_STAT";
    const std::string API_FEATURE_2_DOOR = "DOORF";

    const std::vector<std::string> API_FEATURE_MOONROOF_LIST = {
        "PANPM-DG2G", "PANPM-TUIRWAOC", "PANMRF_ES", "PANPM-FGTU",
        "PANPM-INTRETR", "PANPM-PWRSHD", "PANPW-RGF", "MOONSTAT"
    };

    const std::vector<std::string> API_FEATURE_WINDOWS_LIST = {"PWAAADWWAP", "WDWSTAT"};

// Error Codes
    const std::string API_ERROR_SOA_403 = "403-soa-unableToParseResponseBody";
    const std::string API_ERROR_INVALID_CREDENTIALS = "InvalidCredentials";
    const std::string API_ERROR_SERVICE_ALREADY_STARTED = "ServiceAlreadyStarted";
    const std::string API_ERROR_INVALID_ACCOUNT = "invalidAccount";
    const std::string API_ERROR_PASSWORD_WARNING = "passwordWarning";
    const std::string API_ERROR_ACCOUNT_LOCKED = "accountLocked";
    const std::string API_ERROR_INVALID_TOKEN = "InvalidToken";
    const std::string API_ERROR_VEHICLE_SETUP = "VEHICLESETUPERROR";

// G1 Error Codes
    const std::string API_ERROR_G1_NO_SUBSCRIPTION = "SXM40004";
    const std::string API_ERROR_G1_STOLEN_VEHICLE = "SXM40005";
    const std::string API_ERROR_G1_INVALID_PIN = "SXM40006";
    const std::string API_ERROR_G1_SERVICE_ALREADY_STARTED = "SXM40009";
    const std::string API_ERROR_G1_PIN_LOCKED = "SXM40017";

// Timing Constants
    const int API_MAX_SESSION_AGE_MINS = 240;
    const std::string API_SERVICE_REQ_ID = "serviceRequestId";

  } // namespace api
} // namespace subarulink

#endif // SUBARULINK_API_CONSTANTS_HPP