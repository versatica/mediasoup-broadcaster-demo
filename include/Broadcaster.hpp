#ifndef BROADCASTER_H
#define BROADCASTER_H

#include "mediasoupclient.hpp"
#include "json.hpp"
#include <future>
#include <string>

class Broadcaster : public mediasoupclient::SendTransport::Listener,
                    mediasoupclient::Producer::PublicListener
{
	/* Virtual methods inherited from SendTransport::Listener. */
public:
	std::future<void> OnConnect(
	  mediasoupclient::Transport* transport, const nlohmann::json& transportLocalParameters);
	void OnConnectionStateChange(mediasoupclient::Transport* transport, const std::string& connectionState);
	std::future<std::string> OnProduce(
	  const std::string& kind, nlohmann::json rtpParameters, const nlohmann::json& appData);

	/* Virtual methods inherited from Producer::PublicListener. */
public:
	void OnTransportClose(mediasoupclient::Producer* producer);

public:
	void Start(const std::string& baseUrl, const nlohmann::json& routerRtpCapabilities);
	void Stop();

private:
	mediasoupclient::Device device;

	std::string id = std::to_string(rtc::CreateRandomId());
	std::string transportId;
	std::string baseUrl;
};

#endif // STOKER_HPP
