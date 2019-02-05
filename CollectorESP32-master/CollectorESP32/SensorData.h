#pragma once
#include <time.h>
#include <string>
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

class SensorData {
private:
	char IDboard;
	int channel;
	int RSSI;
	int seconds;
	int useconds;
	string source;
	string sequence_ctrl;
	string SSID;
	string tags;

public:
	SensorData(char IDboard, int c, int rssi, int sec, int usec, string s, string seq_ctrl, string ssid, string str_tag);
	virtual ~SensorData();
	void printData();
	string createValues();
};
