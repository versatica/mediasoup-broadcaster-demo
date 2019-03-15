# mediasoup-demo broadcaster (libmediasoupclient)

## Usage

Once installed (see *Installation* below):

```bash
SERVER_URL=https://my.mediasoup-demo.org:4443 ROOM_ID=broadcaster build/broadcaster
```

## Dependencies

### [libmediasoupclient](https://github.com/jmillan/libmediasoupclient)
### [cpr](https://github.com/whoshuu/cpr)

## Installation

```bash
git clone https://github.com/jmillan/broadcaster.git
git submodule update --init --recursive

cmake . -Bbuild                                            \
-DLIBWEBRTC_INCLUDE_PATH:PATH=/Your/libwebrtc/include/path \
-DLIBWEBRTC_BINARY_PATH:PATH=/Your/libwebrtc/binary/path   \
-DOPENSSL_INCLUDE_DIR:PATH=/usr/local/opt/openssl/include  \
-DCMAKE_USE_OPENSSL=ON

make -C build
```
