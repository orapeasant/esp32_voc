#!/bin/bash
PROJECT=esp32_voc.ino
#BOARD=esp32:esp32:esp32da
BOARD=esp32:esp32:esp32s3
BAUDRATE=115200
COMMAND=$1

INCLUDE+=" -I ./src/lg/"
INCLUDE+=" -I ./src/blynk/"
INCLUDE+=" -I ./src/qcbor/"
INCLUDE+=" -I ./src/edgeimpulse/"
INCLUDE+=" -I ./src/Multichallen_Gas_GM/"
INCLUDE+=" -I ./src/edgeimpulse/edge-impulse-sdk/"
INCLUDE+=" -I ./src/edgeimpulse/edge-impulse-sdk/dsp/"
INCLUDE+=" -I ./src/edgeimpulse/model-parameters/"
INCLUDE+=" -I ./src/edgeimpulse/tflite-model/"

#LIBRARIES+=" --libraries  ./libs"
LIBRARIES+=""
BUILDPATH="./bin"
#CONFIG="--config-file ./arduino-cli.yaml"

FLAGS+=" -O3"
FLAGS+=" -g3"
FLAGS+=" -DEI_SENSOR_AQ_STREAM=int"
FLAGS+=" -DEIDSP_QUANTIZE_FILTERBANK=0"
FLAGS+=" -D__STATIC_FORCEINLINE=__STATIC_INLINE"


PORT=$(arduino-cli board list | grep USB | cut -d ' ' -f1)

build() {
	echo "Building $PROJECT"
	#echo "INCLUDE $INCLUDE"
	#echo "FLAGS $FLAGS"
	arduino-cli compile --fqbn  $BOARD  $CONFIG $LIBRARIES --build-property compiler.cpp.extra_flags="$INCLUDE" --build-property build.extra_flags="$FLAGS" --build-path="$BUILDPATH"  $PROJECT  &
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
	arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=$BAUDRATE
else
	echo "Nothing to do for target"
fi
