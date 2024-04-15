#!/bin/bash
PROJECT=ei-mbed-qcbor-http-acc.ino
BOARD=esp32:esp32:esp32da
COMMAND=$1

INCLUDE+=" -I ./src/lg"
INCLUDE+=" -I ./src/blynk"
INCLUDE+=" -I ./src/qcbor"
INCLUDE+=" -I ./src/edgeimpulse"
INCLUDE+=" -I ./src/Multichallen_Gas_GM"

FLAGS+=" -O3"
FLAGS+=" -g3"
FLAGS+=" -DEI_SENSOR_AQ_STREAM=int"
FLAGS+=" -DEIDSP_QUANTIZE_FILTERBANK=0"
FLAGS+=" -D__STATIC_FORCEINLINE=__STATIC_INLINE"

BUILDPATH="./bin"

rm -Rf /tmp/arduino/sketches/*

PORT=$(arduino-cli board list | grep USB | cut -d ' ' -f1)

build() {
	echo "Building $PROJECT"
	#echo "INCLUDE $INCLUDE"
	#echo "FLAGS $FLAGS"
	arduino-cli compile --fqbn  $BOARD --build-property compiler.cpp.extra_flags="$INCLUDE" --build-property build.extra_flags="$FLAGS" --build-path="$BUILDPATH" $PROJECT &
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
}

flash() {
	arduino-cli upload -p $PORT --fqbn $BOARD -i $BUILDPATH/$PROJECT.bin
}

echo "Port: $PORT"
if [ "$COMMAND" = "--build" ];
then
	build
elif [ "$COMMAND" = "--flash" ];
then
	flash
elif [ "$COMMAND" = "--all" ];
then
	build
	status=$?
	[ $status -eq 0 ] && flash
elif [ "$COMMAND" = "--monitor" ];
then
	arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
else
	echo "Nothing to do for target"
fi
