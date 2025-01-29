# SubarulinkCpp

A modern C++ library for interacting with Subaru's STARLINK vehicle services. This library allows you to remotely monitor and control your Subaru vehicle using the STARLINK API.

## Features

- Vehicle status monitoring (location, odometer, tire pressure, etc.)
- Remote control capabilities (lock/unlock, lights, horn, remote start)
- Climate control management with presets
- EV-specific features for supported vehicles
- Comprehensive vehicle health monitoring
- Support for both Generation 1 and Generation 2/3 STARLINK telematics
- Asynchronous API design using std::future
- Thread-safe implementation

## Requirements

- C++17 or higher
- CMake 3.14 or higher
- nlohmann/json 3.11.2
- libcpr (included as submodule)
- OpenSSL
- Threads

## Installation

1. Clone the repository with submodules:
```bash
git clone --recursive https://github.com/awiswasi/subarulinkcpp.git
cd subarulinkcpp
```

2. Create build directory and compile:
```bash
mkdir build && cd build
cmake ..
make
```

## Usage Example

Here's a basic example of how to use the library:

```cpp
#include <iostream>
#include "controller.h"

int main() {
    try {
        // Initialize controller
        subarulink::Controller ctrl(
            "your_email@example.com",  // STARLINK username
            "your_password",           // STARLINK password
            "device_id",              // Unique device identifier
            "1234",                   // STARLINK PIN
            "MyDevice",               // Device name
            "USA"                     // Country (USA or CAN)
        );

        // Connect to the service
        if (!ctrl.connect().get()) {
            std::cerr << "Failed to connect" << std::endl;
            return 1;
        }

        // Get list of vehicles
        auto vehicles = ctrl.get_vehicles();
        
        for (const auto& vin : vehicles) {
            std::cout << "Vehicle: " << ctrl.vin_to_name(vin) << std::endl;
            
            // Fetch latest vehicle data
            auto data = ctrl.get_data(vin).get();
            
            // Print odometer reading
            auto it = data.vehicle_status.find("ODOMETER");
            if (it != data.vehicle_status.end()) {
                std::cout << "Odometer: " << it->second << " miles" << std::endl;
            }

            // Lock the vehicle
            if (ctrl.get_remote_status(vin)) {
                bool success = ctrl.lock(vin).get();
                std::cout << "Lock command " << (success ? "succeeded" : "failed") << std::endl;
            }
        }
    }
    catch (const subarulink::SubaruException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

### Remote Start with Climate Control

Here's an example of using remote start with climate control presets:

```cpp
// Get available climate presets
auto presets = ctrl.list_climate_preset_names(vin).get();
for (const auto& preset : presets) {
    std::cout << "Available preset: " << preset << std::endl;
}

// Start the vehicle with a specific preset
if (ctrl.get_res_status(vin)) {
    bool success = ctrl.remote_start(vin, "Preset1").get();
    std::cout << "Remote start " << (success ? "succeeded" : "failed") << std::endl;
}
```

## Vehicle Features

The library can check for various vehicle capabilities:

```cpp
std::string vin = "YOUR_VIN";

// Check various vehicle capabilities
bool has_remote = ctrl.get_remote_status(vin);
bool has_remote_start = ctrl.get_res_status(vin);
bool is_ev = ctrl.get_ev_status(vin);
bool has_tpms = ctrl.has_tpms(vin);
bool has_windows = ctrl.has_power_windows(vin).get();
```

## Error Handling

The library uses custom exceptions for error handling:

```cpp
try {
    ctrl.remote_start(vin, "Preset1").get();
} catch (const subarulink::InvalidPIN& e) {
    std::cerr << "PIN error: " << e.what() << std::endl;
} catch (const subarulink::VehicleNotSupported& e) {
    std::cerr << "Vehicle doesn't support this feature: " << e.what() << std::endl;
} catch (const subarulink::SubaruException& e) {
    std::cerr << "General error: " << e.what() << std::endl;
}
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Disclaimer

This project is not affiliated with or endorsed by Subaru. Use at your own risk.
