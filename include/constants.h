#pragma once
#ifndef SUBARULINK_CONSTANTS_HPP
#define SUBARULINK_CONSTANTS_HPP

#include <string>
#include <vector>
#include <map>
#include <array>
#include <cstdint>

namespace subarulink {

// Country constants
const std::string COUNTRY_USA = "USA";
const std::string COUNTRY_CAN = "CAN";

// Vehicle data fields
namespace vehicle_fields {
    const std::string AVG_FUEL_CONSUMPTION = "AVG_FUEL_CONSUMPTION";
    const std::string DIST_TO_EMPTY = "DISTANCE_TO_EMPTY_FUEL";
    const std::string DOOR_BOOT_POSITION = "DOOR_BOOT_POSITION";
    const std::string DOOR_ENGINE_HOOD_POSITION = "DOOR_ENGINE_HOOD_POSITION";
    const std::string DOOR_FRONT_LEFT_POSITION = "DOOR_FRONT_LEFT_POSITION";
    const std::string DOOR_FRONT_RIGHT_POSITION = "DOOR_FRONT_RIGHT_POSITION";
    const std::string DOOR_REAR_LEFT_POSITION = "DOOR_REAR_LEFT_POSITION";
    const std::string DOOR_REAR_RIGHT_POSITION = "DOOR_REAR_RIGHT_POSITION";
    // EV specific fields
    const std::string EV_CHARGER_STATE_TYPE = "EV_CHARGER_STATE_TYPE";
    const std::string EV_DISTANCE_TO_EMPTY = "EV_DISTANCE_TO_EMPTY";
    const std::string EV_IS_PLUGGED_IN = "EV_IS_PLUGGED_IN";
    const std::string EV_STATE_OF_CHARGE_MODE = "EV_STATE_OF_CHARGE_MODE";
    const std::string EV_STATE_OF_CHARGE_PERCENT = "EV_STATE_OF_CHARGE_PERCENT";
    const std::string EV_TIME_TO_FULLY_CHARGED = "EV_TIME_TO_FULLY_CHARGED";
    const std::string EV_TIME_TO_FULLY_CHARGED_UTC = "EV_TIME_TO_FULLY_CHARGED_UTC";

    const std::string ODOMETER = "ODOMETER";
    const std::string TIMESTAMP = "TIMESTAMP";
    const std::string TIRE_PRESSURE_FL = "TIRE_PRESSURE_FL";
    const std::string TIRE_PRESSURE_FR = "TIRE_PRESSURE_FR";
    const std::string TIRE_PRESSURE_RL = "TIRE_PRESSURE_RL";
    const std::string TIRE_PRESSURE_RR = "TIRE_PRESSURE_RR";
}

// Vehicle health status
namespace health {
    const std::string FEATURES = "FEATURES";
    const std::string MIL_NAME = "MIL";
    const std::string TROUBLE = "ISTROUBLE";
    const std::string ONDATE = "ONDATE";
    const std::string RECOMMENDED_TIRE_PRESSURE = "RECOMMENDED_TIRE_PRESSURE";
    const std::string RECOMMENDED_TIRE_PRESSURE_FRONT = "FRONT_TIRES";
    const std::string RECOMMENDED_TIRE_PRESSURE_REAR = "REAR_TIRES";
}

// Status enums
enum class DoorStatus {
    OPEN,
    CLOSED
};

enum class LockStatus {
    LOCKED,
    UNLOCKED,
    UNKNOWN
};

enum class WindowStatus {
    OPEN,
    VENTED,
    CLOSED,
    UNKNOWN
};

enum class SunroofStatus {
    OPEN,
    SLIDE_PARTLY_OPEN,
    TILT,
    TILT_PARTLY_OPEN,
    CLOSED
};

enum class VehicleState {
    IGNITION_ON,
    IGNITION_OFF
};

// Climate control constants
namespace climate {

}

namespace climate_control {
  const int TEMP_F_MAX = 85;
  const int TEMP_F_MIN = 60;
  const int TEMP_C_MAX = 30;
  const int TEMP_C_MIN = 15;

  const std::string RUNTIME_10_MIN = "10";
  const std::string RUNTIME_5_MIN = "5";

  struct Mode {
    static const std::string DEFROST;
    static const std::string FEET_DEFROST;
    static const std::string FACE;
    static const std::string FEET;
    static const std::string SPLIT;
    static const std::string AUTO;
  };

  struct HeatSeat {
    static const std::string HIGH;
    static const std::string MEDIUM;
    static const std::string LOW;
    static const std::string OFF;
  };

  struct FanSpeed {
    static const std::string LOW;
    static const std::string MEDIUM;
    static const std::string HIGH;
    static const std::string AUTO;
  };

  // Temperature controls
  const std::string TEMP_F = "climateZoneFrontTemp";
  const std::string TEMP_C = "climateZoneFrontTempCelsius";

  // Runtime
  const std::string RUNTIME = "runTimeMinutes";

  // Mode
  const std::string MODE = "climateZoneFrontAirMode";

  // Seat heating
  const std::string HEAT_SEAT_LEFT = "heatedSeatFrontLeft";
  const std::string HEAT_SEAT_RIGHT = "heatedSeatFrontRight";

  // Defrost
  const std::string REAR_DEFROST = "heatedRearWindowActive";
  const std::string REAR_DEFROST_ON = "true";
  const std::string REAR_DEFROST_OFF = "false";

  // Fan
  const std::string FAN_SPEED = "climateZoneFrontAirVolume";

  // Air circulation
  const std::string RECIRCULATE = "outerAirCirculation";
  const std::string RECIRCULATE_OFF = "outsideAir";
  const std::string RECIRCULATE_ON = "recirculation";

  // AC
  const std::string REAR_AC = "airConditionOn";
  const std::string REAR_AC_ON = "true";
  const std::string REAR_AC_OFF = "false";

  // Preset settings
  const std::string PRESET_NAME = "name";
  const std::string PRESET_INDEX = "index";
  const std::string CAN_EDIT = "canEdit";
  const std::string CAN_EDIT_VALUE = "true";
  const std::string DISABLED = "disabled";
  const std::string DISABLED_VALUE = "false";
  const std::string PRESET_TYPE = "presetType";
  const std::string PRESET_TYPE_USER = "userPreset";
  const std::string START_CONFIGURATION = "startConfiguration";
  const std::string START_CONFIGURATION_EV = "START_CLIMATE_CONTROL_ONLY_ALLOW_KEY_IN_IGNITION";
  const std::string START_CONFIGURATION_RES = "START_ENGINE_ALLOW_KEY_IN_IGNITION";

  // VALID_CLIMATE_OPTIONS equivalent using std::map and std::vector
  const std::map<std::string, std::vector<std::string>> VALID_CLIMATE_OPTIONS = {
      {TEMP_C, []() {
        std::vector<std::string> temps;
        for (int i = TEMP_C_MIN; i <= TEMP_C_MAX; i++) {
          temps.push_back(std::to_string(i));
        }
        return temps;
      }()},
      {TEMP_F, []() {
        std::vector<std::string> temps;
        for (int i = TEMP_F_MIN; i <= TEMP_F_MAX; i++) {
          temps.push_back(std::to_string(i));
        }
        return temps;
      }()},
      {FAN_SPEED, {FanSpeed::AUTO, FanSpeed::LOW, FanSpeed::MEDIUM, FanSpeed::HIGH}},
      {HEAT_SEAT_LEFT, {HeatSeat::OFF, HeatSeat::LOW, HeatSeat::MEDIUM, HeatSeat::HIGH}},
      {HEAT_SEAT_RIGHT, {HeatSeat::OFF, HeatSeat::LOW, HeatSeat::MEDIUM, HeatSeat::HIGH}},
      {MODE, {Mode::DEFROST, Mode::FEET_DEFROST, Mode::FACE, Mode::FEET, Mode::SPLIT, Mode::AUTO}},
      {RECIRCULATE, {RECIRCULATE_OFF, RECIRCULATE_ON}},
      {REAR_AC, {REAR_AC_OFF, REAR_AC_ON}},
      {REAR_DEFROST, {REAR_DEFROST_OFF, REAR_DEFROST_ON}},
      {RUNTIME, {RUNTIME_5_MIN, RUNTIME_10_MIN}},
      {PRESET_TYPE, {PRESET_TYPE_USER}},
      {START_CONFIGURATION, {START_CONFIGURATION_EV, START_CONFIGURATION_RES}},
  };

  // Configuration constants
  const std::map<std::string, std::string> START_CONFIG_CONSTS_EV = {
      {CAN_EDIT, CAN_EDIT_VALUE},
      {DISABLED, DISABLED_VALUE},
      {PRESET_TYPE, PRESET_TYPE_USER},
      {START_CONFIGURATION, START_CONFIGURATION_EV}
  };

  const std::map<std::string, std::string> START_CONFIG_CONSTS_RES = {
      {CAN_EDIT, CAN_EDIT_VALUE},
      {DISABLED, DISABLED_VALUE},
      {PRESET_TYPE, PRESET_TYPE_USER},
      {START_CONFIGURATION, START_CONFIGURATION_RES}
  };
}

// Door constants
namespace door {
    const std::string WHICH_DOOR = "unlockDoorType";
    const std::string ALL_DOORS = "ALL_DOORS_CMD";
    const std::string DRIVERS_DOOR = "FRONT_LEFT_DOOR_CMD";
    const std::string TAILGATE_DOOR = "TAILGATE_DOOR_CMD";
    const std::vector<std::string> VALID_DOORS = {ALL_DOORS, DRIVERS_DOOR, TAILGATE_DOOR};
}

// Error values
namespace error_values {
    const std::string BAD_AVG_FUEL_CONSUMPTION = "16383";
    const std::string BAD_DISTANCE_TO_EMPTY_FUEL = "16383";
    const std::string BAD_EV_TIME_TO_FULLY_CHARGED = "65535";
    const std::string BAD_TIRE_PRESSURE = "32767";
    const double BAD_LONGITUDE = 180.0;
    const double BAD_LATITUDE = 90.0;
    constexpr nullptr_t BAD_ODOMETER = nullptr;
    const std::string UNKNOWN = "UNKNOWN";
    const std::string NOT_EQUIPPED = "NOT_EQUIPPED";
}

// Time intervals
const int POLL_INTERVAL = 7200;
const int FETCH_INTERVAL = 300;

// Vehicle information keys
namespace vehicle_info {
    const std::string INFO = "vehicle_info";
    const std::string STATUS = "vehicle_status";
    const std::string HEALTH = "vehicle_health";
    const std::string MODEL_YEAR = "model_year";
    const std::string MODEL_NAME = "model_name";
    const std::string NAME = "vehicle_name";
    const std::string FEATURES = "vehicle_features";
    const std::string SUBSCRIPTION_FEATURES = "subscription_features";
    const std::string SUBSCRIPTION_STATUS = "subscription_status";
    const std::string CLIMATE = "climate";
    const std::string LAST_FETCH = "last_fetch";
    const std::string LAST_UPDATE = "last_update";
}

// API fields to redact
const std::vector<std::string> RAW_API_FIELDS_TO_REDACT = {
    "cachedStateCode",
    "customer",
    "email",
    "firstName",
    "lastName",
    "latitude",
    "licensePlate",
    "licensePlateState",
    "longitude",
    "nickname",
    "odometer",
    "odometerValue",
    "odometerValueKilometers",
    "oemCustId",
    "phone",
    "preferredDealer",
    "sessionCustomer",
    "timeZone",
    "userOemCustId",
    "vehicleGeoPosition",
    "vehicleKey",
    "vehicleMileage",
    "vehicleName",
    "vhsId",
    "vin",
    "zip"
};

} // namespace subarulink

#endif // SUBARULINK_CONSTANTS_HPP
