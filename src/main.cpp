#include "Broadcaster.hpp"
#include "mediasoupclient.hpp"
#include <cpr/cpr.h>
#include <cstdlib>
#include <iostream>

using json = nlohmann::json;

static Broadcaster broadcaster;

void signalHandler(int signum)
{
	std::cout << "Interrupt signal (" << signum << ") received" << std::endl;

	// Remove broadcaster from the server.
	broadcaster.Stop();

	exit(signum);
}

int main(int argc, char* argv[])
{
	// Register signal SIGINT and signal handler.
	signal(SIGINT, signalHandler);

	// Retrieve configuration from environment variables.
	const char* serverUrl = std::getenv("SERVER_URL");
	const char* roomId    = std::getenv("ROOM_ID");

	if (serverUrl == nullptr)
	{
		std::cout << "missing 'SERVER_URL' environment variable" << std::endl;

		return 1;
	}

	if (roomId == nullptr)
	{
		std::cout << "missing 'ROOM_ID' environment variable" << std::endl;

		return 1;
	}

	std::string baseUrl = serverUrl;
	baseUrl.append("/rooms/").append(roomId);

	// Set RTC logging severity to warning.
	// rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_INFO);

	auto logLevel = mediasoupclient::Logger::LogLevel::LOG_DEBUG;
	mediasoupclient::Logger::SetLogLevel(logLevel);
	mediasoupclient::Logger::SetDefaultHandler();

	// Initilize mediasoupclient.
	mediasoupclient::PeerConnection::ClassInit();

	std::cout << ">>> welcome to mediasoup broadcaster app!\n" << std::endl;

	std::cout << ">>> verifying that room '" << roomId << "' exists..." << std::endl;
	auto r = cpr::GetAsync(cpr::Url{ baseUrl }).get();

	if (r.status_code != 200)
	{
		std::cout << "unable to retrieve room info"
		          << ". status code: " << r.status_code << ". body: " << r.text << std::endl;

		return 1;
	}

	auto response = nlohmann::json::parse(r.text);

	auto it = response.find("rtpCapabilities");
	if (it == response.end())
	{
		std::cout << "'routerRtpCapabilities' missing in response" << std::endl;

		return 1;
	}

	broadcaster.Start(baseUrl, *it);

	std::cout << "thanks for flying libmediasoup broadcaster!" << std::endl;

	return 0;
}
