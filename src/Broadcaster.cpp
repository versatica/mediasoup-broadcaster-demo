#include "Broadcaster.hpp"
#include "mediasoupclient.hpp"
#include "peerConnectionUtils.hpp"
#include "json.hpp"
#include <cpr/cpr.h>
#include <cstdlib>
#include <iostream>
#include <string>

using json = nlohmann::json;

void Broadcaster::OnTransportClose(mediasoupclient::Producer* /*producer*/)
{
	std::cout << "[INFO] Broadcaster::OnTransportClose()" << std::endl;
}

/* Transport::Listener::OnConnect
 *
 * Fired for the first Transport::Consume() or Transport::Produce().
 * Update the already created remote transport with the local DTLS parameters.
 */
std::future<void> Broadcaster::OnConnect(
  mediasoupclient::Transport* /*transport*/, const json& dtlsParameters)
{
	std::cout << "[INFO] Broadcaster::OnConnect()" << std::endl;
	// std::cout << "[INFO] dtlsParameters: " << dtlsParameters.dump(4) << std::endl;

	std::promise<void> promise;

	/* clang-format off */
	json body =
	{
		{ "dtlsParameters", dtlsParameters }
	};
	/* clang-format on */

	auto r = cpr::PostAsync(
	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
	                     this->transportId + "/connect" },
	           cpr::Body{ body.dump() },
	           cpr::Header{ { "Content-Type", "application/json" } })
	           .get();

	if (r.status_code == 200)
	{
		promise.set_value();
	}
	else
	{
		std::cerr << "[ERROR] unable to connect transport"
		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;

		promise.set_exception(std::make_exception_ptr(r.text));
	}

	return promise.get_future();
}

/*
 * Transport::Listener::OnConnectionStateChange.
 */
void Broadcaster::OnConnectionStateChange(
  mediasoupclient::Transport* transport, const std::string& connectionState)
{
	std::cout << "[INFO] Broadcaster::OnConnectionStateChange() [connectionState:" << connectionState
	          << "]" << std::endl;

	if (connectionState == "failed")
	{
		Stop();
		std::exit(0);
	}
}

/* Producer::Listener::OnProduce
 *
 * Fired when a producer needs to be created in mediasoup.
 * Retrieve the remote producer ID and feed the caller with it.
 */
std::future<std::string> Broadcaster::OnProduce(
  mediasoupclient::SendTransport* /*transport*/,
  const std::string& kind,
  json rtpParameters,
  const json& /*appData*/)
{
	std::cout << "[INFO] Broadcaster::OnProduce()" << std::endl;
	// std::cout << "[INFO] rtpParameters: " << rtpParameters.dump(4) << std::endl;

	std::promise<std::string> promise;

	/* clang-format off */
	json body =
	{
		{ "kind",          kind          },
		{ "rtpParameters", rtpParameters }
	};
	/* clang-format on */

	auto r = cpr::PostAsync(
	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
	                     this->transportId + "/producers" },
	           cpr::Body{ body.dump() },
	           cpr::Header{ { "Content-Type", "application/json" } })
	           .get();

	if (r.status_code == 200)
	{
		auto response = json::parse(r.text);

		auto it = response.find("id");
		if (it == response.end() || !it->is_string())
			promise.set_exception(std::make_exception_ptr("'id' missing in response"));

		promise.set_value((*it).get<std::string>());
	}
	else
	{
		std::cerr << "[ERROR] unable to create producer"
		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;

		promise.set_exception(std::make_exception_ptr(r.text));
	}

	return promise.get_future();
}

void Broadcaster::Start(
  const std::string& baseUrl, bool enableAudio, bool useSimulcast, const json& routerRtpCapabilities)
{
	std::cout << "[INFO] Broadcaster::Start()" << std::endl;

	this->baseUrl = baseUrl;

	// Load the device.
	this->device.Load(routerRtpCapabilities);

	std::cout << "[INFO] creating Broadcaster..." << std::endl;

	/* clang-format off */
	json body =
	{
		{ "id",          this->id          },
		{ "displayName", "broadcaster"     },
		{ "device",
			{
				{ "name",    "libmediasoupclient"       },
				{ "version", mediasoupclient::Version() }
			}
		}
	};
	/* clang-format on */

	auto r = cpr::PostAsync(
	           cpr::Url{ this->baseUrl + "/broadcasters" },
	           cpr::Body{ body.dump() },
	           cpr::Header{ { "Content-Type", "application/json" } })
	           .get();

	if (r.status_code != 200)
	{
		std::cerr << "[ERROR] unable to create Broadcaster"
		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;

		return;
	}

	std::cout << "[INFO] creating mediasoup WebRtcTransport..." << std::endl;

	/* clang-format off */
	body =
	{
		{ "type",    "webrtc" },
		{ "rtcpMux", true     }
	};
	/* clang-format on */

	r = cpr::PostAsync(
	      cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports" },
	      cpr::Body{ body.dump() },
	      cpr::Header{ { "Content-Type", "application/json" } })
	      .get();

	if (r.status_code != 200)
	{
		std::cerr << "[ERROR] unable to create mediasoup WebRtcTransport"
		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;

		return;
	}

	auto response = json::parse(r.text);

	if (response.find("id") == response.end())
	{
		std::cerr << "[ERROR] 'id' missing in response" << std::endl;

		return;
	}
	else if (response.find("iceParameters") == response.end())
	{
		std::cerr << "[ERROR] 'iceParametersd' missing in response" << std::endl;

		return;
	}
	else if (response.find("iceCandidates") == response.end())
	{
		std::cerr << "[ERROR] 'iceCandidates' missing in response" << std::endl;

		return;
	}
	else if (response.find("dtlsParameters") == response.end())
	{
		std::cerr << "[ERROR] 'dtlsParameters' missing in response" << std::endl;

		return;
	}

	std::cout << "[INFO] creating SendTransport..." << std::endl;

	this->transportId = response["id"].get<std::string>();

	auto sendTransport = this->device.CreateSendTransport(
	  this,
	  this->transportId,
	  response["iceParameters"],
	  response["iceCandidates"],
	  response["dtlsParameters"]);

	///////////////////////// Create Audio Producer //////////////////////////

	if (enableAudio && this->device.CanProduce("audio"))
	{
		auto audioTrack = createAudioTrack(std::to_string(rtc::CreateRandomId()));

		/* clang-format off */
		json codecOptions = {
			{ "opusStereo", true },
			{ "opusDtx",		true }
		};
		/* clang-format on */

		sendTransport->Produce(this, audioTrack, nullptr, &codecOptions);
	}
	else
	{
		std::cerr << "[WARN] cannot produce audio" << std::endl;
	}

	///////////////////////// Create Video Producer //////////////////////////

	if (this->device.CanProduce("video"))
	{
		auto videoTrack = createVideoTrack(std::to_string(rtc::CreateRandomId()));

		if (useSimulcast)
		{
			std::vector<webrtc::RtpEncodingParameters> encodings;
			encodings.emplace_back(webrtc::RtpEncodingParameters());
			encodings.emplace_back(webrtc::RtpEncodingParameters());
			encodings.emplace_back(webrtc::RtpEncodingParameters());

			sendTransport->Produce(this, videoTrack, &encodings, nullptr);
		}
		else
		{
			sendTransport->Produce(this, videoTrack, nullptr, nullptr);
		}
	}
	else
	{
		std::cerr << "[WARN] cannot produce video" << std::endl;
	}
}

void Broadcaster::Stop()
{
	std::cout << "[INFO] Broadcaster::Stop()" << std::endl;

	cpr::DeleteAsync(cpr::Url{ this->baseUrl + "/broadcasters/" + this->id }).get();
}
