#ifndef BROADCASTER_H
#define BROADCASTER_H

#include "mediasoupclient.hpp"
#include "json.hpp"
#include <future>
#include <string>

class Broadcaster : public mediasoupclient::SendTransport::Listener,
                    mediasoupclient::Producer::Listener
{
	/* Virtual methods inherited from SendTransport::Listener. */
public:
	std::future<void> OnConnect(
	  mediasoupclient::Transport* transport, const nlohmann::json& dtlsParameters) override;
	void OnConnectionStateChange(mediasoupclient::Transport* transport, const std::string& connectionState) override;
	std::future<std::string> OnProduce(
	  mediasoupclient::SendTransport* /*transport*/,
	  const std::string& kind,
	  nlohmann::json rtpParameters,
	  const nlohmann::json& appData) override;

	/* Virtual methods inherited from Producer::Listener. */
public:
	void OnTransportClose(mediasoupclient::Producer* producer) override;

public:
	void Start(
	  const std::string& baseUrl,
	  bool enableAudio,
	  bool useSimulcast,
	  const nlohmann::json& routerRtpCapabilities);
	void Stop();

private:
	mediasoupclient::Device device;
	mediasoupclient::SendTransport* sendTransport{ nullptr };

	std::string id = std::to_string(rtc::CreateRandomId());
	std::string transportId;
	std::string baseUrl;
};

#endif // STOKER_HPP
