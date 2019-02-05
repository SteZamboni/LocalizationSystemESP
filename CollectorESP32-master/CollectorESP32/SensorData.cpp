#include "stdafx.h"
#include "SensorData.h"
#include <time.h>

using namespace std;

SensorData::SensorData(char IDboard, int c, int rssi, int sec, int usec, string s, string seq_ctrl, string ssid, string str_tag) :
	IDboard(IDboard), channel(c), RSSI(rssi), seconds(sec), useconds(usec), source(s), sequence_ctrl(seq_ctrl), SSID(ssid), tags(str_tag) {

}

SensorData::~SensorData() {

}

void SensorData::printData() {
	time_t rawtime = seconds;
	const int buffer_size = 256;
	char buffer[buffer_size];

	struct tm timeinfo;

	errno_t result = localtime_s(&timeinfo, &rawtime);
	asctime_s(buffer, buffer_size, &timeinfo);

	cout << "PACKET TYPE=PROBE IDboard= " << IDboard << " CHAN=" << channel << " RSSI=" << RSSI
		<< " ADDR=" << source << " SEQ=" << sequence_ctrl
		<< " SSID=" << SSID  << " Time= " << buffer << "TAGS= " << tags << endl;

	return;
}

string SensorData::createValues() {
	time_t rawtime = seconds;
	const int buffer_size = 256;
	char buffer[buffer_size];

	struct tm timeinfo;

	errno_t result = localtime_s(&timeinfo, &rawtime);
	strftime(buffer, buffer_size, "%F %X", &timeinfo);

	ostringstream str;
	ostringstream hash;

	hash << buffer << " " << source << " " << sequence_ctrl << " " << SSID;

	str << "('" << IDboard << "','" << hash.str() << "','" << source << "','" << SSID << "','" << buffer << "','" << RSSI << "','" << channel << "','" << sequence_ctrl << "', MD5('" << tags << "'))";

	return str.str();
}

