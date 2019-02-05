#include "DataSensor.h"



DataSensor::DataSensor(int id, double rssi, std::string mac, std::string fingerprint) {
	this->RSSI = rssi;
	this->sensor = id;
	this->mac = mac;
	this->fingerprint = fingerprint;
}

double DataSensor::getRSSI() {
	return this->RSSI;
}

int DataSensor::getSensorId() {
	return this->sensor;
}

std::string DataSensor::getMac() {
	return this->mac;
}

std::string DataSensor::getFingerprint() {
	return this->fingerprint;
}

DataSensor::~DataSensor()
{
}
