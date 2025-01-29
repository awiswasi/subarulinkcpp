#include <chrono>
#include <thread>

#include "controller.h"
#include "api_constants.h"
#include "exceptions.h"
#include "constants.h"

namespace subarulink {

  Controller::Controller(const std::string& username,
                         const std::string& password,
                         const std::string& device_id,
                         const std::string& pin,
                         const std::string& device_name,
                         const std::string& country,
                         int update_interval,
                         int fetch_interval)
      : _pin(pin),
        _country(country),
        _update_interval(update_interval),
        _fetch_interval(fetch_interval),
        _pin_lockout(false) {

    _connection = std::make_unique<Connection>(username, password, device_id, device_name, country);
  }

  std::future<bool> Controller::connect() {
    return std::async(std::launch::async, [this]() {
      auto vehicles = _connection->connect().get();
      for (const auto &vehicle: vehicles) {
        _parse_vehicle(vehicle);
      }
      return !vehicles.empty();
    });
  }

  bool Controller::device_registered() const {
    return _connection->device_registered();
  }

  const std::map <std::string, std::string> &Controller::contact_methods() const {
    return _connection->auth_contact_methods();
  }

  std::future<bool> Controller::request_auth_code(const std::string &contact_method) {
    return _connection->request_auth_code(contact_method);
  }

  std::future<bool> Controller::submit_auth_code(const std::string &code) {
    return _connection->submit_auth_code(code);
  }

  bool Controller::is_pin_required() const {
    for (const auto &pair: _vehicles) {
      if (get_remote_status(pair.first)) {
        return true;
      }
    }
    return false;
  }

  std::vector <std::string> Controller::get_vehicles() const {
    std::vector <std::string> result;
    for (const auto &pair: _vehicles) {
      result.push_back(pair.first);
    }
    return result;
  }

  std::string Controller::get_model_year(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      return it->second.model_year;
    }
    throw SubaruException("Invalid VIN");
  }

  std::string Controller::get_model_name(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      return it->second.model_name;
    }
    throw SubaruException("Invalid VIN");
  }

  bool Controller::get_ev_status(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      return std::find(it->second.vehicle_features.begin(),
                       it->second.vehicle_features.end(),
                       api::API_FEATURE_PHEV) != it->second.vehicle_features.end();
    }
    throw SubaruException("Invalid VIN");
  }

  bool Controller::get_remote_status(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      bool has_remote = std::find(it->second.subscription_features.begin(),
                                  it->second.subscription_features.end(),
                                  api::API_FEATURE_REMOTE) != it->second.subscription_features.end();
      return has_remote && get_subscription_status(vin);
    }
    throw SubaruException("Invalid VIN");
  }

  bool Controller::get_res_status(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      bool has_remote_start = std::find(it->second.vehicle_features.begin(),
                                        it->second.vehicle_features.end(),
                                        api::API_FEATURE_REMOTE_START) != it->second.vehicle_features.end();
      return has_remote_start && get_remote_status(vin);
    }
    throw SubaruException("Invalid VIN");
  }

  std::future<bool> Controller::has_power_windows(const std::string &vin) {
    return std::async(std::launch::async, [this, vin]() {
      auto it = _vehicles.find(vin);
      if (it != _vehicles.end()) {
        // Check if vehicle has explicit power window feature
        for (const auto &feature: api::API_FEATURE_WINDOWS_LIST) {
          if (std::find(it->second.vehicle_features.begin(),
                        it->second.vehicle_features.end(),
                        feature) != it->second.vehicle_features.end()) {
            return true;
          }
        }

        // Check for sunroof which implies power windows
        for (const auto &feature: api::API_FEATURE_MOONROOF_LIST) {
          if (std::find(it->second.vehicle_features.begin(),
                        it->second.vehicle_features.end(),
                        feature) != it->second.vehicle_features.end()) {
            return true;
          }
        }

        // Check G2 vehicles that might have windows without announcing feature
        if (get_api_gen(vin) == api::API_FEATURE_G2_TELEMATICS) {
          auto data_future = get_data(vin);
          auto vehicle_data = data_future.get();
          return !vehicle_data.vehicle_status.empty();  // Simplified check
        }
      }
      return false;
    });
  }

  bool Controller::has_sunroof(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      for (const auto &feature: api::API_FEATURE_MOONROOF_LIST) {
        if (std::find(it->second.vehicle_features.begin(),
                      it->second.vehicle_features.end(),
                      feature) != it->second.vehicle_features.end()) {
          return true;
        }
      }
      return false;
    }
    throw SubaruException("Invalid VIN");
  }

  std::future<bool> Controller::has_lock_status(const std::string &vin) {
    return std::async(std::launch::async, [this, vin]() {
      auto it = _vehicles.find(vin);
      if (it != _vehicles.end()) {
        // Check for explicit lock status feature
        if (std::find(it->second.vehicle_features.begin(),
                      it->second.vehicle_features.end(),
                      api::API_FEATURE_LOCK_STATUS) != it->second.vehicle_features.end()) {
          return true;
        }

        // Check if vehicle provides lock status without announcing feature
        std::string api_gen = get_api_gen(vin);
        if (api_gen == api::API_FEATURE_G2_TELEMATICS ||
            api_gen == api::API_FEATURE_G3_TELEMATICS) {
          auto data_future = get_data(vin);
          auto vehicle_data = data_future.get();
          // Check for actual lock status data
          return !vehicle_data.vehicle_status.empty();
        }
      }
      return false;
    });
  }

  bool Controller::has_tpms(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      return std::find(it->second.vehicle_features.begin(),
                       it->second.vehicle_features.end(),
                       api::API_FEATURE_TPMS) != it->second.vehicle_features.end();
    }
    throw SubaruException("Invalid VIN");
  }

  bool Controller::get_safety_status(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      bool has_safety = std::find(it->second.subscription_features.begin(),
                                  it->second.subscription_features.end(),
                                  api::API_FEATURE_SAFETY) != it->second.subscription_features.end();
      return has_safety && get_subscription_status(vin);
    }
    throw SubaruException("Invalid VIN");
  }

  bool Controller::get_subscription_status(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      return it->second.subscription_status == api::API_FEATURE_ACTIVE;
    }
    throw SubaruException("Invalid VIN");
  }

  std::string Controller::get_api_gen(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      if (std::find(it->second.vehicle_features.begin(),
                    it->second.vehicle_features.end(),
                    api::API_FEATURE_G1_TELEMATICS) != it->second.vehicle_features.end()) {
        return api::API_FEATURE_G1_TELEMATICS;
      }
      if (std::find(it->second.vehicle_features.begin(),
                    it->second.vehicle_features.end(),
                    api::API_FEATURE_G2_TELEMATICS) != it->second.vehicle_features.end()) {
        return api::API_FEATURE_G2_TELEMATICS;
      }
      if (std::find(it->second.vehicle_features.begin(),
                    it->second.vehicle_features.end(),
                    api::API_FEATURE_G3_TELEMATICS) != it->second.vehicle_features.end()) {
        return api::API_FEATURE_G3_TELEMATICS;
      }
    }
    throw SubaruException("Invalid VIN");
  }

  std::string Controller::vin_to_name(const std::string &vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      return it->second.vehicle_name;
    }
    throw SubaruException("Invalid VIN");
  }

  // Data Retrieval Methods

  std::future<VehicleInfo> Controller::get_data(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      auto it = _vehicles.find(vin);
      if (it != _vehicles.end()) {
        std::cout << "Debug: Found vehicle, checking status..." << std::endl;
        if (it->second.vehicle_status.empty()) {
          std::cout << "Debug: Vehicle status empty, fetching..." << std::endl;
          fetch(vin).get();
        }
        std::cout << "Debug: Returning vehicle data" << std::endl;
        return it->second;
      }
      throw SubaruException("Invalid VIN");
    });
  }

  nlohmann::json Controller::get_raw_data(const std::string &vin) const {
    auto it = _raw_api_data.find(vin);
    if (it != _raw_api_data.end()) {
      return it->second;
    }
    throw SubaruException("Invalid VIN");
  }

  std::future <std::vector<std::string>> Controller::list_climate_preset_names(const std::string &vin) {
    return std::async(std::launch::async, [this, vin]() {
      auto it = _vehicles.find(vin);
      if (it != _vehicles.end()) {
        std::vector <std::string> names;
        for (const auto &preset: it->second.climate) {
          names.push_back(preset["name"].get<std::string>());
        }
        return names;
      }
      throw SubaruException("Invalid VIN");
    });
  }

  std::future <nlohmann::json>
  Controller::get_climate_preset_by_name(const std::string &vin, const std::string &preset_name) {
    return std::async(std::launch::async, [this, vin, preset_name]() {
      auto it = _vehicles.find(vin);
      if (it != _vehicles.end()) {
        for (const auto &preset: it->second.climate) {
          if (preset["name"] == preset_name) {
            return preset;
          }
        }
        return nlohmann::json(nullptr);
      }
      throw SubaruException("Invalid VIN");
    });
  }

  std::future <std::vector<nlohmann::json>> Controller::get_user_climate_preset_data(const std::string &vin) {
    return std::async(std::launch::async, [this, vin]() {
      auto it = _vehicles.find(vin);
      if (it != _vehicles.end()) {
        std::vector <nlohmann::json> user_presets;
        for (const auto &preset: it->second.climate) {
          if (preset["presetType"] == "userPreset") {
            user_presets.push_back(preset);
          }
        }
        return user_presets;
      }
      throw SubaruException("Invalid VIN");
    });
  }

  std::future<bool> Controller::delete_climate_preset_by_name(const std::string &vin, const std::string &preset_name) {
    return std::async(std::launch::async, [this, vin, preset_name]() {
      auto preset = get_climate_preset_by_name(vin, preset_name).get();
      if (!preset.is_null() && preset["presetType"] == "userPreset") {
        auto user_presets = get_user_climate_preset_data(vin).get();
        auto it = std::find_if(user_presets.begin(), user_presets.end(),
                               [&preset_name](const nlohmann::json &p) {
                                 return p["name"] == preset_name;
                               });
        if (it != user_presets.end()) {
          user_presets.erase(it);
          return update_user_climate_presets(vin, user_presets).get();
        }
      }
      throw SubaruException("User preset '" + preset_name + "' not found");
    });
  }

  std::future<bool> Controller::update_user_climate_presets(const std::string &vin,
                                                            const std::vector <nlohmann::json> &preset_data) {
    return std::async(std::launch::async, [this, vin, preset_data]() {
      if (!_validate_remote_capability(vin)) {
        throw VehicleNotSupported(
            "Active STARLINK Security Plus subscription and remote start capable vehicle required.");
      }

      if (preset_data.size() > MAX_PRESETS) {
        throw SubaruException("Maximum of 4 climate presets allowed");
      }

      for (const auto &preset: preset_data) {
        if (!_validate_remote_start_params(vin, preset)) {
          throw SubaruException("Invalid climate preset parameters");
        }
      }

      auto response = _post(api::API_G2_SAVE_RES_SETTINGS, {}, preset_data).get();
      if (response["success"].get<bool>()) {
        return _fetch_climate_presets(vin).get();
      }
      return false;
    });
  }

  // Data Update Methods
  std::future<bool> Controller::fetch(const std::string& vin, bool force) {
    return std::async(std::launch::async, [this, vin, force]() {
      std::cout << "Debug: In fetch method for VIN: " << vin << std::endl;

      std::string upper_vin = vin;
      std::transform(upper_vin.begin(), upper_vin.end(), upper_vin.begin(), ::toupper);

      auto it = _vehicles.find(upper_vin);
      if (it != _vehicles.end()) {
        std::cout << "Debug: Found vehicle in _vehicles map" << std::endl;

        std::lock_guard<std::mutex> lock(_controller_mutex);
        auto last_fetch = it->second.last_fetch;
        auto current_time = std::chrono::system_clock::now();

        // If status is empty, we should force fetch regardless of time
        bool should_fetch = force ||
                            it->second.vehicle_status.empty() ||
                            std::chrono::duration_cast<std::chrono::seconds>(
                                current_time - last_fetch).count() > _fetch_interval;

        if (should_fetch) {
          std::cout << "Debug: Fetching fresh data..." << std::endl;
          bool result = _fetch_status(upper_vin).get();
          std::cout << "Debug: _fetch_status returned: " << result << std::endl;

          if (result) {
            it->second.last_fetch = current_time;
          }
          return result;
        } else {
          std::cout << "Debug: Using cached data" << std::endl;
        }
      } else {
        std::cout << "Debug: Vehicle not found in _vehicles map" << std::endl;
      }
      return false;
    });
  }

  std::future<bool> Controller::update(const std::string& vin, bool force) {
    return std::async(std::launch::async, [this, vin, force]() {
      std::string upper_vin = vin;
      std::transform(upper_vin.begin(), upper_vin.end(), upper_vin.begin(), ::toupper);

      if (!get_remote_status(upper_vin)) {
        throw VehicleNotSupported("Active STARLINK Security Plus subscription required.");
      }

      std::lock_guard<std::mutex> lock(_controller_mutex);
      auto it = _vehicles.find(upper_vin);
      auto last_update = it->second.last_update;
      auto current_time = std::chrono::system_clock::now();

      if (force || std::chrono::duration_cast<std::chrono::seconds>(
          current_time - last_update).count() > _update_interval) {
        bool result = _locate(upper_vin, true).get();
        if (result) {
          it->second.last_update = current_time;
        }
        return result;
      }
      return false;
    });
  }

  // Interval Management Methods
  int Controller::get_update_interval() const {
    return _update_interval;
  }

  bool Controller::set_update_interval(int value) {
    if (value >= 300) {  // Minimum 5 minutes
      _update_interval = value;
      return true;
    }
    return false;
  }

  int Controller::get_fetch_interval() const {
    return _fetch_interval;
  }

  bool Controller::set_fetch_interval(int value) {
    if (value >= 60) {  // Minimum 1 minute
      _fetch_interval = value;
      return true;
    }
    return false;
  }

  // Time Related Methods
  std::chrono::system_clock::time_point Controller::get_last_fetch_time(const std::string& vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      return it->second.last_fetch;
    }
    throw SubaruException("Invalid VIN");
  }

  std::chrono::system_clock::time_point Controller::get_last_update_time(const std::string& vin) const {
    auto it = _vehicles.find(vin);
    if (it != _vehicles.end()) {
      return it->second.last_update;
    }
    throw SubaruException("Invalid VIN");
  }

  std::future<bool> Controller::_fetch_status(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      std::cout << "Debug: Fetching vehicle status data..." << std::endl;

      try {
        auto vehicle_status = _get_vehicle_status(vin).get();
        _raw_api_data[vin]["vehicleStatus"] = vehicle_status;

        if (vehicle_status.find("success") != vehicle_status.end() &&
            vehicle_status["success"].get<bool>() &&
            vehicle_status.find("data") != vehicle_status.end()) {

          try {
            auto status = _parse_vehicle_status(vehicle_status, vin);

            // Use insert with iterators to handle all values
            for (auto it = status.begin(); it != status.end(); ++it) {
              _vehicles[vin].vehicle_status[it.key()] = it.value();
            }

            // Additional data for Security Plus and Gen2/3
            if (get_remote_status(vin) &&
                (get_api_gen(vin) == api::API_FEATURE_G2_TELEMATICS ||
                 get_api_gen(vin) == api::API_FEATURE_G3_TELEMATICS)) {

              std::cout << "Debug: Fetching additional data for G2/G3 vehicle" << std::endl;

              // Get condition data
              auto condition_resp = _remote_query(vin, api::API_CONDITION).get();
              if (condition_resp.find("success") != condition_resp.end() &&
                  condition_resp["success"].get<bool>()) {
                _raw_api_data[vin]["condition"] = condition_resp;
                if (condition_resp.find("data") != condition_resp.end()) {
                  auto condition_status = _parse_condition(condition_resp, vin).get();
                  for (auto it = condition_status.begin(); it != condition_status.end(); ++it) {
                    _vehicles[vin].vehicle_status[it.key()] = it.value();
                  }
                }
              }

              // Get vehicle health data
              auto health_resp = _remote_query(vin, api::API_VEHICLE_HEALTH).get();
              if (health_resp.find("success") != health_resp.end() &&
                  health_resp["success"].get<bool>()) {
                _raw_api_data[vin]["health"] = health_resp;
                if (health_resp.find("data") != health_resp.end()) {
                  auto health_data = _parse_health(health_resp, vin);
                  for (auto it = health_data.begin(); it != health_data.end(); ++it) {
                    _vehicles[vin].vehicle_health[it.key()] = it.value();
                  }
                }
              }

              // Get location data
              bool locate_success = _locate(vin).get();
            }

            // Fetch climate presets for supported vehicles
            if (get_res_status(vin) || get_ev_status(vin)) {
              _fetch_climate_presets(vin).get();
            }

            return true;
          } catch (const nlohmann::json::exception& e) {
            std::cout << "Debug: JSON parsing error: " << e.what() << std::endl;
            throw;
          }
        }
        std::cout << "Debug: Vehicle status response was not successful or missing data" << std::endl;
        return false;

      } catch (const std::exception& e) {
        std::cout << "Debug: Error in _fetch_status: " << e.what() << std::endl;
        if (std::string(e.what()).find("HTTP 500") != std::string::npos) {
          return false;
        }
        throw;
      }
    });
  }

  // Vehicle Control Methods

  std::future<bool> Controller::charge_start(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      if (!get_ev_status(vin)) {
        throw VehicleNotSupported("PHEV charging not supported for this vehicle");
      }
      auto [success, _] = _remote_command(vin, api::API_EV_CHARGE_NOW, api::API_REMOTE_SVC_STATUS).get();
      return success;
    });
  }

  std::future<bool> Controller::lock(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      nlohmann::json form_data = {{"forceKeyInCar", false}};
      auto [success, _] = _actuate(vin, api::API_LOCK, form_data).get();
      return success;
    });
  }

  std::future<bool> Controller::unlock(const std::string& vin, const std::string& door) {
    return std::async(std::launch::async, [this, vin, door]() {
      if (std::find(door::VALID_DOORS.begin(), door::VALID_DOORS.end(), door) != door::VALID_DOORS.end()) {
        nlohmann::json form_data = {{door::WHICH_DOOR, door}};
        auto [success, _] = _actuate(vin, api::API_UNLOCK, form_data).get();
        return success;
      }
      throw SubaruException("Invalid door specified for unlock command");
    });
  }

  std::future<bool> Controller::lights(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      std::string poll_url = api::API_REMOTE_SVC_STATUS;
      if (get_api_gen(vin) == api::API_FEATURE_G1_TELEMATICS) {
        poll_url = api::API_G1_HORN_LIGHTS_STATUS;
      }
      auto [success, _] = _actuate(vin, api::API_LIGHTS, nlohmann::json(), poll_url).get();
      return success;
    });
  }

  std::future<bool> Controller::lights_stop(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      std::string poll_url = api::API_REMOTE_SVC_STATUS;
      if (get_api_gen(vin) == api::API_FEATURE_G1_TELEMATICS) {
        poll_url = api::API_G1_HORN_LIGHTS_STATUS;
      }
      auto [success, _] = _actuate(vin, api::API_LIGHTS_STOP, nlohmann::json(), poll_url).get();
      return success;
    });
  }

  std::future<bool> Controller::horn(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      std::string poll_url = api::API_REMOTE_SVC_STATUS;
      if (get_api_gen(vin) == api::API_FEATURE_G1_TELEMATICS) {
        poll_url = api::API_G1_HORN_LIGHTS_STATUS;
      }
      auto [success, _] = _actuate(vin, api::API_HORN_LIGHTS, nlohmann::json(), poll_url).get();
      return success;
    });
  }

  std::future<bool> Controller::horn_stop(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      std::string poll_url = api::API_REMOTE_SVC_STATUS;
      if (get_api_gen(vin) == api::API_FEATURE_G1_TELEMATICS) {
        poll_url = api::API_G1_HORN_LIGHTS_STATUS;
      }
      auto [success, _] = _actuate(vin, api::API_HORN_LIGHTS_STOP, nlohmann::json(), poll_url).get();
      return success;
    });
  }

  std::future<bool> Controller::remote_stop(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      if (!get_res_status(vin) && !get_ev_status(vin)) {
        throw VehicleNotSupported("Remote Start not supported for this vehicle");
      }
      auto [success, _] = _actuate(vin, api::API_G2_REMOTE_ENGINE_STOP).get();
      return success;
    });
  }

  std::future<bool> Controller::remote_start(const std::string& vin, const std::string& preset_name) {
    return std::async(std::launch::async, [this, vin, preset_name]() {
      if (!_validate_remote_capability(vin)) {
        throw VehicleNotSupported("Remote start capability not available");
      }

      auto preset_data = get_climate_preset_by_name(vin, preset_name).get();
      if (!preset_data.is_null()) {
        auto response = _post(api::API_G2_SAVE_RES_QUICK_START_SETTINGS, {}, preset_data).get();
        if (response["success"].get<bool>()) {
          auto [success, _] = _actuate(vin, api::API_G2_REMOTE_ENGINE_START, preset_data).get();
          return success;
        }
        throw SubaruException("Failed to save climate preset settings");
      }
      throw SubaruException("Climate preset '" + preset_name + "' not found");
    });
  }

  // PIN Management
  bool Controller::invalid_pin_entered() const {
    return _pin_lockout;
  }

  bool Controller::update_saved_pin(const std::string& new_pin) {
    if (new_pin != _pin) {
      _pin = new_pin;
      _pin_lockout = false;
      return true;
    }
    return false;
  }

  // Private Helper Methods

  std::future<nlohmann::json> Controller::_get(const std::string& url,
                                               const std::map<std::string, std::string>& params) {
    return _connection->get(url, params);
  }

  std::future<nlohmann::json> Controller::_post(const std::string& url,
                                                const std::map<std::string, std::string>& params,
                                                const nlohmann::json& json_data) {
    return _connection->post(url, params, json_data);
  }

  void Controller::_check_error_code(const nlohmann::json& js_resp) {
    if (js_resp.contains("errorCode")) {
      std::string error = js_resp["errorCode"].get<std::string>();

      if (error == api::API_ERROR_INVALID_CREDENTIALS ||
          error == "SXM40006") {
        _pin_lockout = true;
        throw InvalidPIN("Invalid PIN: " + error);
      }

      if (error == api::API_ERROR_SERVICE_ALREADY_STARTED ||
          error == api::API_ERROR_G1_SERVICE_ALREADY_STARTED) {
        return;
      }

      throw SubaruException("Unhandled API error: " + error);
    }
  }

  void Controller::_parse_vehicle(const nlohmann::json& vehicle) {
    std::string vin = vehicle["vin"].get<std::string>();
    _vehicle_mutex.emplace(vin, std::make_unique<std::mutex>());
    _raw_api_data[vin] = {{"switchVehicle", vehicle}};

    VehicleInfo info;
    info.model_year = vehicle[api::API_VEHICLE_MODEL_YEAR].get<std::string>();
    info.model_name = vehicle[api::API_VEHICLE_MODEL_NAME].get<std::string>();
    info.vehicle_name = vehicle[api::API_VEHICLE_NAME].get<std::string>();
    info.vehicle_features = vehicle[api::API_VEHICLE_FEATURES].get<std::vector<std::string>>();
    info.subscription_features = vehicle[api::API_VEHICLE_SUBSCRIPTION_FEATURES].get<std::vector<std::string>>();
    info.subscription_status = vehicle[api::API_VEHICLE_SUBSCRIPTION_STATUS].get<std::string>();
    info.last_fetch = std::chrono::system_clock::now();
    info.last_update = std::chrono::system_clock::now();

    _vehicles[vin] = info;
  }

  nlohmann::json Controller::_parse_vehicle_status(const nlohmann::json& js_resp, const std::string& vin) {
    nlohmann::json status;
    auto data = js_resp["data"];
    auto& old_status = _vehicles[vin].vehicle_status;

    // Always valid values
    if (data.find(api::API_ODOMETER) != data.end() && !data[api::API_ODOMETER].is_null()) {
      status[vehicle_fields::ODOMETER] = data[api::API_ODOMETER].get<int>();
    }

    // Parse timestamp
    if (data.find(api::API_TIMESTAMP) != data.end()) {
      status[vehicle_fields::TIMESTAMP] = data[api::API_TIMESTAMP].get<std::string>();
    }

    // Optional values - keep old if present
    if (data.find(api::API_AVG_FUEL_CONSUMPTION) != data.end() &&
        !data[api::API_AVG_FUEL_CONSUMPTION].is_null()) {
      // Convert from string if necessary
      if (data[api::API_AVG_FUEL_CONSUMPTION].is_string()) {
        status[vehicle_fields::AVG_FUEL_CONSUMPTION] =
            std::stod(data[api::API_AVG_FUEL_CONSUMPTION].get<std::string>());
      } else {
        status[vehicle_fields::AVG_FUEL_CONSUMPTION] =
            data[api::API_AVG_FUEL_CONSUMPTION].get<double>();
      }
    }

    // Handle TPMS data
    if (has_tpms(vin)) {
      const std::vector<std::pair<std::string, std::string>> tire_sensors = {
          std::make_pair(vehicle_fields::TIRE_PRESSURE_FL, api::API_TIRE_PRESSURE_FL),
          std::make_pair(vehicle_fields::TIRE_PRESSURE_FR, api::API_TIRE_PRESSURE_FR),
          std::make_pair(vehicle_fields::TIRE_PRESSURE_RL, api::API_TIRE_PRESSURE_RL),
          std::make_pair(vehicle_fields::TIRE_PRESSURE_RR, api::API_TIRE_PRESSURE_RR)
      };

      for (const auto& [key, api_key] : tire_sensors) {
        if (data.find(api_key) != data.end() && !data[api_key].is_null()) {
          double value;
          if (data[api_key].is_string()) {
            value = std::stod(data[api_key].get<std::string>());
          } else {
            value = data[api_key].get<double>();
          }
          status[key] = std::round(value * 10.0) / 10.0;
        }
      }
    }

    return status;
  }

  nlohmann::json Controller::_parse_health(const nlohmann::json& js_resp, const std::string& vin) {
    nlohmann::json keep_data;
    auto data = js_resp["data"]["vehicleHealthItems"];

    keep_data["HEALTH_TROUBLE"] = false;
    keep_data["HEALTH_FEATURES"] = nlohmann::json::object();

    for (const auto& trouble_mil : data) {
      auto feature = trouble_mil[api::API_HEALTH_FEATURE].get<std::string>();
      auto& vehicle_features = _vehicles[vin].vehicle_features;

      if (std::find(vehicle_features.begin(), vehicle_features.end(), feature) != vehicle_features.end()) {
        nlohmann::json mil_item;
        mil_item["HEALTH_TROUBLE"] = false;
        mil_item["HEALTH_ONDATE"] = nullptr;

        if (trouble_mil[api::API_HEALTH_TROUBLE].get<bool>()) {
          mil_item["HEALTH_TROUBLE"] = true;
          auto ondates = trouble_mil[api::API_HEALTH_ONDATES].get<std::vector<std::string>>();
          std::sort(ondates.begin(), ondates.end(), std::greater<>());
          mil_item["HEALTH_ONDATE"] = ondates[0];
          keep_data["HEALTH_TROUBLE"] = true;
        }
        keep_data["HEALTH_FEATURES"][feature] = mil_item;
      }
    }

    return keep_data;
  }

  std::future<nlohmann::json> Controller::_parse_condition(const nlohmann::json& js_resp, const std::string& vin) {
    return std::async(std::launch::async, [this, js_resp, vin]() {
      nlohmann::json keep_data;
      auto data = js_resp["data"]["result"];

      // Basic data - treat all as strings for consistency
      for (const auto& [key, value] : {
          std::make_pair("DOOR_BOOT_POSITION", api::API_DOOR_BOOT_POSITION),
          std::make_pair("DOOR_ENGINE_HOOD_POSITION", api::API_DOOR_ENGINE_HOOD_POSITION),
          std::make_pair("DOOR_FRONT_LEFT_POSITION", api::API_DOOR_FRONT_LEFT_POSITION),
          std::make_pair("DOOR_FRONT_RIGHT_POSITION", api::API_DOOR_FRONT_RIGHT_POSITION),
          std::make_pair("DOOR_REAR_LEFT_POSITION", api::API_DOOR_REAR_LEFT_POSITION),
          std::make_pair("DOOR_REAR_RIGHT_POSITION", api::API_DOOR_REAR_RIGHT_POSITION)
      }) {
        if (data.find(value) != data.end() && !data[value].is_null()) {
          keep_data[key] = data[value].get<std::string>();
        }
      }

      // Handle timestamp
      if (data.find(api::API_LAST_UPDATED_DATE) != data.end()) {
        keep_data["TIMESTAMP"] = data[api::API_LAST_UPDATED_DATE].get<std::string>();
        keep_data["LAST_UPDATED_DATE"] = data[api::API_LAST_UPDATED_DATE].get<std::string>();
      }

      // Handle window status
      auto has_power_windows_future = has_power_windows(vin);
      if (has_power_windows_future.get()) {
        for (const auto& [key, value] : {
            std::make_pair("WINDOW_FRONT_LEFT_STATUS", api::API_WINDOW_FRONT_LEFT_STATUS),
            std::make_pair("WINDOW_FRONT_RIGHT_STATUS", api::API_WINDOW_FRONT_RIGHT_STATUS),
            std::make_pair("WINDOW_REAR_LEFT_STATUS", api::API_WINDOW_REAR_LEFT_STATUS),
            std::make_pair("WINDOW_REAR_RIGHT_STATUS", api::API_WINDOW_REAR_RIGHT_STATUS)
        }) {
          if (data.find(value) != data.end() && !data[value].is_null()) {
            keep_data[key] = data[value].get<std::string>();
          }
        }
      }

      // Handle sunroof if equipped
      if (has_sunroof(vin)) {
        if (data.find(api::API_WINDOW_SUNROOF_STATUS) != data.end()) {
          keep_data["WINDOW_SUNROOF_STATUS"] = data[api::API_WINDOW_SUNROOF_STATUS].get<std::string>();
        }
      }

      // Handle EV specific values
      if (get_ev_status(vin)) {
        if (data.find(api::API_EV_DISTANCE_TO_EMPTY) != data.end() &&
            !data[api::API_EV_DISTANCE_TO_EMPTY].is_null()) {
          if (data[api::API_EV_DISTANCE_TO_EMPTY].is_string()) {
            keep_data["EV_DISTANCE_TO_EMPTY"] =
                std::stoi(data[api::API_EV_DISTANCE_TO_EMPTY].get<std::string>());
          } else {
            keep_data["EV_DISTANCE_TO_EMPTY"] =
                data[api::API_EV_DISTANCE_TO_EMPTY].get<int>();
          }
        }
      }

      // Add debug output
      std::cout << "Debug: Parsed condition data: " << keep_data.dump(2) << std::endl;

      return keep_data;
    });
  }

  std::future<bool> Controller::_fetch_climate_presets(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      if (get_res_status(vin) || get_ev_status(vin)) {
        std::vector<nlohmann::json> presets;

        // Fetch STARLINK Presets
        auto js_resp = _post(api::API_G2_FETCH_RES_SUBARU_PRESETS).get();
        _raw_api_data[vin]["climatePresetSettings"] = js_resp;

        if (js_resp.contains("data")) {
          for (const auto& preset : js_resp["data"]) {
            auto preset_data = nlohmann::json::parse(preset.get<std::string>());
            if (get_ev_status(vin) && preset_data["vehicleType"] == "phev") {
              presets.push_back(preset_data);
            } else if (!get_ev_status(vin) && preset_data["vehicleType"] == "gas") {
              presets.push_back(preset_data);
            }
          }
        }

        // Fetch User Defined Presets
        js_resp = _post(api::API_G2_FETCH_RES_USER_PRESETS).get();
        _raw_api_data[vin]["remoteEngineStartSettings"] = js_resp;

        if (js_resp.contains("data") && js_resp["data"].is_string()) {
          auto user_presets = nlohmann::json::parse(js_resp["data"].get<std::string>());
          for (const auto& preset : user_presets) {
            presets.push_back(preset);
          }
        }

        _vehicles[vin].climate = presets;
        return true;
      }
      throw VehicleNotSupported("Active STARLINK Security Plus subscription required.");
    });
  }

  std::future<std::tuple<bool, nlohmann::json>> Controller::_actuate(
      const std::string& vin,
      const std::string& cmd,
      const nlohmann::json& data,
      const std::string& poll_url) {

    return std::async(std::launch::async, [this, vin, cmd, data, poll_url]() {
      nlohmann::json form_data = {
          {"delay", 0},
          {"vin", vin}
      };

      if (!data.is_null()) {
        form_data.update(data);
      }

      if (get_remote_status(vin)) {
        return _remote_command(vin, cmd, poll_url, form_data).get();
      }
      throw VehicleNotSupported("Active STARLINK Security Plus subscription required.");
    });
  }

  bool Controller::_validate_remote_start_params(const std::string& vin, const nlohmann::json& preset_data) {
    bool is_valid = true;
    std::string err_msg;
    nlohmann::json data_copy = preset_data;  // Create a mutable copy

    try {
      // Validate each preset parameter against valid options
      for (const auto& [key, value] : preset_data.items()) {
        if (key == climate_control::PRESET_NAME && value.is_string()) {
          continue;
        }

        auto it = climate_control::VALID_CLIMATE_OPTIONS.find(key);
        if (it != climate_control::VALID_CLIMATE_OPTIONS.end()) {
          if (std::find(it->second.begin(), it->second.end(), value.get<std::string>()) == it->second.end()) {
            is_valid = false;
            err_msg = "Invalid value for " + key + ": " + value.dump();
            break;
          }
        }
      }
    } catch (const std::exception& e) {
      is_valid = false;
      err_msg = std::string("Invalid option: ") + e.what();
    }

    if (!is_valid) {
      throw SubaruException(err_msg);
    }

    // Update config constants based on vehicle type
    if (get_ev_status(vin)) {
      for (const auto& [key, value] : climate_control::START_CONFIG_CONSTS_EV) {
        data_copy[key] = value;  // Modify the copy instead
      }
    } else {
      for (const auto& [key, value] : climate_control::START_CONFIG_CONSTS_RES) {
        data_copy[key] = value;  // Modify the copy instead
      }
    }

    return is_valid;
  }

  std::future<nlohmann::json> Controller::_remote_query(const std::string& vin, const std::string& cmd) {
    return std::async(std::launch::async, [this, vin, cmd]() {
      int tries_left = 2;
      nlohmann::json js_resp;

      while (tries_left > 0) {
        _connection->validate_session(vin).get();

        // Get API generation
        std::string api_gen = get_api_gen(vin);
        if (api_gen == api::API_FEATURE_G1_TELEMATICS) {
          api_gen = "g1";
        } else {
          api_gen = "g2";  // G3 uses G2 API for now
        }

        {
          std::lock_guard<std::mutex> lock(*_vehicle_mutex[vin]);

          // Create the modified command properly
          std::string modified_cmd = cmd;
          size_t pos = modified_cmd.find("api_gen");
          if (pos != std::string::npos) {
            modified_cmd.replace(pos, 7, api_gen); // 7 is length of "api_gen"
          }

          std::cout << "Debug: Making remote query to: " << modified_cmd << std::endl;

          js_resp = _post(modified_cmd).get();

          if (js_resp["success"].get<bool>()) {
            return js_resp;
          }

          if (js_resp.find("errorCode") != js_resp.end() &&
              js_resp["errorCode"] == api::API_ERROR_SOA_403) {
            tries_left--;
          } else {
            tries_left = 0;
          }
        }
      }
      throw SubaruException("Remote query failed. Response: " + js_resp.dump());
    });
  }

  std::future<bool> Controller::_locate(const std::string& vin, bool hard_poll) {
    return std::async(std::launch::async, [this, vin, hard_poll]() {
      nlohmann::json js_resp;
      bool success = false;

      if (hard_poll) {
        // Send locate command to get real-time position
        std::string api_gen = get_api_gen(vin);
        std::string locate_cmd = (api_gen == api::API_FEATURE_G1_TELEMATICS) ?
                                 api::API_G1_LOCATE_UPDATE : api::API_G2_LOCATE_UPDATE;
        std::string poll_url = (api_gen == api::API_FEATURE_G1_TELEMATICS) ?
                               api::API_G1_LOCATE_STATUS : api::API_G2_LOCATE_STATUS;

        try {
          std::cout << "Debug: Starting locate request..." << std::endl;
          auto result = _remote_command(vin, locate_cmd, poll_url).get();
          success = std::get<0>(result);
          js_resp = std::get<1>(result);

          if (success && js_resp["success"].get<bool>()) {
            if (js_resp["data"].contains("result")) {
              std::cout << "Debug: Processing locate result..." << std::endl;
              _parse_location(vin, js_resp["data"]["result"]);
            } else {
              // Initiate a regular locate query since the command only gave us status
              std::cout << "Debug: No location data in response, fetching location..." << std::endl;
              js_resp = _remote_query(vin, api::API_LOCATE).get();
              _raw_api_data[vin]["locate"] = js_resp;
              if (js_resp["success"].get<bool>() && js_resp["data"].contains("result")) {
                _parse_location(vin, js_resp["data"]["result"]);
                return true;
              }
            }
          }
        } catch (const nlohmann::json::exception& e) {
          std::cout << "Debug: JSON error in _locate: " << e.what() << std::endl;
          std::cout << "Debug: Response was: " << js_resp.dump(2) << std::endl;
          return false;
        }
      } else {
        // Get last reported location
        try {
          js_resp = _remote_query(vin, api::API_LOCATE).get();
          _raw_api_data[vin]["locate"] = js_resp;
          if (js_resp["success"].get<bool>() && js_resp["data"].contains("result")) {
            _parse_location(vin, js_resp["data"]["result"]);
            return true;
          }
        } catch (const nlohmann::json::exception& e) {
          std::cout << "Debug: JSON error in _locate: " << e.what() << std::endl;
          std::cout << "Debug: Response was: " << js_resp.dump(2) << std::endl;
          return false;
        }
      }

      return false;
    });
  }

  std::future<std::tuple<bool, bool, nlohmann::json>> Controller::_execute_remote_command(
      const std::string& vin,
      const std::string& cmd,
      const nlohmann::json& data,
      const std::string& poll_url) {

    return std::async(std::launch::async, [this, vin, cmd, data, poll_url]() {
      // G3 uses G2 API for now
      std::string api_gen = (get_api_gen(vin) == api::API_FEATURE_G1_TELEMATICS) ? "g1" : "g2";

      nlohmann::json form_data = {
          {"pin", _pin},
          {"delay", 0},
          {"vin", vin}
      };

      if (!data.is_null()) {
        form_data.update(data);
      }

      // Create a local copy of cmd before modifying it
      std::string modified_cmd = cmd;
      modified_cmd.replace(modified_cmd.find("api_gen"), 7, api_gen);

      auto js_resp = _post(modified_cmd, {}, form_data).get();

      if (js_resp["errorCode"] == api::API_ERROR_SOA_403) {
        return std::make_tuple(true, false, js_resp);
      }

      if (js_resp["errorCode"] == api::API_ERROR_G1_SERVICE_ALREADY_STARTED ||
          js_resp["errorCode"] == api::API_ERROR_SERVICE_ALREADY_STARTED) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        return std::make_tuple(true, false, js_resp);
      }

      if (js_resp["success"].get<bool>()) {
        std::string req_id = js_resp["data"][api::API_SERVICE_REQ_ID];
        auto [success, response] = _wait_request_status(vin, req_id, poll_url).get();
        return std::make_tuple(false, success, response);
      }

      return std::make_tuple(false, false, js_resp);
    });
  }

  std::future<std::tuple<bool, nlohmann::json>> Controller::_remote_command(
      const std::string& vin,
      const std::string& cmd,
      const std::string& poll_url,
      const nlohmann::json& data) {
    return std::async(std::launch::async, [this, vin, cmd, poll_url, data]() {
      bool try_again = true;
      while (try_again && !_pin_lockout) {
        if (_connection->get_session_age() > MAX_SESSION_AGE_MINS) {
          _connection->reset_session();
        }

        _connection->validate_session(vin).get();

        auto [again, success, response] = _execute_remote_command(vin, cmd, data, poll_url).get();
        try_again = again;

        if (success) {
          return std::make_tuple(true, response);
        }
      }

      if (_pin_lockout) {
        throw PINLockoutProtect("Remote command cancelled to prevent account lockout");
      }

      throw SubaruException("Unexpected error in remote command");
    });
  }

  std::future<nlohmann::json> Controller::_get_vehicle_status(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      std::cout << "Debug: In _get_vehicle_status for VIN: " << vin << std::endl;

      try {
        std::cout << "Debug: Validating session..." << std::endl;
        _connection->validate_session(vin).get();

        std::cout << "Debug: Making API_VEHICLE_STATUS request..." << std::endl;
        auto response = _get(api::API_VEHICLE_STATUS).get();

        std::cout << "Debug: Vehicle status API response: " << response.dump(2) << std::endl;
        return response;

      } catch (const std::exception& e) {
        std::cout << "Debug: Error in _get_vehicle_status: " << e.what() << std::endl;
        throw;
      }
    });
  }

  bool Controller::_validate_remote_capability(const std::string& vin) {
    return get_res_status(vin) || get_ev_status(vin);
  }

  void Controller::_check_pin_lockout() const {
    if (_pin_lockout) {
      throw PINLockoutProtect("Remote command cancelled to prevent account lockout");
    }
  }

  void Controller::_validate_vin(const std::string& vin) const {
    if (_vehicles.find(vin) == _vehicles.end()) {
      throw SubaruException("Invalid VIN");
    }
  }

  void Controller::_validate_pin(const std::string& pin) const {
    if (pin.length() != PIN_LENGTH || !std::all_of(pin.begin(), pin.end(), ::isdigit)) {
      throw InvalidPIN("PIN must be 4 digits");
    }
  }

  std::future<std::tuple<bool, nlohmann::json>> Controller::_wait_request_status(
      const std::string& vin,
      const std::string& req_id,
      const std::string& poll_url,
      int attempts) {

    return std::async(std::launch::async, [this, vin, req_id, poll_url, attempts]() {
      int remaining_attempts = attempts;

      while (remaining_attempts > 0) {
        try {
          _connection->validate_session(vin).get();

          std::map<std::string, std::string> params = {
              {"serviceRequestId", req_id}
          };

          auto js_resp = _post(poll_url, params).get();
          _check_error_code(js_resp);

          if (js_resp["success"].get<bool>()) {
            // Check if service is completed
            auto status = js_resp["data"]["remoteServiceState"].get<std::string>();

            if (status == "SUCCESS") {
              return std::make_tuple(true, js_resp);
            } else if (status == "FAILED") {
              return std::make_tuple(false, js_resp);
            }
          }
        } catch (const SubaruException& e) {
          if (std::string(e.what()).find("HTTP 500") != std::string::npos) {
            // Server error, continue polling
            remaining_attempts--;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
          }
          throw;
        }

        remaining_attempts--;
        if (remaining_attempts > 0) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      }

      return std::make_tuple(false, nlohmann::json());
    });
  }

  void Controller::_parse_location(const std::string& vin, const nlohmann::json& result) {
    auto& vehicle_status = _vehicles[vin].vehicle_status;

    // Initialize location validity flag
    vehicle_status["LOCATION_VALID"] = false;

    // Check if location data exists and is valid
    if (result.find("longitude") != result.end() && result.find("latitude") != result.end()) {
      double longitude = 0.0;
      double latitude = 0.0;

      // Handle longitude
      if (result["longitude"].is_string()) {
        longitude = std::stod(result["longitude"].get<std::string>());
      } else {
        longitude = result["longitude"].get<double>();
      }

      // Handle latitude
      if (result["latitude"].is_string()) {
        latitude = std::stod(result["latitude"].get<std::string>());
      } else {
        latitude = result["latitude"].get<double>();
      }

      // Check if coordinates are valid (not default/error values)
      if (longitude != error_values::BAD_LONGITUDE &&
          latitude != error_values::BAD_LATITUDE) {

        vehicle_status["LONGITUDE"] = longitude;
        vehicle_status["LATITUDE"] = latitude;
        vehicle_status["LOCATION_VALID"] = true;

        // Add timestamp if available
        if (result.find("locationTimestamp") != result.end()) {
          vehicle_status["LOCATION_TIMESTAMP"] = result["locationTimestamp"].get<std::string>();
        }
      }
    }

    // Parse heading if available
    if (result.find("heading") != result.end() && !result["heading"].is_null()) {
      if (result["heading"].is_string()) {
        vehicle_status["HEADING"] = result["heading"].get<std::string>();
      } else if (result["heading"].is_number()) {
        vehicle_status["HEADING"] = std::to_string(result["heading"].get<double>());
      }
    }

    // Parse location name/address if available
    if (result.find("locationName") != result.end() && !result["locationName"].is_null()) {
      vehicle_status["LOCATION_NAME"] = result["locationName"].get<std::string>();
    }

    std::cout << "Debug: Parsed location data: " << nlohmann::json(vehicle_status).dump(2) << std::endl;
  }

} // namespace subarulink
