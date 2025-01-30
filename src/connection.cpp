#include <chrono>
#include <thread>
#include <algorithm>
#include <iostream>

#include "connection.h"
#include "exceptions.h"

namespace subarulink {

  const std::string Connection::API_VERSION = "/g2v30";
  const std::map<std::string, std::string> Connection::API_SERVER = {
      {"USA", "mobileapi.prod.subarucs.com"},
      {"CAN", "mobileapi.ca.prod.subarucs.com"}
  };

  Connection::Connection(const std::string& username,
                         const std::string& password,
                         const std::string& device_id,
                         const std::string& device_name,
                         const std::string& country)
      : _username(username),
        _password(password),
        _device_id(device_id),
        _device_name(device_name),
        _country(country),
        _authenticated(false),
        _registered(false),
        _session_login_time(0.0) {

    _session = std::make_shared<cpr::Session>();

    // Create API_MOBILE_APP map
    std::map<std::string, std::string> API_MOBILE_APP = {
        {"USA", "com.subaru.telematics.app.remote"},
        {"CAN", "ca.subaru.telematics.remote"}
    };

    // Set headers
    _headers = {
        {"User-Agent", "Mozilla/5.0 (Linux; Android 10; Android SDK built for x86 Build/QSR1.191030.002; wv) "
                       "AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/74.0.3729.185 Mobile Safari/537.36"},
        {"Origin", "file://"},
        {"X-Requested-With", API_MOBILE_APP.at(_country)},
        {"Accept-Language", "en-US,en;q=0.9"},
        {"Accept-Encoding", "gzip, deflate"},
        {"Accept", "*/*"}
    };

    _session->SetHeader(cpr::Header(_headers.begin(), _headers.end()));
  }

  std::future<std::vector<nlohmann::json>> Connection::connect() {
    return std::async(std::launch::async, [this]() {
      auto auth_result = _authenticate().get();
      if (!auth_result) {
        throw SubaruException("Authentication failed");
      }

      _get_vehicle_data().get();

      if (!device_registered()) {
        _get_contact_methods().get();
      }

      return _vehicles;
    });
  }

  std::future<bool> Connection::_authenticate(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      if (_username.empty() || _password.empty() || _device_id.empty()) {
        throw IncompleteCredentials("Connection requires email, password and device id.");
      }

      std::cout << "Debug: Starting authentication flow" << std::endl;
      std::cout << "Debug: device_id being used: " << _device_id << std::endl;

      // Create form data
      std::map<std::string, std::string> form_data = {
          {"env", "cloudprod"},
          {"loginUsername", _username},
          {"password", _password},
          {"deviceId", _device_id},
          {"passwordToken", ""},
          {"selectedVin", vin},
          {"pushToken", ""},
          {"deviceType", "android"}
      };

      try {
        std::string endpoint = "https://" + API_SERVER.at(_country) + API_VERSION + "/login.json";
        std::cout << "Debug: Making authentication request to: " << endpoint << std::endl;

        auto response = _make_request("/login.json", "POST", _headers, {}, form_data, nlohmann::json()).get();
        std::cout << "Debug: Full response received: " << response.dump(2) << std::endl;

        if (response["success"].get<bool>()) {
          std::cout << "Debug: Authentication successful" << std::endl;
          _authenticated = true;
          _session_login_time = std::chrono::system_clock::now().time_since_epoch().count() / 1000.0;
          _registered = response["data"]["deviceRegistered"].get<bool>();

          _list_of_vins.clear();
          if (response["data"].contains("vehicles")) {
            for (const auto& vehicle : response["data"]["vehicles"]) {
              _list_of_vins.push_back(vehicle["vin"].get<std::string>());
            }
          }
          _current_vin = "";
          return true;
        }

        if (response.contains("errorCode")) {
          std::string error = response["errorCode"].get<std::string>();
          std::cout << "Debug: Authentication failed with error: " << error << std::endl;

          if (error == "InvalidAccount" || error == "InvalidCredentials") {
            throw InvalidCredentials(error);
          }
          throw SubaruException(error);
        }

        throw SubaruException("Unexpected response format");
      } catch (const std::exception& e) {
        std::cout << "Debug: Error during authentication: " << e.what() << std::endl;
        throw;
      }
    });
  }

  std::future<bool> Connection::validate_session(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      auto response = _make_request("/validateSession.json", "GET").get();

      if (response["success"].get<bool>()) {
        if (vin != _current_vin) {
          return _select_vehicle(vin).get().contains("success");
        }
        return true;
      }

      _authenticate(vin).get();
      return _select_vehicle(vin).get().contains("success");
    });
  }

  std::future<nlohmann::json> Connection::_select_vehicle(const std::string& vin) {
    return std::async(std::launch::async, [this, vin]() {
      std::map<std::string, std::string> params = {
          {"vin", vin},
          {"_", std::to_string(std::time(nullptr))}
      };

      auto response = get("/selectVehicle.json", params).get();

      if (response["success"].get<bool>()) {
        _current_vin = vin;
        return response["data"];
      }

      if (response["errorCode"] == "VEHICLESETUPERROR") {
        reset_session();
        return nlohmann::json{};
      }

      reset_session();
      throw SubaruException("Failed to switch vehicle: " + response["errorCode"].get<std::string>());
    });
  }

  std::future<bool> Connection::request_auth_code(const std::string& contact_method) {
    return std::async(std::launch::async, [this, contact_method]() {
      if (_auth_contact_options.find(contact_method) == _auth_contact_options.end()) {
        return false;
      }

      std::cout << "Debug: Requesting 2FA code" << std::endl;

      std::map<std::string, std::string> form_data = {
          {"contactMethod", contact_method},
          {"languagePreference", "EN"}
      };

      auto response = _make_request("/twoStepAuthSendVerification.json", "POST", {}, {}, form_data, nlohmann::json()).get();
      return response.contains("success") && response["success"].get<bool>();
    });
  }

  std::future<bool> Connection::submit_auth_code(const std::string& code, bool make_permanent) {
    return std::async(std::launch::async, [this, code, make_permanent]() {
      if (code.length() != 6 || !std::all_of(code.begin(), code.end(), ::isdigit)) {
        return false;
      }

      std::map<std::string, std::string> form_data = {
          {"deviceId", _device_id},
          {"deviceName", _device_name},
          {"verificationCode", code}
      };

      if (make_permanent) {
        form_data["rememberDevice"] = "on";
      }

      auto response = _make_request("/twoStepAuthVerify.json", "POST", {}, {}, form_data, nlohmann::json()).get();

      if (response["success"].get<bool>()) {
        while (!_registered) {
          std::this_thread::sleep_for(std::chrono::seconds(3));
          _authenticate().get();
          _current_vin = "";
        }
        return true;
      }
      return false;
    });
  }

  std::future<void> Connection::_get_vehicle_data() {
    return std::async(std::launch::async, [this]() {
      for (const auto& vin : _list_of_vins) {
        std::map<std::string, std::string> params = {
            {"vin", vin},
            {"_", std::to_string(std::time(nullptr))}
        };

        auto response = get("/selectVehicle.json", params).get();
        _vehicles.push_back(response["data"]);
        _current_vin = vin;
      }
    });
  }

  std::future<void> Connection::_get_contact_methods() {
    return std::async(std::launch::async, [this]() {
      auto response = _make_request("/twoStepAuthContacts.json", "POST").get();
      if (response.contains("data")) {
        _auth_contact_options = response["data"].get<std::map<std::string, std::string>>();

        std::cout << "Debug: Available 2FA contact methods:" << std::endl;
        for (const auto& [method, contact] : _auth_contact_options) {
          std::cout << "  " << method << ": " << contact << std::endl;
        }
      }
    });
  }

  std::future<nlohmann::json> Connection::_make_request(
      const std::string& url,
      const std::string& method,
      const std::map<std::string, std::string>& headers,
      const std::map<std::string, std::string>& params,
      const std::map<std::string, std::string>& data,
      const nlohmann::json& json_data,
      const std::string& baseurl) {

    return std::async(std::launch::async, [this, url, method, headers, params, data, json_data, baseurl]() {
      std::string base = baseurl.empty() ?
                         "https://" + API_SERVER.at(_country) + API_VERSION : baseurl;
      std::string endpoint = base + url;

      std::cout << "Debug: Making request to: " << endpoint << std::endl;
      std::cout << "Debug: Method: " << method << std::endl;

      std::lock_guard<std::mutex> lock(_mutex);
      _session->SetUrl(cpr::Url{endpoint});

      // Handle parameters
      if (!params.empty()) {
        cpr::Parameters parameters;
        for (const auto& param : params) {
          parameters.Add({param.first, param.second});
        }
        _session->SetParameters(parameters);
      }

      // Handle headers
      if (!headers.empty()) {
        cpr::Header header;
        for (const auto& h : headers) {
          header.insert({h.first, h.second});
        }
        _session->SetHeader(header);
      }

      cpr::Response response;
      if (method == "POST") {
        if (!data.empty()) {
          std::vector<cpr::Pair> pairs;
          for (const auto& d : data) {
            pairs.push_back({d.first, d.second});
          }
          cpr::Payload payload(pairs.begin(), pairs.end());
          _session->SetPayload(payload);

          std::cout << "Debug: Setting form data:" << std::endl;
          for (const auto& pair : pairs) {
            std::cout << "  " << pair.key << ": " << pair.value << std::endl;
          }
        }
        else if (!json_data.empty()) {
          _session->SetBody(cpr::Body{json_data.dump()});
          std::cout << "Debug: Setting JSON body: " << json_data.dump() << std::endl;
        }

        response = _session->Post();
      } else {
        response = _session->Get();
      }

      std::cout << "Debug: Response status: " << response.status_code << std::endl;
      std::cout << "Debug: Response text: " << response.text.substr(0, 200) << "..." << std::endl;

      if (response.status_code > 299) {
        throw SubaruException("HTTP " + std::to_string(response.status_code) + ": " + response.text);
      }

      auto js_resp = nlohmann::json::parse(response.text);
      if (!js_resp.contains("success") && !js_resp.contains("serviceType")) {
        throw SubaruException("Unexpected response: " + response.text);
      }

      return js_resp;
    });
  }

  std::future<nlohmann::json> Connection::get(const std::string& url,
                                              const std::map<std::string, std::string>& params) {
    return std::async(std::launch::async, [this, url, params]() {
      if (!_authenticated) {
        return nlohmann::json{};
      }
      return _make_request(url, "GET", _headers, params).get();
    });
  }

  std::future<nlohmann::json> Connection::post(const std::string& url,
                                               const std::map<std::string, std::string>& params,
                                               const nlohmann::json& json_data) {
    return std::async(std::launch::async, [this, url, params, json_data]() {
      if (!_authenticated) {
        return nlohmann::json{};
      }
      return _make_request(url, "POST", _headers, params, {}, json_data).get();
    });
  }

  double Connection::get_session_age() const {
    auto current_time = std::chrono::system_clock::now().time_since_epoch().count() / 1000.0;
    return (current_time - _session_login_time) / 60.0;
  }

  void Connection::reset_session() {
    std::lock_guard<std::mutex> lock(_mutex);
    _session = std::make_shared<cpr::Session>();
    _session->SetHeader(cpr::Header(_headers.begin(), _headers.end()));
  }

} // namespace subarulink