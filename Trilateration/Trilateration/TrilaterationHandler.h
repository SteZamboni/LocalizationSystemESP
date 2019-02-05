#pragma once
#include "RSSIHandler.h"
#include "Position.h"
#include "DataSensor.h"
#include "EntryDB.h"
#include <vector>
#include <map>
#include <mariadb\mysql.h>

#define N 256
#define TOLL_REL 2

class TrilaterationHandler
{
	MYSQL* conn;
	const int Nsta = 10;
	int nSchede;
	std::vector<Position> stations;
	RSSIHandler r;
	bool setup_connection();
	bool stationsQuery();
public:
	TrilaterationHandler();
	void addStation(int i, float x, float y);
	bool isInArea(Position, std::vector<DataSensor>);
	Position calcPosition(std::vector<DataSensor>);
	std::vector < EntryDB> mins2Data();
	std::vector<EntryDB> mins2Data(std::map<std::string, std::vector<DataSensor>>& mapMacMedia);
	double distanceToMediaRSSI(std::vector<DataSensor>, std::vector<DataSensor>);
	Position calcRealPosition(std::vector<DataSensor> data, Position p);
	std::map <std::string, Position> getDevices();
	void loadDB(std::map <std::string, Position>);
	~TrilaterationHandler();
};

