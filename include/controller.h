#pragma once
#ifndef SUBARULINK_CONTROLLER_HPP
#define SUBARULINK_CONTROLLER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <future>
#include <mutex>

#include "nlohmann/json.hpp"

#include "connection.h"

namespace subarulink {

  struct VehicleInfo {
    std::string model_year;
    std::string model_name;
    std::string vehicle_name;
    std::vector<std::string> vehicle_features;
    std::vector<std::string> subscription_features;
    std::string subscription_status;
    std::map<std::string, nlohmann::json> vehicle_status;
    std::map<std::string, nlohmann::json> vehicle_health;
    std::vector<nlohmann::json> climate;
    std::chrono::system_clock::time_point last_fetch;
    std::chrono::system_clock::time_point last_update;
  };

  class Controller {
  public:
    Controller(const std::string& username,
               const std::string& password,
               const std::string& device_id,
               const std::string& pin,
               const std::string& device_name,
               const std::string& country = "USA",
               int update_interval = 7200,
               int fetch_interval = 300);

    // Public Methods
    std::future<bool> connect();
    bool device_registered() const;
    const std::map<std::string, std::string>& contact_methods() const;
    std::future<bool> request_auth_code(const std::string& contact_method);
    std::future<bool> submit_auth_code(const std::string& code);
    bool is_pin_required() const;
    std::future<bool> test_pin();
    std::vector<std::string> get_vehicles() const;

    // Vehicle Information Methods
    std::string get_model_year(const std::string& vin) const;
    std::string get_model_name(const std::string& vin) const;
    bool get_ev_status(const std::string& vin) const;
    bool get_remote_status(const std::string& vin) const;
    bool get_res_status(const std::string& vin) const;
    std::future<bool> has_power_windows(const std::string& vin);
    bool has_sunroof(const std::string& vin) const;
    std::future<bool> has_lock_status(const std::string& vin);
    bool has_tpms(const std::string& vin) const;
    bool get_safety_status(const std::string& vin) const;
    bool get_subscription_status(const std::string& vin) const;
    std::string get_api_gen(const std::string& vin) const;
    std::string vin_to_name(const std::string& vin) const;

    // Data Retrieval Methods
    std::future<VehicleInfo> get_data(const std::string& vin);
    nlohmann::json get_raw_data(const std::string& vin) const;
    std::future<std::vector<std::string>> list_climate_preset_names(const std::string& vin);
    std::future<nlohmann::json> get_climate_preset_by_name(const std::string& vin, const std::string& preset_name);
    std::future<std::vector<nlohmann::json>> get_user_climate_preset_data(const std::string& vin);

    // Climate Control Methods
    std::future<bool> delete_climate_preset_by_name(const std::string& vin, const std::string& preset_name);
    std::future<bool> update_user_climate_presets(const std::string& vin, const std::vector<nlohmann::json>& preset_data);

    // Data Update Methods
    std::future<bool> fetch(const std::string& vin, bool force = false);
    std::future<bool> update(const std::string& vin, bool force = false);

    // Interval Management
    int get_update_interval() const;
    bool set_update_interval(int value);
    int get_fetch_interval() const;
    bool set_fetch_interval(int value);

    // Time Related Methods
    std::chrono::system_clock::time_point get_last_fetch_time(const std::string& vin) const;
    std::chrono::system_clock::time_point get_last_update_time(const std::string& vin) const;

    // Vehicle Control Methods
    std::future<bool> charge_start(const std::string& vin);
    std::future<bool> lock(const std::string& vin);
    std::future<bool> unlock(const std::string& vin, const std::string& door = "ALL_DOORS_CMD");
    std::future<bool> lights(const std::string& vin);
    std::future<bool> lights_stop(const std::string& vin);
    std::future<bool> horn(const std::string& vin);
    std::future<bool> horn_stop(const std::string& vin);
    std::future<bool> remote_stop(const std::string& vin);
    std::future<bool> remote_start(const std::string& vin, const std::string& preset_name);

    // PIN Management
    bool invalid_pin_entered() const;
    bool update_saved_pin(const std::string& new_pin);

  private:
    std::unique_ptr<Connection> _connection;
    std::string _country;
    int _update_interval;
    int _fetch_interval;
    std::map<std::string, VehicleInfo> _vehicles;
    std::map<std::string, std::unique_ptr<std::mutex>> _vehicle_mutex;
    std::string _pin;
    std::mutex _controller_mutex;
    bool _pin_lockout;
    std::map<std::string, nlohmann::json> _raw_api_data;
    std::string version;

    // Private Helper Methods
    std::future<nlohmann::json> _get(const std::string& url, const std::map<std::string, std::string>& params = {});
    std::future<nlohmann::json> _post(const std::string& url,
                                      const std::map<std::string, std::string>& params = {},
                                      const nlohmann::json& json_data = nlohmann::json());
    void _check_error_code(const nlohmann::json& js_resp);
    void _parse_vehicle(const nlohmann::json& vehicle);

    // Remote Service Methods
    std::future<std::tuple<bool, nlohmann::json>> _remote_command(
        const std::string& vin,
        const std::string& cmd,
        const std::string& poll_url,
        const nlohmann::json& data = nlohmann::json());

    std::future<std::tuple<bool, nlohmann::json>> _actuate(
        const std::string& vin,
        const std::string& cmd,
        const nlohmann::json& data = nlohmann::json(),
        const std::string& poll_url = "/g2v30/remoteService/status.json");

    std::future<nlohmann::json> _get_vehicle_status(const std::string& vin);
    std::future<bool> _fetch_status(const std::string& vin);
    std::future<bool> _locate(const std::string& vin, bool hard_poll = false);
    void _parse_location(const std::string& vin, const nlohmann::json& result);

    // Request Status Methods
    std::future<std::tuple<bool, nlohmann::json>> _wait_request_status(
        const std::string& vin,
        const std::string& req_id,
        const std::string& poll_url,
        int attempts = 20);

    // Climate Control Methods
    std::future<bool> _fetch_climate_presets(const std::string& vin);
    bool _validate_remote_start_params(const std::string& vin, const nlohmann::json& preset_data);
    bool _validate_remote_capability(const std::string& vin);

    // Data Parsing Methods
    nlohmann::json _parse_vehicle_status(const nlohmann::json& js_resp, const std::string& vin);
    std::future<nlohmann::json> _parse_condition(const nlohmann::json& js_resp, const std::string& vin);
    nlohmann::json _parse_health(const nlohmann::json& js_resp, const std::string& vin);
    nlohmann::json _parse_recommended_tire_pressure(const std::string& vin);

    // Remote Query Methods
    std::future<nlohmann::json> _remote_query(const std::string& vin, const std::string& cmd);
    std::future<std::tuple<bool, bool, nlohmann::json>> _execute_remote_command(
        const std::string& vin,
        const std::string& cmd,
        const nlohmann::json& data,
        const std::string& poll_url);

    // Error Handling Methods
    void _check_pin_lockout() const;
    void _validate_vin(const std::string& vin) const;
    void _validate_pin(const std::string& pin) const;

    // Constants
    static constexpr int MAX_SESSION_AGE_MINS = 30;
    static constexpr int MAX_PRESETS = 4;
    static constexpr int PIN_LENGTH = 4;
  };

} // namespace subarulink

#endif // SUBARULINK_CONTROLLER_HPP