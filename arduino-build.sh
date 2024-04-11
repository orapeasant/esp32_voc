#!/bin/bash
PROJECT=esp32_voc.ino
BOARD=esp32:esp32:esp32da
COMMAND=$1

INCLUDE+=" -I ./src"
INCLUDE+=" -I ./src/blynk"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c"
#below needs to be moved here, otherwise, compilation error.
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/QCBOR/src"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/QCBOR/inc"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/QCBOR/test"
#above needs to be moved here, otherwise, compilation error.
INCLUDE+=" -I ./src/SFUD/sfud/inc"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/inc"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/inc/signing"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/inc/mbedtls"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/mbedtls/library"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/mbedtls/include"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/mbedtls/3rdparty/everest/include"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/mbedtls/3rdparty/everest/include/everest"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/mbedtls/3rdparty/everest/include/everest/kremlib"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-c/mbedtls/tests/include"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-platform/esp32_voc/"
INCLUDE+=" -I ./src/edge-impulse/ingestion-sdk-platform/sensors/"
INCLUDE+=" -I ./src/firmware-sdk"
INCLUDE+=" -I ./src/firmware-sdk/sensor-aq"



FLAGS+=" -O3"
FLAGS+=" -g3"
FLAGS+=" -DEI_SENSOR_AQ_STREAM=int"
FLAGS+=" -DEIDSP_QUANTIZE_FILTERBANK=0"
FLAGS+=" -D__STATIC_FORCEINLINE=__STATIC_INLINE"

rm -Rf /tmp/arduino/sketches/*

if [ "$COMMAND" = "--build" ];
then
	echo "Building $PROJECT"
	#echo "INCLUDE $INCLUDE"
	#echo "FLAGS $FLAGS"
	arduino-cli compile --fqbn  $BOARD --build-property compiler.cpp.extra_flags="$INCLUDE $FLAGS" $PROJECT &
	pid=$! # Process Id of the previous running command
	sec=0
	while kill -0 $pid 2>/dev/null
	do
		echo "Still building...$sec"
		sec=$((sec + 2))
		sleep 2
	done
	wait $pid
	ret=$?
	if [ $ret -eq 0 ]; then
		echo "Building $PROJECT done"
	else
		exit "Building $PROJECT failed"
	fi
elif [ "$COMMAND" = "--flash" ];
then
	arduino-cli upload -p $(arduino-cli board list | grep Arduino | cut -d ' ' -f1) --fqbn $BOARD -i *.bin
elif [ "$COMMAND" = "--all" ];
then
	arduino-cli compile --fqbn  $BOARD $PROJECT
	status=$?
	[ $status -eq 0 ] && arduino-cli upload -p $(arduino-cli board list | grep Arduino | cut -d ' ' -f1) --fqbn $BOARD -i *.bin
else
	echo "Nothing to do for target"
fi
