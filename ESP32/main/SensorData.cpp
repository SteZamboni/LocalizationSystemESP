/*
 * SensorData.cpp
 *
 *  Created on: 23 mag 2018
 *      Author: Patrizio
 */

#include "SensorData.h"




using namespace std;

SensorData::SensorData(int c, int rssi, struct timeval t, char* s, char* seq_ctrl, char* ssid):
			channel(c), RSSI(rssi), time(t), source(s), sequence_ctrl(seq_ctrl), SSID(ssid){

}

SensorData::~SensorData() {

}

void SensorData::printData(){
	cout << "PACKET TYPE=PROBE CHAN=" << channel << " RSSI=" << RSSI
		 << " ADDR=" << source << " SEQ=" << sequence_ctrl
		 << " Time_sec=" << time.tv_sec << " Time_usec=" << time.tv_usec << " SSID=" << SSID << endl;
	return;
}

string SensorData::serialize(){
	ostringstream stream;

	stream << channel << "\t" << RSSI << "\t"
		 << source << "\t" << sequence_ctrl << "\t"
		 << time.tv_sec << "\t" << time.tv_usec << "\t" << SSID << "\n";

	return stream.str();
}
