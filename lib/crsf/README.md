# CRSF lib

This started as trying to do a full implementation of [Team BlackSheep's CRSFv3 spec](https://github.com/tbs-fpv/tbs-crsf-spec/tree/main)
but that turned out hard so I just did what ELRS needs for what I want

The main goal is a parsing library that didn't allocate memory, was not platform dependant, and had unit tests.

## TODO
* rename cause "CRSF lib" is not very disambiguating
* frame vs packet: crsf has frames, but I was not consistant in wordsing


## based on
* [crsf v3 spec](https://github.com/tbs-fpv/tbs-crsf-spec)
* [Britannio Jarrett's pico_crsf](https://github.com/britannio/pico_crsf/)
* [ELRS implementation](https://github.com/ExpressLRS/ExpressLRS/blob/master/src/lib/CrsfProtocol/crsf_protocol.h)
* [CaptnBry's CRServoF](https://github.com/CapnBry/CRServoF)

## testing
I can't work out how to make cmake run memcheck and normal tests at the same time, so do this
```
cmake --build build -t check
cmake --build build -t check_mem
```
or
```
watchexec cmake --build build -t check -t check_mem -j15
```
