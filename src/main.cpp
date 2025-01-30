// main.cpp
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include "controller.h"

// Helper functions
std::string get_masked_input(const std::string& prompt) {
  std::string input;
  std::cout << prompt;
  // Note: In production, use platform-specific password masking
  std::getline(std::cin, input);
  return input;
}

std::string generate_device_id() {
  return std::to_string(
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch()
      ).count()
  );
}

bool handle_2fa_registration(subarulink::Controller& ctrl) {
  auto methods = ctrl.contact_methods();
  if (methods.empty()) {
    std::cerr << "No 2FA contact methods available" << std::endl;
    return false;
  }

  // Display available 2FA methods
  std::cout << "\nSelect 2FA method:" << std::endl;
  int i = 1;
  for (const auto& method : methods) {
    std::cout << i++ << ". " << method.second << std::endl;
  }

  // Get user choice
  int choice;
  std::cout << "Enter choice (1-" << methods.size() << "): ";
  std::cin >> choice;
  std::cin.ignore();  // Clear newline

  // Get selected method
  auto it = methods.begin();
  std::advance(it, choice - 1);

  // Request code
  std::cout << "Requesting authentication code..." << std::endl;
  if (!ctrl.request_auth_code(it->first).get()) {
    std::cerr << "Failed to request authentication code" << std::endl;
    return false;
  }

  // Get verification code from user
  std::cout << "Enter verification code sent to " << it->second << ": ";
  std::string code;
  std::getline(std::cin, code);

  // Submit verification code
  std::cout << "Submitting verification code..." << std::endl;
  if (!ctrl.submit_auth_code(code).get()) {
    std::cerr << "Failed to verify code" << std::endl;
    return false;
  }

  std::cout << "Device successfully registered!" << std::endl;
  return true;
}

void display_vehicle_info(const std::string& vin, subarulink::Controller& ctrl) {
  std::cout << "\nVehicle: " << ctrl.vin_to_name(vin) << std::endl;
  std::cout << "Year: " << ctrl.get_model_year(vin) << std::endl;
  std::cout << "Model: " << ctrl.get_model_name(vin) << std::endl;

  // Get detailed data
  auto vehicle_data = ctrl.get_data(vin).get();

  try {
    // Display odometer
    auto odo_it = vehicle_data.vehicle_status.find("ODOMETER");
    if (odo_it != vehicle_data.vehicle_status.end() && !odo_it->second.is_null()) {
      std::cout << "Odometer: " << odo_it->second.get<int>() << " miles" << std::endl;
    } else {
      std::cout << "Odometer: Not available" << std::endl;
    }

    // Display average fuel consumption
    auto mpg_it = vehicle_data.vehicle_status.find("AVG_FUEL_CONSUMPTION");
    if (mpg_it != vehicle_data.vehicle_status.end() && !mpg_it->second.is_null()) {
      std::cout << "Average MPG: " << std::fixed << std::setprecision(1)
                << mpg_it->second.get<double>() << std::endl;
    } else {
      std::cout << "Average MPG: Not available" << std::endl;
    }

    // Display range
    auto range_it = vehicle_data.vehicle_status.find("DISTANCE_TO_EMPTY_FUEL");
    if (range_it != vehicle_data.vehicle_status.end() && !range_it->second.is_null()) {
      std::cout << "Range: " << range_it->second.get<int>() << " miles" << std::endl;
    } else {
      std::cout << "Range: Not available" << std::endl;
    }

    // Display location if available
    auto lat_it = vehicle_data.vehicle_status.find("LATITUDE");
    auto lon_it = vehicle_data.vehicle_status.find("LONGITUDE");
    if (lat_it != vehicle_data.vehicle_status.end() && lon_it != vehicle_data.vehicle_status.end() &&
        !lat_it->second.is_null() && !lon_it->second.is_null()) {
      std::cout << "Location: " << std::fixed << std::setprecision(6)
                << lat_it->second.get<double>() << ", "
                << lon_it->second.get<double>() << std::endl;
    } else {
      std::cout << "Location: Not available" << std::endl;
    }

  } catch (const std::exception& e) {
    std::cout << "Error reading vehicle data: " << e.what() << std::endl;
  }
}

int main() {
  try {
    // Get credentials
    std::cout << "Enter Subaru Starlink username: ";
    std::string username;
    std::getline(std::cin, username);

    std::string password = get_masked_input("Enter Subaru Starlink password: ");
    std::string pin = get_masked_input("Enter Subaru Starlink PIN: ");

    // Create controller
    subarulink::Controller ctrl(
        username,
        password,
        generate_device_id(),
        pin,
        "SubarulinkCpp",  // device name
        "USA"             // country
    );

    // Connect - this will handle basic authentication
    if (!ctrl.connect().get()) {
      std::cerr << "Failed to connect" << std::endl;
      return 1;
    }

    // Handle 2FA if needed
    if (!ctrl.device_registered() && !handle_2fa_registration(ctrl)) {
      return 1;
    }

    // Get and display vehicle information
    auto vehicles = ctrl.get_vehicles();
    std::cout << "\nFound " << vehicles.size() << " vehicles:" << std::endl;

    for (const auto& vin : vehicles) {
      display_vehicle_info(vin, ctrl);
    }

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
