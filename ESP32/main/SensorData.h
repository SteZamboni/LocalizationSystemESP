/*
 * SensorData.h
 *
 *  Created on: 23 mag 2018
 *      Author: Patrizio
 */

#ifndef MAIN_SENSORDATA_H_
#define MAIN_SENSORDATA_H_

#include <sys/time.h>
#include <string>
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

class SensorData {
private:
	int channel ;
	int RSSI;
	struct timeval time;
	string source;
	string sequence_ctrl;
	string SSID;

public:
	SensorData(int c, int rssi, struct timeval t, char* s, char* seq_ctrl, char* ssid);
	virtual ~SensorData();
	void printData();
	string serialize();
};

#endif /* MAIN_SENSORDATA_H_ */
