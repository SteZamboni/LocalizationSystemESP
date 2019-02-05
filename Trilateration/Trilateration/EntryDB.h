#pragma once
#include <string>

class EntryDB
{
	std::string hash;
	std::string mac;
	std::string fingerprint;
	double RSSI;
	int id;
public:
	EntryDB(std::string, std::string, std::string, double, int);
	std::string getHash();
	std::string getMac();
	std::string getFingerprint();
	double getRSSI();
	int getId();
	~EntryDB();
};

