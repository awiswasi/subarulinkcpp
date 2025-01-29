#include "constants.h"

namespace subarulink {

// Climate control static member definitions
  namespace climate_control {

// Mode definitions
    const std::string Mode::DEFROST = "DEFROST";
    const std::string Mode::FEET_DEFROST = "FEET_DEFROST";
    const std::string Mode::FACE = "FACE";
    const std::string Mode::FEET = "FEET";
    const std::string Mode::SPLIT = "SPLIT";
    const std::string Mode::AUTO = "AUTO";

// HeatSeat definitions
    const std::string HeatSeat::HIGH = "HIGH";
    const std::string HeatSeat::MEDIUM = "MEDIUM";
    const std::string HeatSeat::LOW = "LOW";
    const std::string HeatSeat::OFF = "OFF";

// FanSpeed definitions
    const std::string FanSpeed::LOW = "LOW";
    const std::string FanSpeed::MEDIUM = "MEDIUM";
    const std::string FanSpeed::HIGH = "HIGH";
    const std::string FanSpeed::AUTO = "AUTO";

// Constants for temp ranges (already defined in header as constexpr/const)
// const int TEMP_F_MAX = 85;
// const int TEMP_F_MIN = 60;
// const int TEMP_C_MAX = 30;
// const int TEMP_C_MIN = 15;

// Runtime constants (already defined in header)
// const std::string RUNTIME_10_MIN = "10";
// const std::string RUNTIME_5_MIN = "5";

// Other climate control constants are already properly defined in the header
// as they are not static class members

  } // namespace climate_control

// Other namespaces don't have static members that need definition

// vehicle_fields namespace constants are already properly defined in header
// health namespace constants are already properly defined in header
// door namespace constants are already properly defined in header
// error_values namespace constants are already properly defined in header
// vehicle_info namespace constants are already properly defined in header

} // namespace subarulink