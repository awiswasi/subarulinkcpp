#pragma once
#ifndef SUBARULINK_CONNECTION_HPP
#define SUBARULINK_CONNECTION_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <future>
#include <mutex>

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"
#include "exceptions.h"

namespace subarulink {

  class Connection {
  public:
    // Constructor declaration only - implementation goes in .cpp
    Connection(const std::string& username,
               const std::string& password,
               const std::string& device_id,
               const std::string& device_name,
               const std::string& country);

    // Async methods - implementations in .cpp
    std::future<std::vector<nlohmann::json>> connect();
    std::future<bool> validate_session(const std::string& vin);
    std::future<bool> request_auth_code(const std::string& contact_method);
    std::future<bool> submit_auth_code(const std::string& code, bool make_permanent = true);

    // Simple inline getters can stay in header
    bool device_registered() const { return _registered; }
    const std::map<std::string, std::string>& auth_contact_methods() const { return _auth_contact_options; }

    // Non-inline methods - implementations in .cpp
    double get_session_age() const;
    void reset_session();

    // HTTP methods - implementations in .cpp
    std::future<nlohmann::json> get(const std::string& url,
                                    const std::map<std::string, std::string>& params = {});
    std::future<nlohmann::json> post(const std::string& url,
                                     const std::map<std::string, std::string>& params = {},
                                     const nlohmann::json& json_data = nlohmann::json());

  private:
    // Member variables
    std::string _username;
    std::string _password;
    std::string _device_id;
    std::string _device_name;
    std::string _country;
    std::string _current_vin;

    bool _authenticated{false};
    bool _registered{false};
    double _session_login_time{0.0};

    std::vector<std::string> _list_of_vins;
    std::vector<nlohmann::json> _vehicles;
    std::map<std::string, std::string> _auth_contact_options;
    std::map<std::string, std::string> _headers;

    std::mutex _mutex;
    std::shared_ptr<cpr::Session> _session;

    // Private method declarations - implementations in .cpp
    std::future<bool> _authenticate(const std::string& vin = "");
    std::future<nlohmann::json> _select_vehicle(const std::string& vin);
    std::future<void> _get_vehicle_data();
    std::future<void> _get_contact_methods();

    // HTTP request helper - implementation in .cpp
    std::future<nlohmann::json> _make_request(
        const std::string& url,
        const std::string& method,
        const std::map<std::string, std::string>& headers = {},
        const std::map<std::string, std::string>& params = {},
        const std::map<std::string, std::string>& data = {},
        const nlohmann::json& json_data = nlohmann::json(),
        const std::string& baseurl = "");

    // Constants
    static const std::string API_VERSION;
    static const std::map<std::string, std::string> API_SERVER;
  };

} // namespace subarulink

#endif // SUBARULINK_CONNECTION_HPP
