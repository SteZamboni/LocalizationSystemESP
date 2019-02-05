#pragma once
#include <string>

class DataSensor
{
	std::string fingerprint;
	std::string mac;
	double RSSI;
	int sensor;
public:
	DataSensor(int, double, std::string, std::string);
	double getRSSI();
	int getSensorId();
	std::string getMac();
	std::string getFingerprint();
	~DataSensor();
};

