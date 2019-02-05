#include "EntryDB.h"



EntryDB::EntryDB(std::string hash, std::string mac, std::string fingerprint, double RSSI, int id) {
	this->fingerprint = fingerprint;
	this->hash = hash;
	this->id = id;
	this->mac = mac;
	this->RSSI = RSSI;
}

std::string EntryDB::getHash() {
	return this->hash;
}
std::string EntryDB::getMac() {
	return this->mac;
}
std::string EntryDB::getFingerprint() {
	return this->fingerprint;
}
double EntryDB::getRSSI() {
	return this->RSSI;
}
int EntryDB::getId() {
	return this->id;
}

EntryDB::~EntryDB()
{
}
