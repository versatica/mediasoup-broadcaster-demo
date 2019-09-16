# mediasoup broadcaster demo (libmediasoupclient)

[libmediasoupclient][libmediasoupclient] based application that takes the system microphone and webcam and produces the media to the specified room in [mediasoup-demo][mediasoup-demo] application.


## Resources

* mediasoup website and documentation: [mediasoup.org](https://mediasoup.org)
* mediasoup support forum: [mediasoup.discourse.group](https://mediasoup.discourse.group)


## Usage

Once installed (see **Installation** below):

```bash
SERVER_URL=https://my.mediasoup-demo.org:4443 ROOM_ID=broadcaster build/broadcaster
```

Environment variables:

* `SERVER_URL`: The URL of the mediasoup-demo HTTP API server (required).
* `ROOM_ID`: Room id (required).
* `WEBRTC_DEBUG`: Enable libwebrtc full logging if "true" (optional).

## Dependencies

Already included in the repository.

* [libmediasoupclient][libmediasoupclient]
* [cpr][cpr]


## Installation

```bash
git clone https://github.com/versatica/mediasoup-broadcaster-demo.git

cmake . -Bbuild                                            \
-DLIBWEBRTC_INCLUDE_PATH:PATH=/Your/libwebrtc/include/path \
-DLIBWEBRTC_BINARY_PATH:PATH=/Your/libwebrtc/binary/path   \
-DOPENSSL_INCLUDE_DIR:PATH=/usr/local/opt/openssl/include  \
-DCMAKE_USE_OPENSSL=ON

make -C build
```




[mediasoup-demo]: https://github.com/versatica/libmediasoupclient
[libmediasoupclient]: https://github.com/versatica/libmediasoupclient
[cpr]: https://github.com/whoshuu/cpr
