# mediasoup broadcaster demo (libmediasoupclient)

[libmediasoupclient][libmediasoupclient] based application that takes the system microphone and webcam and produces the media to the specified room in [mediasoup-demo][mediasoup-demo] application.

## Usage

Once installed (see *Installation* below):

```bash
SERVER_URL=https://my.mediasoup-demo.org:4443 ROOM_ID=broadcaster build/broadcaster
```

## Dependencies (already included in the repository)

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
