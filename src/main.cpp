#include "Broadcaster.hpp"
#include "mediasoupclient.hpp"
#include <cpr/cpr.h>
#include <cstdlib>
#include <iostream>
#include <csignal> // sigsuspend()
#include <string>

using json = nlohmann::json;

static Broadcaster broadcaster;

void signalHandler(int signum)
{
	std::cout << "[INFO] interrupt signal (" << signum << ") received" << std::endl;

	// Remove broadcaster from the server.
	broadcaster.Stop();

	std::cout << "[INFO] leaving!" << std::endl;

	std::exit(signum);
}

int main(int argc, char* argv[])
{
	// Register signal SIGINT and signal handler.
	signal(SIGINT, signalHandler);

	// Retrieve configuration from environment variables.
	const char* serverUrl   = std::getenv("SERVER_URL");
	const char* roomId      = std::getenv("ROOM_ID");
	const char* webrtcDebug = std::getenv("WEBRTC_DEBUG");

	if (serverUrl == nullptr)
	{
		std::cerr << "[ERROR] missing 'SERVER_URL' environment variable" << std::endl;

		return 1;
	}

	if (roomId == nullptr)
	{
		std::cerr << "[ERROR] missing 'ROOM_ID' environment variable" << std::endl;

		return 1;
	}

	std::string baseUrl = serverUrl;
	baseUrl.append("/rooms/").append(roomId);

	// Set RTC logging severity.
	if (webrtcDebug && std::string(webrtcDebug) == "info")
		rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_INFO);
	else if (webrtcDebug && std::string(webrtcDebug) == "warn")
		rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_WARNING);
	else if (webrtcDebug && std::string(webrtcDebug) == "error")
		rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_ERROR);

	auto logLevel = mediasoupclient::Logger::LogLevel::LOG_DEBUG;
	mediasoupclient::Logger::SetLogLevel(logLevel);
	mediasoupclient::Logger::SetDefaultHandler();

	// Initilize mediasoupclient.
	mediasoupclient::Initialize();

	std::cout << "[INFO] welcome to mediasoup broadcaster app!\n" << std::endl;

	std::cout << "[INFO] verifying that room '" << roomId << "' exists..." << std::endl;
	auto r = cpr::GetAsync(cpr::Url{ baseUrl }).get();

	if (r.status_code != 200)
	{
		std::cerr << "[ERROR] unable to retrieve room info"
		          << " [status code:" << r.status_code << ", body:\""
		          << r.text << "\"]" << std::endl;

		return 1;
	}

	auto response = nlohmann::json::parse(r.text);

	broadcaster.Start(baseUrl, response);

	std::cout << "[INFO] press Ctrl+C or Cmd+C to leave...";

	(void)sigsuspend(nullptr);

	return 0;
}
