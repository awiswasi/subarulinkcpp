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

/**
 * @brief Structure containing comprehensive vehicle information and status
 */
  struct VehicleInfo {
    std::string model_year;              ///< Vehicle model year
    std::string model_name;              ///< Vehicle model name
    std::string vehicle_name;            ///< User-defined vehicle name
    std::vector <std::string> vehicle_features;        ///< List of vehicle features
    std::vector <std::string> subscription_features;   ///< List of active subscription features
    std::string subscription_status;     ///< Current subscription status
    std::map <std::string, nlohmann::json> vehicle_status;  ///< Current vehicle status information
    std::map <std::string, nlohmann::json> vehicle_health;  ///< Vehicle health information
    std::vector <nlohmann::json> climate;  ///< Climate control presets
    std::chrono::system_clock::time_point last_fetch;  ///< Timestamp of last data fetch
    std::chrono::system_clock::time_point last_update;  ///< Timestamp of last update
  };

/**
 * @brief Main controller class for interacting with Subaru STARLINK services
 */
  class Controller {
  public:
    /**
     * @brief Constructs a new Controller object
     * @param username STARLINK account username/email
     * @param password STARLINK account password
     * @param device_id Unique device identifier
     * @param pin STARLINK security PIN
     * @param device_name Name to identify this device
     * @param country Country code ("USA" or "CAN")
     * @param update_interval Time in seconds between updates (default: 7200)
     * @param fetch_interval Time in seconds between fetches (default: 300)
     */
    Controller(const std::string &username,
               const std::string &password,
               const std::string &device_id,
               const std::string &pin,
               const std::string &device_name,
               const std::string &country = "USA",
               int update_interval = 7200,
               int fetch_interval = 300);

    /**
     * @brief Establishes connection with STARLINK services
     * @return Future containing success status
     */
    std::future<bool> connect();

    /**
     * @brief Checks if device is registered with STARLINK
     * @return True if device is registered, false otherwise
     */
    bool device_registered() const;

    /**
     * @brief Gets available 2FA contact methods
     * @return Map of contact method IDs to contact information
     */
    const std::map <std::string, std::string> &contact_methods() const;

    /**
     * @brief Requests an authentication code for 2FA
     * @param contact_method Contact method ID to use
     * @return Future containing success status
     */
    std::future<bool> request_auth_code(const std::string &contact_method);

    /**
     * @brief Submits 2FA authentication code
     * @param code The verification code received
     * @return Future containing success status
     */
    std::future<bool> submit_auth_code(const std::string &code);

    /**
     * @brief Checks if PIN is required for operations
     * @return True if PIN is required, false otherwise
     */
    bool is_pin_required() const;

    /**
     * @brief Tests if current PIN is valid
     * @return Future containing validation status
     */
    std::future<bool> test_pin();

    /**
     * @brief Gets list of vehicles associated with account
     * @return Vector of VIN numbers
     */
    std::vector <std::string> get_vehicles() const;

    // Vehicle Information Methods

    /**
     * @brief Gets vehicle model year
     * @param vin Vehicle identification number
     * @return Model year as string
     * @throws SubaruException if VIN is invalid
     */
    std::string get_model_year(const std::string &vin) const;

    /**
     * @brief Gets vehicle model name
     * @param vin Vehicle identification number
     * @return Model name
     * @throws SubaruException if VIN is invalid
     */
    std::string get_model_name(const std::string &vin) const;

    /**
     * @brief Checks if vehicle is electric/PHEV
     * @param vin Vehicle identification number
     * @return True if vehicle is EV/PHEV, false otherwise
     * @throws SubaruException if VIN is invalid
     */
    bool get_ev_status(const std::string &vin) const;

    /**
     * @brief Checks if vehicle has active remote services
     * @param vin Vehicle identification number
     * @return True if remote services are active
     * @throws SubaruException if VIN is invalid
     */
    bool get_remote_status(const std::string &vin) const;

    /**
     * @brief Checks if vehicle has remote start capability
     * @param vin Vehicle identification number
     * @return True if remote start is available
     * @throws SubaruException if VIN is invalid
     */
    bool get_res_status(const std::string &vin) const;

    /**
     * @brief Checks if vehicle has power windows
     * @param vin Vehicle identification number
     * @return Future containing power windows status
     */
    std::future<bool> has_power_windows(const std::string &vin);

    /**
     * @brief Checks if vehicle has a sunroof
     * @param vin Vehicle identification number
     * @return True if vehicle has sunroof
     * @throws SubaruException if VIN is invalid
     */
    bool has_sunroof(const std::string &vin) const;

    /**
     * @brief Checks if vehicle reports lock status
     * @param vin Vehicle identification number
     * @return Future containing lock status reporting capability
     */
    std::future<bool> has_lock_status(const std::string &vin);

    /**
     * @brief Checks if vehicle has TPMS
     * @param vin Vehicle identification number
     * @return True if vehicle has TPMS
     * @throws SubaruException if VIN is invalid
     */
    bool has_tpms(const std::string &vin) const;

    /**
     * @brief Checks vehicle safety service status
     * @param vin Vehicle identification number
     * @return True if safety services are active
     * @throws SubaruException if VIN is invalid
     */
    bool get_safety_status(const std::string &vin) const;

    /**
     * @brief Gets subscription status
     * @param vin Vehicle identification number
     * @return True if subscription is active
     * @throws SubaruException if VIN is invalid
     */
    bool get_subscription_status(const std::string &vin) const;

    /**
     * @brief Gets API generation for vehicle
     * @param vin Vehicle identification number
     * @return API generation string ("g1", "g2", or "g3")
     * @throws SubaruException if VIN is invalid
     */
    std::string get_api_gen(const std::string &vin) const;

    /**
     * @brief Gets user-defined vehicle name
     * @param vin Vehicle identification number
     * @return Vehicle name
     * @throws SubaruException if VIN is invalid
     */
    std::string vin_to_name(const std::string &vin) const;

    // Data Retrieval Methods

    /**
     * @brief Gets comprehensive vehicle data
     * @param vin Vehicle identification number
     * @return Future containing VehicleInfo structure
     */
    std::future <VehicleInfo> get_data(const std::string &vin);

    /**
     * @brief Gets raw API response data
     * @param vin Vehicle identification number
     * @return Raw JSON data from API
     * @throws SubaruException if VIN is invalid
     */
    nlohmann::json get_raw_data(const std::string &vin) const;

    /**
     * @brief Lists available climate control presets
     * @param vin Vehicle identification number
     * @return Future containing vector of preset names
     */
    std::future <std::vector<std::string>> list_climate_preset_names(const std::string &vin);

    /**
     * @brief Gets climate preset by name
     * @param vin Vehicle identification number
     * @param preset_name Name of the preset to retrieve
     * @return Future containing preset data as JSON
     */
    std::future <nlohmann::json> get_climate_preset_by_name(const std::string &vin, const std::string &preset_name);

    /**
     * @brief Gets all user-defined climate presets
     * @param vin Vehicle identification number
     * @return Future containing vector of preset data
     */
    std::future <std::vector<nlohmann::json>> get_user_climate_preset_data(const std::string &vin);

    // Climate Control Methods

    /**
     * @brief Deletes a climate preset
     * @param vin Vehicle identification number
     * @param preset_name Name of preset to delete
     * @return Future containing success status
     */
    std::future<bool> delete_climate_preset_by_name(const std::string &vin, const std::string &preset_name);

    /**
     * @brief Updates user climate presets
     * @param vin Vehicle identification number
     * @param preset_data Vector of preset configurations
     * @return Future containing success status
     */
    std::future<bool>
    update_user_climate_presets(const std::string &vin, const std::vector <nlohmann::json> &preset_data);

    // Data Update Methods

    /**
     * @brief Fetches latest vehicle data
     * @param vin Vehicle identification number
     * @param force Force fetch regardless of cache
     * @return Future containing success status
     */
    std::future<bool> fetch(const std::string &vin, bool force = false);

    /**
     * @brief Updates vehicle location
     * @param vin Vehicle identification number
     * @param force Force update regardless of cache
     * @return Future containing success status
     */
    std::future<bool> update(const std::string &vin, bool force = false);

    // Interval Management

    /**
     * @brief Gets update interval
     * @return Update interval in seconds
     */
    int get_update_interval() const;

    /**
     * @brief Sets update interval
     * @param value New interval in seconds
     * @return True if value was accepted
     */
    bool set_update_interval(int value);

    /**
     * @brief Gets fetch interval
     * @return Fetch interval in seconds
     */
    int get_fetch_interval() const;

    /**
     * @brief Sets fetch interval
     * @param value New interval in seconds
     * @return True if value was accepted
     */
    bool set_fetch_interval(int value);

    // Time Related Methods

    /**
     * @brief Gets timestamp of last fetch
     * @param vin Vehicle identification number
     * @return Timestamp of last fetch
     * @throws SubaruException if VIN is invalid
     */
    std::chrono::system_clock::time_point get_last_fetch_time(const std::string &vin) const;

    /**
     * @brief Gets timestamp of last update
     * @param vin Vehicle identification number
     * @return Timestamp of last update
     * @throws SubaruException if VIN is invalid
     */
    std::chrono::system_clock::time_point get_last_update_time(const std::string &vin) const;

    // Vehicle Control Methods

    /**
     * @brief Starts EV charging
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> charge_start(const std::string &vin);

    /**
     * @brief Locks vehicle
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> lock(const std::string &vin);

    /**
     * @brief Unlocks vehicle
     * @param vin Vehicle identification number
     * @param door Door to unlock ("ALL_DOORS_CMD", "FRONT_LEFT_DOOR_CMD", or "TAILGATE_DOOR_CMD")
     * @return Future containing success status
     */
    std::future<bool> unlock(const std::string &vin, const std::string &door = "ALL_DOORS_CMD");

    /**
     * @brief Activates exterior lights
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> lights(const std::string &vin);

    /**
     * @brief Deactivates exterior lights
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> lights_stop(const std::string &vin);

    /**
     * @brief Activates horn
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> horn(const std::string &vin);

    /**
     * @brief Deactivates horn
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> horn_stop(const std::string &vin);

    /**
     * @brief Stops remote engine
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> remote_stop(const std::string &vin);

    /**
     * @brief Starts remote engine with climate preset
     * @param vin Vehicle identification number
     * @param preset_name Climate preset to use
     * @return Future containing success status
     */
    std::future<bool> remote_start(const std::string &vin, const std::string &preset_name);

    // PIN Management

    /**
     * @brief Checks if PIN has been invalidated
     * @return True if PIN is invalid
     */
    bool invalid_pin_entered() const;

    /**
     * @brief Updates stored PIN
     * @param new_pin New PIN to use
     * @return True if PIN was updated
     */
    bool update_saved_pin(const std::string &new_pin);

  private:
    std::unique_ptr <Connection> _connection;    ///< Connection handler
    std::string _country;                       ///< Country code
    int _update_interval;                       ///< Update interval in seconds
    int _fetch_interval;                        ///< Fetch interval in seconds
    std::map <std::string, VehicleInfo> _vehicles;  ///< Vehicle information cache
    std::map <std::string, std::unique_ptr<std::mutex>> _vehicle_mutex;  ///< Per-vehicle mutex
    std::string _pin;                           ///< STARLINK security PIN
    std::mutex _controller_mutex;               ///< Controller-wide mutex
    bool _pin_lockout;                          ///< PIN lockout status
    std::map <std::string, nlohmann::json> _raw_api_data;  ///< Raw API response cache
    std::string version;                        ///< API version

    // Constants
    static constexpr int MAX_SESSION_AGE_MINS = 30;  ///< Maximum session age in minutes
    static constexpr int MAX_PRESETS = 4;            ///< Maximum number of climate presets
    static constexpr int PIN_LENGTH = 4;             ///< Required PIN length

    /**
     * @brief Makes GET request to API
     * @param url API endpoint URL
     * @param params Query parameters
     * @return Future containing JSON response
     */
    std::future <nlohmann::json> _get(const std::string &url, const std::map <std::string, std::string> &params = {});

    /**
     * @brief Makes POST request to API
     * @param url API endpoint URL
     * @param params Query parameters
     * @param json_data JSON request body
     * @return Future containing JSON response
     */
    std::future <nlohmann::json> _post(const std::string &url,
                                       const std::map <std::string, std::string> &params = {},
                                       const nlohmann::json &json_data = nlohmann::json());

    /**
     * @brief Checks API response for error codes
     * @param js_resp JSON response to check
     * @throws InvalidPIN if PIN error detected
     * @throws SubaruException for other errors
     */
    void _check_error_code(const nlohmann::json &js_resp);

    /**
     * @brief Parses vehicle information from API response
     * @param vehicle JSON vehicle data
     */
    void _parse_vehicle(const nlohmann::json &vehicle);

    /**
     * @brief Executes remote command with retry logic
     * @param vin Vehicle identification number
     * @param cmd Command to execute
     * @param poll_url Status polling endpoint
     * @param data Additional command data
     * @return Future containing tuple of success status and response
     */
    std::future <std::tuple<bool, nlohmann::json>> _remote_command(
        const std::string &vin,
        const std::string &cmd,
        const std::string &poll_url,
        const nlohmann::json &data = nlohmann::json());

    /**
     * @brief Executes vehicle actuation command
     * @param vin Vehicle identification number
     * @param cmd Command to execute
     * @param data Command parameters
     * @param poll_url Status polling endpoint
     * @return Future containing tuple of success status and response
     */
    std::future <std::tuple<bool, nlohmann::json>> _actuate(
        const std::string &vin,
        const std::string &cmd,
        const nlohmann::json &data = nlohmann::json(),
        const std::string &poll_url = "/g2v30/remoteService/status.json");

    /**
     * @brief Retrieves vehicle status from API
     * @param vin Vehicle identification number
     * @return Future containing JSON status data
     */
    std::future <nlohmann::json> _get_vehicle_status(const std::string &vin);

    /**
     * @brief Updates vehicle status data
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> _fetch_status(const std::string &vin);

    /**
     * @brief Updates vehicle location
     * @param vin Vehicle identification number
     * @param hard_poll Force real-time location update
     * @return Future containing success status
     */
    std::future<bool> _locate(const std::string &vin, bool hard_poll = false);

    /**
     * @brief Parses location data from API response
     * @param vin Vehicle identification number
     * @param result JSON location data
     */
    void _parse_location(const std::string &vin, const nlohmann::json &result);

    /**
     * @brief Polls for command completion status
     * @param vin Vehicle identification number
     * @param req_id Request ID to poll
     * @param poll_url Status polling endpoint
     * @param attempts Maximum number of polling attempts
     * @return Future containing tuple of success status and response
     */
    std::future <std::tuple<bool, nlohmann::json>> _wait_request_status(
        const std::string &vin,
        const std::string &req_id,
        const std::string &poll_url,
        int attempts = 20);

    /**
     * @brief Retrieves climate presets from API
     * @param vin Vehicle identification number
     * @return Future containing success status
     */
    std::future<bool> _fetch_climate_presets(const std::string &vin);

    /**
     * @brief Validates remote start parameters
     * @param vin Vehicle identification number
     * @param preset_data Preset configuration to validate
     * @return True if parameters are valid
     */
    bool _validate_remote_start_params(const std::string &vin, const nlohmann::json &preset_data);

    /**
     * @brief Checks if vehicle supports remote capabilities
     * @param vin Vehicle identification number
     * @return True if remote capabilities are supported
     */
    bool _validate_remote_capability(const std::string &vin);

    /**
     * @brief Parses vehicle status from API response
     * @param js_resp JSON response data
     * @param vin Vehicle identification number
     * @return Parsed vehicle status data
     */
    nlohmann::json _parse_vehicle_status(const nlohmann::json &js_resp, const std::string &vin);

    /**
     * @brief Parses vehicle condition data
     * @param js_resp JSON response data
     * @param vin Vehicle identification number
     * @return Future containing parsed condition data
     */
    std::future <nlohmann::json> _parse_condition(const nlohmann::json &js_resp, const std::string &vin);

    /**
     * @brief Parses vehicle health data
     * @param js_resp JSON response data
     * @param vin Vehicle identification number
     * @return Parsed health data
     */
    nlohmann::json _parse_health(const nlohmann::json &js_resp, const std::string &vin);

    /**
     * @brief Gets recommended tire pressures
     * @param vin Vehicle identification number
     * @return Recommended tire pressure data
     */
    nlohmann::json _parse_recommended_tire_pressure(const std::string &vin);

    /**
     * @brief Makes query to remote service API
     * @param vin Vehicle identification number
     * @param cmd Command to execute
     * @return Future containing JSON response
     */
    std::future <nlohmann::json> _remote_query(const std::string &vin, const std::string &cmd);

    /**
     * @brief Executes remote command and handles retries
     * @param vin Vehicle identification number
     * @param cmd Command to execute
     * @param data Command parameters
     * @param poll_url Status polling endpoint
     * @return Future containing tuple of retry flag, success status, and response
     */
    std::future <std::tuple<bool, bool, nlohmann::json>> _execute_remote_command(
        const std::string &vin,
        const std::string &cmd,
        const nlohmann::json &data,
        const std::string &poll_url);

    /**
     * @brief Checks if PIN is in lockout state
     * @throws PINLockoutProtect if PIN is locked out
     */
    void _check_pin_lockout() const;

    /**
     * @brief Validates VIN format
     * @param vin VIN to validate
     * @throws SubaruException if VIN is invalid
     */
    void _validate_vin(const std::string &vin) const;

    /**
     * @brief Validates PIN format
     * @param pin PIN to validate
     * @throws InvalidPIN if PIN format is invalid
     */
    void _validate_pin(const std::string &pin) const;
  };

} // namespace subarulink

#endif // SUBARULINK_CONTROLLER_HPP
