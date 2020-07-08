#include "Broadcaster.hpp"
#include "mediasoupclient.hpp"
#include "MediaStreamTrackFactory.hpp"
#include "json.hpp"
#include <cpr/cpr.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <thread>
#include <functional>

using json = nlohmann::json;

Broadcaster::~Broadcaster() 
{
	this->Stop();
}

void Broadcaster::OnTransportClose(mediasoupclient::Producer* /*producer*/)
{
	std::cout << "[INFO] Broadcaster::OnTransportClose()" << std::endl;
}

void Broadcaster::OnTransportClose(mediasoupclient::DataProducer* /*dataProducer*/)
{
	std::cout << "[INFO] Broadcaster::OnTransportClose()" << std::endl;
}

/* Transport::Listener::OnConnect
 *
 * Fired for the first Transport::Consume() or Transport::Produce().
 * Update the already created remote transport with the local DTLS parameters.
 */
std::future<void> Broadcaster::OnConnect(
  mediasoupclient::Transport* transport, const json& dtlsParameters)
{
	std::cout << "[INFO] Broadcaster::OnConnect()" << std::endl;
	// std::cout << "[INFO] dtlsParameters: " << dtlsParameters.dump(4) << std::endl;

	if (transport->GetId() == this->sendTransportId){
		return this->OnConnectSendTransport(dtlsParameters);
	} else if (transport->GetId() == this->recvTransportId) {
		return this->OnConnectRecvTransport(dtlsParameters);
	}

	throw "Unknown transport requested to connect.";
}

std::future<void> Broadcaster::OnConnectSendTransport(const json& dtlsParameters) {
	std::promise<void> promise;

	/* clang-format off */
	json body =
	{
		{ "dtlsParameters", dtlsParameters }
	};
	/* clang-format on */

	auto r = cpr::PostAsync(
	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
	                     this->sendTransportId + "/connect" },
	           cpr::Body{ body.dump() },
	           cpr::Header{ { "Content-Type", "application/json" } },
			   cpr::VerifySsl{verifySsl})
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

std::future<void> Broadcaster::OnConnectRecvTransport(const json& dtlsParameters) {
	std::promise<void> promise;

	/* clang-format off */
	json body =
	{
		{ "dtlsParameters", dtlsParameters }
	};
	/* clang-format on */

	auto r = cpr::PostAsync(
	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
	                     this->recvTransportId + "/connect" },
	           cpr::Body{ body.dump() },
	           cpr::Header{ { "Content-Type", "application/json" } },
			   cpr::VerifySsl{verifySsl})
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
  mediasoupclient::Transport*  /*transport*/, const std::string& connectionState)
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
	                     this->sendTransportId + "/producers" },
	           cpr::Body{ body.dump() },
	           cpr::Header{ { "Content-Type", "application/json" } },
			   cpr::VerifySsl{verifySsl})
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

/* Producer::Listener::OnProduceData
 *
 * Fired when a data producer needs to be created in mediasoup.
 * Retrieve the remote producer ID and feed the caller with it.
 */
std::future<std::string> Broadcaster::OnProduceData(
  mediasoupclient::SendTransport* transport,
  const json& sctpStreamParameters,
  const std::string& label,
  const std::string& protocol,
  const json& appData)
{
	std::cout << "[INFO] Broadcaster::OnProduceData()" << std::endl;
	// std::cout << "[INFO] rtpParameters: " << rtpParameters.dump(4) << std::endl;

	std::promise<std::string> promise;

	/* clang-format off */
	json body =
    {
        { "label"                , label },
        { "protocol"             , protocol },
        { "sctpStreamParameters" , sctpStreamParameters }
		// { "appData"				 , "someAppData" }
	};
	/* clang-format on */

	auto r = cpr::PostAsync(
	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
	                     this->sendTransportId + "/produce/data" },
	           cpr::Body{ body.dump() },
	           cpr::Header{ { "Content-Type", "application/json" } },
			   cpr::VerifySsl{verifySsl})
	           .get();

	if (r.status_code == 200)
	{
		auto response = json::parse(r.text);

		auto it = response.find("id");
		if (it == response.end() || !it->is_string()) {
			promise.set_exception(std::make_exception_ptr("'id' missing in response"));
		} else {
			this->dataProducerId = (*it).get<std::string>();
			promise.set_value(this->dataProducerId);
		}
	}
	else
	{
		std::cerr << "[ERROR] unable to create data producer"
		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;

		promise.set_exception(std::make_exception_ptr(r.text));
	}

	return promise.get_future();
}

void Broadcaster::Start(
  const std::string& baseUrl, bool enableAudio, bool useSimulcast, const json& routerRtpCapabilities, bool verifySsl)
{
	std::cout << "[INFO] Broadcaster::Start()" << std::endl;

	this->baseUrl = baseUrl;
	this->verifySsl = verifySsl;

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
		},
		{ "rtpCapabilities", this->device.GetRtpCapabilities() }
	};
	/* clang-format on */

	auto r = cpr::PostAsync(
	           cpr::Url{ this->baseUrl + "/broadcasters" },
	           cpr::Body{ body.dump() },
	           cpr::Header{ { "Content-Type", "application/json" } },
			   cpr::VerifySsl{verifySsl})
	           .get();

	if (r.status_code != 200)
	{
		std::cerr << "[ERROR] unable to create Broadcaster"
		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;

		return;
	}

	this->CreateSendTransport(enableAudio, useSimulcast);

	this->CreateRecvTransport();
  	this->SendDataPeriodically(sendTransport, "chat", 10);
	if (recvTransport) {
		this->CreateDataConsumer(this->dataProducerId);
	}
}

void Broadcaster::CreateDataConsumer(const std::string& dataProducerId) {

	/* clang-format off */
	json body =
	{
		{ "dataProducerId", dataProducerId }
	};
	/* clang-format on */
	// create server data consumer
	auto r = cpr::PostAsync(
		cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" + this->recvTransportId + "/consume/data" },
		cpr::Body{ body.dump() },
		cpr::Header{ { "Content-Type", "application/json" } },
		cpr::VerifySsl{verifySsl})
		.get();
	if (r.status_code != 200)
	{
		std::cerr << "[ERROR] server unable to consume mediasoup recv WebRtcTransport"
				<< " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
		return;
	}

	auto response = json::parse(r.text);
	if (response.find("id") == response.end())
	{
		std::cerr << "[ERROR] 'id' missing in response" << std::endl;
		return;
	}
	std::string dataConsumerId = response["id"].get<std::string>();
	// create client consumer
	recvTransport->ConsumeData(this, dataConsumerId, dataProducerId, "chat", "", nlohmann::json());
}

void Broadcaster::CreateSendTransport(bool enableAudio, bool useSimulcast) {
	std::cout << "[INFO] creating mediasoup send WebRtcTransport..." << std::endl;

	json sctpCapabilities = this->device.GetSctpCapabilities();
	/* clang-format off */
	json body =
	{
		{ "type",    "webrtc" },
		{ "rtcpMux", true     },
		{ "sctpCapabilities", sctpCapabilities }
	};
	/* clang-format on */

	auto r = cpr::PostAsync(
	      cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports" },
	      cpr::Body{ body.dump() },
	      cpr::Header{ { "Content-Type", "application/json" } },
		  cpr::VerifySsl{verifySsl})
	      .get();

	if (r.status_code != 200)
	{
		std::cerr << "[ERROR] unable to create send mediasoup WebRtcTransport"
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
	else if (response.find("sctpParameters") == response.end())
	{
		std::cerr << "[ERROR] 'sctpParameters' missing in response" << std::endl;

		return;
	}

	std::cout << "[INFO] creating SendTransport..." << std::endl;

	this->sendTransportId = response["id"].get<std::string>();

	this->sendTransport = this->device.CreateSendTransport(
		this,
	  	this->sendTransportId,
	  	response["iceParameters"],
	  	response["iceCandidates"],
	  	response["dtlsParameters"],
	  	response["sctpParameters"]);

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

		this->sendTransport->Produce(this, audioTrack, nullptr, &codecOptions);
	}
	else
	{
		std::cerr << "[WARN] cannot produce audio" << std::endl;
	}

	///////////////////////// Create Video Producer //////////////////////////

	if (this->device.CanProduce("video"))
	{
		auto videoTrack = createSquaresVideoTrack(std::to_string(rtc::CreateRandomId()));

		if (useSimulcast)
		{
			std::vector<webrtc::RtpEncodingParameters> encodings;
			encodings.emplace_back(webrtc::RtpEncodingParameters());
			encodings.emplace_back(webrtc::RtpEncodingParameters());
			encodings.emplace_back(webrtc::RtpEncodingParameters());

			this->sendTransport->Produce(this, videoTrack, &encodings, nullptr);
		}
		else
		{
			this->sendTransport->Produce(this, videoTrack, nullptr, nullptr);
		}
	}
	else
	{
		std::cerr << "[WARN] cannot produce video" << std::endl;

		return;
	}
}

void Broadcaster::CreateRecvTransport() {
	std::cout << "[INFO] creating mediasoup recv WebRtcTransport..." << std::endl;

	json sctpCapabilities = this->device.GetSctpCapabilities();
	/* clang-format off */
	json body =
	{
		{ "type",    "webrtc" },
		{ "rtcpMux", true     },
		{ "sctpCapabilities", sctpCapabilities }
	};
	/* clang-format on */

	// create server transport
	auto r = cpr::PostAsync(
	      cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports" },
	      cpr::Body{ body.dump() },
	      cpr::Header{ { "Content-Type", "application/json" } },
		  cpr::VerifySsl{verifySsl})
	      .get();

	if (r.status_code != 200)
	{
		std::cerr << "[ERROR] unable to create mediasoup recv WebRtcTransport"
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
		std::cerr << "[ERROR] 'iceParameters' missing in response" << std::endl;

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
	else if (response.find("sctpParameters") == response.end())
	{
		std::cerr << "[ERROR] 'sctpParameters' missing in response" << std::endl;

		return;
	}

	this->recvTransportId = response["id"].get<std::string>();


	std::cout << "[INFO] creating RecvTransport..." << std::endl;

	auto sctpParameters = response["sctpParameters"];

	this->recvTransport = this->device.CreateRecvTransport(
		this,
	  	this->recvTransportId,
	  	response["iceParameters"],
	  	response["iceCandidates"],
	  	response["dtlsParameters"],
	  	sctpParameters);
}

void Broadcaster::SendDataPeriodically(mediasoupclient::SendTransport* sendTransport, std::string dataChannelLabel, uint32_t intervalSeconds)
{
	dataProducer = sendTransport->ProduceData(this);

    std::thread([this, intervalSeconds]() {
		bool run = true;
        while (run)
        {
            std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
			std::time_t t = std::chrono::system_clock::to_time_t(p);
			std::string s = std::ctime(&t);
			auto dataBuffer = webrtc::DataBuffer(s);
			std::cout << "[INFO] sending chat data: " << s <<  std::endl;
			dataProducer->Send(dataBuffer);
			run = timer_killer.wait_for(std::chrono::seconds(intervalSeconds));
        }
    }).detach();
}

void Broadcaster::OnMessage(mediasoupclient::DataConsumer* dataConsumer, const webrtc::DataBuffer& buffer)
{
	std::cout << "[INFO] Broadcaster::OnMessage()" << std::endl;
	if (dataConsumer->GetLabel() == "chat") {
		std::string s = std::string(buffer.data.data<char>(), buffer.data.size());
		std::cout << "[INFO] received chat data: " + s  <<  std::endl;
	}
}

void Broadcaster::Stop()
{
	std::cout << "[INFO] Broadcaster::Stop()" << std::endl;

	this->timer_killer.kill();

	if (this->recvTransport) {
		recvTransport->Close();
	}

	if (this->sendTransport) {
		sendTransport->Close();
	}

	cpr::DeleteAsync(cpr::Url{ this->baseUrl + "/broadcasters/" + this->id }).get();
}

void Broadcaster::OnOpen(mediasoupclient::DataProducer* dataProducer) {
	std::cout << "[INFO] Broadcaster::OnOpen()" << std::endl;
}
void Broadcaster::OnClose(mediasoupclient::DataProducer* dataProducer){
	std::cout << "[INFO] Broadcaster::OnClose()" << std::endl;
}
void Broadcaster::OnBufferedAmountChange(mediasoupclient::DataProducer* dataProducer, uint64_t size){
	std::cout << "[INFO] Broadcaster::OnBufferedAmountChange()" << std::endl;
}

