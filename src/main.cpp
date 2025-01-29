// main.cpp
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "controller.h"


std::string get_masked_input(const std::string& prompt) {
  std::string input;
  std::cout << prompt;
  // Note: In production, use platform-specific password masking
  std::getline(std::cin, input);
  return input;
}

int main() {
  try {
    // Get credentials
    std::cout << "Enter Subaru Starlink username: ";
    std::string username;
    std::getline(std::cin, username);

    std::string password = get_masked_input("Enter Subaru Starlink password: ");
    std::string pin = get_masked_input("Enter Subaru Starlink PIN: ");

    // Generate device ID based on seconds since epoch
    std::string device_id = std::to_string(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );

    // Create controller
    subarulink::Controller ctrl(
        username,
        password,
        device_id,
        pin,
        "SubarulinkCpp", // device name
        "USA"           // country
    );

    // Connect - this will handle basic authentication
    auto connect_future = ctrl.connect();
    if (!connect_future.get()) {
      std::cerr << "Failed to connect" << std::endl;
      return 1;
    }

    // Check if device needs to be registered (2FA required)
    if (!ctrl.device_registered()) {
      auto methods = ctrl.contact_methods();
      if (methods.empty()) {
        std::cerr << "No 2FA contact methods available" << std::endl;
        return 1;
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
      auto request_future = ctrl.request_auth_code(it->first);
      if (!request_future.get()) {
        std::cerr << "Failed to request authentication code" << std::endl;
        return 1;
      }

      // Get verification code from user
      std::cout << "Enter verification code sent to " << it->second << ": ";
      std::string code;
      std::getline(std::cin, code);

      // Submit verification code
      std::cout << "Submitting verification code..." << std::endl;
      auto verify_future = ctrl.submit_auth_code(code);
      if (!verify_future.get()) {
        std::cerr << "Failed to verify code" << std::endl;
        return 1;
      }

      std::cout << "Device successfully registered!" << std::endl;
    }

    // Now proceed with getting vehicle information
    auto vehicles = ctrl.get_vehicles();
    std::cout << "\nFound " << vehicles.size() << " vehicles:" << std::endl;

    for (const auto& vin : vehicles) {
      std::cout << "\nVehicle: " << ctrl.vin_to_name(vin) << std::endl;
      std::cout << "Year: " << ctrl.get_model_year(vin) << std::endl;
      std::cout << "Model: " << ctrl.get_model_name(vin) << std::endl;

      // Get detailed data
      auto data_future = ctrl.get_data(vin);
      auto vehicle_data = data_future.get();

      // Display odometer
      try {
        auto it = vehicle_data.vehicle_status.find("ODOMETER");
        if (it != vehicle_data.vehicle_status.end() && !it->second.is_null()) {
          std::cout << "Odometer: " << it->second.get<int>()
                    << " miles" << std::endl;
        } else {
          std::cout << "Odometer data not available" << std::endl;
        }
      } catch (const std::exception& e) {
        std::cout << "Error reading odometer: " << e.what() << std::endl;
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}