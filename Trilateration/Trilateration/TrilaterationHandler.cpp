#include "TrilaterationHandler.h"
#include "RSSIHandler.h"
#include "Position.h"
#include "DataSensor.h"
#include "EntryDB.h"
#include "math.h"
#include <string>
#include <time.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
using namespace std;


const char* MY_HOSTNAME = "192.168.43.38";
const char* MY_DATABASE = "STALPAdb";
const char* MY_USERNAME = "ssluser";
const char* MY_PASSWORD = "sslpass";

const char* PRIVATE_KEY_CLIENT = "client-key.pem";
const char* PUBLIC_CLIENT_KEY = "client-cert.pem";
const char* CERTIFICATE_AUTORITY_FILE = "ca-cert.pem";

const char* MY_SOCKET = NULL;

enum {
	MY_PORT_NO = 3306,
	MY_OPT = 0
};

bool TrilaterationHandler::stationsQuery() {
	MYSQL_RES *res;
	MYSQL_ROW row;

	for (int i = 0; i < Nsta; i++) {
		stations[i]=(Position(-1, -1));
	}

	if (mysql_query(conn, "SELECT * FROM Coordinates;")) {
		cerr << mysql_error(conn) << endl;
		return false;
	}

	res = mysql_use_result(conn);
	while ((row = mysql_fetch_row(res)) != NULL) {
		stations[atoi(row[0])] = Position(atof(row[1]), atof(row[2]));
	}

	mysql_free_result(res);
	return true;
}

TrilaterationHandler::TrilaterationHandler()
{
	for (int i = 0; i < Nsta; i++) {
		stations.push_back(Position(-1, -1));
	}
	this->conn = mysql_init(NULL);
	if (!setup_connection()) {
		cerr << "Error connecting" << endl;
	}
	if (!stationsQuery()) {
		cerr << "Error stations query" << endl;
	}
	int n=0;
	for (auto e : stations) {
		if (e.getXpos() != -1) n++;
	}
	this->nSchede = n;

}

void TrilaterationHandler::addStation(int i, float x, float y) {
	stations[i] = (Position(x, y));
}

double area(Position pos1, Position pos2, Position pos3)
{
	double x1 = pos1.getXpos();
	double x2 = pos2.getXpos();
	double x3 = pos3.getXpos();
	double y1 = pos1.getYpos();
	double y2 = pos2.getYpos();
	double y3 = pos3.getYpos();
	return abs((x1*(y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
}

bool isInside(Position a, Position b, Position c, Position point) {
	/* Calculate area of triangle ABC */
	double A = area(a, b, c);

	/* Calculate area of triangle PBC */
	double A1 = area(point, b, c);

	/* Calculate area of triangle PAC */
	double A2 = area(point, a, c);

	/* Calculate area of triangle PAB */
	double A3 = area(point, a, b);

	/* Check if sum of A1, A2 and A3 is same as A */
	return (A*1.08 > A1 + A2 + A3);
}

bool TrilaterationHandler::isInArea(Position pos, vector<DataSensor> data) {
	if (nSchede == 2) {
		if (pos.getXpos() < 0 || pos.getXpos() > stations[data[1].getSensorId()].getXpos()) return false;
		else return true;
	}
	else {
		int i, j, k, npoints;
		npoints = data.size();
		for (i = 0; i < npoints; i++) {
			for (j = i + 1; j < npoints; j++) {
				for (k = j + 1; k  < npoints; k ++)	{
					Position a = stations[data[i].getSensorId()];
					Position b = stations[data[j].getSensorId()];
					Position c = stations[data[k].getSensorId()];
					if (isInside(a, b, c, pos)) return true;
				}
			}
		}
	}
	return false;
}

bool sortData(DataSensor d1, DataSensor d2) {
	return (d1.getRSSI() > d2.getRSSI());
}

Position TrilaterationHandler::calcPosition(vector<DataSensor> data) {
	if (nSchede==2){
		vector<double> point1, point2;
		point1.push_back(stations[0].getXpos());
		point1.push_back(stations[0].getYpos());
		point2.push_back(stations[1].getXpos());
		point2.push_back(stations[1].getYpos());
		double r1, r2;
		r1 = r.CalculateDistanceRSSI(data[0].getRSSI());
		r2 = r.CalculateDistanceRSSI(data[1].getRSSI());

		double p2p1Distance = abs(point2[0] - point1[0]);
		double y = 0;
		double x = 0;
		if ((r1 + r2) < p2p1Distance) {
			x = (p2p1Distance*r1) / (r1 + r2);
		}
		else {
			x = (pow(r1, 2) - pow(r2, 2) + pow(p2p1Distance, 2)) / (2 * p2p1Distance);
		}
		return Position(x, y);
	} else {
		std::sort(data.begin(), data.end(), sortData);
		vector<double> point1, point2, point3;
		point1.push_back(stations[data[0].getSensorId()].getXpos());
		point1.push_back(stations[data[0].getSensorId()].getYpos());
		point2.push_back(stations[data[1].getSensorId()].getXpos());
		point2.push_back(stations[data[1].getSensorId()].getYpos());
		point3.push_back(stations[data[2].getSensorId()].getXpos());
		point3.push_back(stations[data[2].getSensorId()].getYpos());

		double r1, r2, r3;
		r1 = r.CalculateDistanceRSSI(data[0].getRSSI());
		r2 = r.CalculateDistanceRSSI(data[1].getRSSI());
		r3 = r.CalculateDistanceRSSI(data[2].getRSSI());

		//unit vector in a direction from point1 to point 2
		double p2p1Distance = pow(pow(point2[0] - point1[0], 2) + pow(point2[1] - point1[1], 2), 0.5);
		double exx = (point2[0] - point1[0]) / p2p1Distance;
		double exy = (point2[1] - point1[1]) / p2p1Distance;
		//signed magnitude of the x component
		double i = exx * (point3[0] - point1[0]) + exy * (point3[1] - point1[1]);
		//the unit vector in the y direction. 
		double eyx = (point3[0] - point1[0] - i * exx) / pow(pow(point3[0] - point1[0] - i * exx, 2) + pow(point3[1] - point1[1] - i * exy, 2), 0.5);
		double eyy = (point3[1] - point1[1] - i * exy) / pow(pow(point3[0] - point1[0] - i * exx, 2) + pow(point3[1] - point1[1] - i * exy, 2), 0.5);
		//the signed magnitude of the y component
		double j = eyx * (point3[0] - point1[0]) + eyy * (point3[1] - point1[1]);
		//coordinates
		double x = (pow(r1, 2) - pow(r2, 2) + pow(p2p1Distance, 2)) / (2 * p2p1Distance);
		double y = (pow(r1, 2) - pow(r3, 2) + pow(i, 2) + pow(j, 2)) / (2 * j) - i * x / j;
		//result coordinates
		double finalX = point1[0] + x * exx + y * eyx;
		double finalY = point1[1] + x * exy + y * eyy;
		return Position(finalX, finalY);
	}
}

bool TrilaterationHandler::setup_connection() {

	//Set encrypted connection using SSL
	mysql_ssl_set(conn, PRIVATE_KEY_CLIENT, PUBLIC_CLIENT_KEY, CERTIFICATE_AUTORITY_FILE, NULL, NULL);

	// Establish a MySQL connection
	if (!mysql_real_connect(
		conn,
		MY_HOSTNAME, MY_USERNAME,
		MY_PASSWORD, MY_DATABASE,
		MY_PORT_NO, MY_SOCKET, MY_OPT)) {
		cerr << mysql_error(conn) << endl;
		return false;
	}

	return true;
}

std::vector<EntryDB> TrilaterationHandler::mins2Data() {
	MYSQL_RES *res;
	MYSQL_ROW row;

	vector<EntryDB> rows5mins;

	//Creation of actual timestamp
	time_t seconds = time(0);
	char actual_timestamp[N], mins1bef_timestamp[N];
	struct tm timeinfo;

	errno_t result = localtime_s(&timeinfo, &seconds);
	strftime(actual_timestamp, N, "%F %X", &timeinfo);

	seconds -= 60 * 2;

	result = localtime_s(&timeinfo, &seconds);
	strftime(mins1bef_timestamp, N, "%F %X", &timeinfo);
	/*
	if (!setup_connection()) {
		cout << "Connection with DB failed!" << endl;
	}*/
		ostringstream str,str2;

		str << "SELECT * FROM SensorMeasure WHERE Timestamp <= '" << actual_timestamp << "' AND Timestamp >= '" << mins1bef_timestamp << "' ORDER BY Timestamp;";

		if (mysql_query(conn, str.str().c_str())) {
			cerr << mysql_error(conn) << endl;
		}

		res = mysql_use_result(conn);
		while ((row = mysql_fetch_row(res)) != NULL) {
			//Creation of EntryDB
			EntryDB e(std::string(row[1]), std::string(row[2]), std::string(row[8]), atof(row[5]), atoi(row[0]));

			//Insert into vector
			rows5mins.push_back(e);

		}

		mysql_free_result(res);

	// Close a MySQL connection
	//mysql_close(conn);

	return rows5mins;
}

std::vector<EntryDB> TrilaterationHandler::mins2Data(std::map<std::string, std::vector<DataSensor>>& mapMacMedia) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	vector<EntryDB> rows5mins;

	//Creation of actual timestamp
	time_t seconds = time(0);
	char actual_timestamp[N], mins1bef_timestamp[N];
	struct tm timeinfo;

	errno_t result = localtime_s(&timeinfo, &seconds);
	strftime(actual_timestamp, N, "%F %X", &timeinfo);

	seconds -= 60 * 2;

	result = localtime_s(&timeinfo, &seconds);
	strftime(mins1bef_timestamp, N, "%F %X", &timeinfo);
	/*
	if (!setup_connection()) {
	cout << "Connection with DB failed!" << endl;
	}*/
	ostringstream str, str2;

	str << "SELECT * FROM SensorMeasure WHERE Timestamp <= '" << actual_timestamp << "' AND Timestamp >= '" << mins1bef_timestamp << "' ORDER BY Timestamp;";

	if (mysql_query(conn, str.str().c_str())) {
		cerr << mysql_error(conn) << endl;
	}

	res = mysql_use_result(conn);
	while ((row = mysql_fetch_row(res)) != NULL) {
		//Creation of EntryDB
		EntryDB e(std::string(row[1]), std::string(row[2]), std::string(row[8]), atof(row[5]), atoi(row[0]));

		//Insert into vector
		rows5mins.push_back(e);

	}

	mysql_free_result(res);

	str2 << "SELECT MACsender, IDboard, Fingerprint, AVG(RSSI) FROM SensorMeasure WHERE Timestamp <= '" << actual_timestamp << "' AND Timestamp >= '" << mins1bef_timestamp << "' GROUP BY  MACsender, IDboard, Fingerprint;";

	if (mysql_query(conn, str2.str().c_str())) {
		cerr << mysql_error(conn) << endl;
	}

	res = mysql_use_result(conn);
	while ((row = mysql_fetch_row(res)) != NULL) {

		//Insert into vector
		if (mapMacMedia.count(std::string(row[0])) == 0) {
			mapMacMedia.insert(std::pair<std::string, std::vector<DataSensor>>(std::string(row[0]), std::vector<DataSensor>()));
		}
		mapMacMedia[row[0]].push_back(DataSensor(atoi(row[1]), atof(row[3]), std::string(row[0]), std::string(row[2])));

	}

	// Close a MySQL connection
	//mysql_close(conn);

	return rows5mins;
}

double TrilaterationHandler::distanceToMediaRSSI(std::vector<DataSensor> data, std::vector<DataSensor> media) {
	double dist = 0;
	for (auto i = data.begin(); i != data.end(); i++) {
		for (auto j = media.begin(); j != media.end(); j++) {
			if (i->getSensorId() == j->getSensorId()) {
				dist += abs(i->getRSSI() - j->getRSSI());
			}
		}
	}
	return dist;
}

Position TrilaterationHandler::calcRealPosition(std::vector<DataSensor> data, Position p) {
	if (nSchede == 2)return p; //Vedere se fare realpos con 2 schede
	RSSIHandler r;
	std::sort(data.begin(), data.end(), sortData);
	for (int i = 0; i < 10; i++) {
		// Computethe delta distance to add to the position, it add the 1/10 of the distance first loop, than 1/15 than 1/20 ecc...
		double dist3 = r.CalculateDistanceRSSI(data[2].getRSSI()) / (10.0 + i * 5);
		double dist2 = r.CalculateDistanceRSSI(data[1].getRSSI()) / (10.0 + i * 5);
		double dist1 = r.CalculateDistanceRSSI(data[0].getRSSI()) / (10.0 + i * 5);

		// Compute the 3 angular coefficient of the line between the station and the point
		double m3 = (p.getYpos() - stations[data[2].getSensorId()].getYpos()) / (p.getXpos() - stations[data[2].getSensorId()].getXpos());
		double m2 = (p.getYpos() - stations[data[1].getSensorId()].getYpos()) / (p.getXpos() - stations[data[1].getSensorId()].getXpos());
		double m1 = (p.getYpos() - stations[data[0].getSensorId()].getYpos()) / (p.getXpos() - stations[data[0].getSensorId()].getXpos());

		// Compute the deltaX from the 3 stations based on Pitagora theorem d^2 = x^2 + y^2 ----> x^2 = d^2 - y^2 ---> x^2 = d^2 - m^2x^2 ---> x^2(m^2 + 1) = d^2
		double x3 = dist3 / pow((pow(m3, 2) + 1.0), 0.5);
		double x2 = dist2 / pow((pow(m2, 2) + 1.0), 0.5);
		double x1 = dist1 / pow((pow(m1, 2) + 1.0), 0.5);
		// Compute the deltaY with Pitagora theorem
		double y3 = pow((pow(dist3, 2) - pow(x3, 2)), 0.5);
		double y2 = pow((pow(dist2, 2) - pow(x2, 2)), 0.5);
		double y1 = pow((pow(dist1, 2) - pow(x1, 2)), 0.5);

		// These if check how to orient the deltaX and deltaY according to the direction of the line and position of the stations
		if (stations[data[2].getSensorId()].getXpos() > p.getXpos()) 	x3 = -x3;
		if (stations[data[1].getSensorId()].getXpos() > p.getXpos()) 	x2 = -x2;
		if (stations[data[0].getSensorId()].getXpos() > p.getXpos()) 	x1 = -x1;

		if (m3 < 0) y3 = -y3;
		if (m2 < 0) y2 = -y2;
		if (m1 < 0) y1 = -y1;

		// Try computing the X and Y "pulling" the point to the stations
		double xT = p.getXpos() + x3 + x2 + x1;
		double yT = p.getYpos() + y3 + y2 + y1;

		x3 = -x3;
		x2 = -x2;
		x1 = -x1;
		y3 = -y3;
		y2 = -y2;
		y1 = -y1;

		// Try computing the X and Y "pushing away" the point to the stations
		double xS = p.getXpos() + x3 + x2 + x1;
		double yS = p.getYpos() + y3 + y2 + y1;

		double distFromNearestOLD = pow((pow((p.getXpos() - stations[data[0].getSensorId()].getXpos()), 2) + pow((p.getYpos() - stations[data[0].getSensorId()].getYpos()), 2)), 0.5);
		
		// Check wich one of the two has the best value of distance from the nearest station --> the nearest station is the most accurate, so if it's right for the nearest
		// it's possible that is the most accurate value
		double distFromNearestT = pow((pow((xT - stations[data[0].getSensorId()].getXpos()), 2) + pow((yT - stations[data[0].getSensorId()].getYpos()), 2)), 0.5);
		double distFromNearestS = pow((pow((xS - stations[data[0].getSensorId()].getXpos()), 2) + pow((yS - stations[data[0].getSensorId()].getYpos()), 2)), 0.5);

		// Update the position with the one better between the pull and the push, and if it's better keep looping, else return the new position
		if (abs(dist1 - distFromNearestT) < abs(dist1 - distFromNearestS)) {
			if (abs(dist1 - distFromNearestT) < abs(dist1 - distFromNearestOLD)) {
				p = Position(xT, yT);
			}
			else return p;
		}
		else {
			if (abs(dist1 - distFromNearestS) < abs(dist1 - distFromNearestOLD)) {
				p = Position(xS, yS);
			}
			else return p;
		}

		if (data[0].getRSSI() < -53) return p;

	}
	return p;
}

std::map <std::string, Position> TrilaterationHandler::getDevices() {
	std::map <std::string, std::vector<DataSensor>> data;
	std::map <std::string, std::vector<DataSensor>> devicePositionsTMP;
	std::map <std::string, Position> devicePositions;
	std::map <std::string, std::vector<DataSensor>> mapMacMedia;

	std::vector<EntryDB> arr;

	if (!stationsQuery()) {
		cerr << "Error connecting" << endl;
	}
	int n = 0;
	for (auto e : stations) {
		if (e.getXpos() != -1) n++;
	}
	this->nSchede = n;
	if (n == 1) return devicePositions;
	// Functions retrieves last 2 minutes of data in the DB and fill the mapMacMedia
	arr = mins2Data(mapMacMedia);

	for (auto& e : arr) {
		// Check if RSSI is too high --- if is higher then 80 is not a trustable distance
		if (e.getRSSI() > -80) {
			if (data.count(e.getHash()) == 0) {
				//Check if the board is in the initial n boards and not inserted later
				if (stations[e.getId()].getXpos() == -1) {
					if (!stationsQuery()) {
						cerr << "Error connecting" << endl;
					}
					int n = 0;
					for (auto e : stations) {
						if (e.getXpos() != -1) n++;
					}
					this->nSchede = n;
				}
				// Init in the map the entry with the MAC just read if it's the first time i receive that MAC
				data.insert(std::pair<std::string, std::vector<DataSensor>>(e.getHash(), std::vector<DataSensor>()));
				// Push back the data just read
				data[e.getHash()].push_back(DataSensor(e.getId(), e.getRSSI(), e.getMac(), e.getFingerprint()));
			}
			else {
				// The entry already exists, just add the new data
				data[e.getHash()].push_back(DataSensor(e.getId(), e.getRSSI(), e.getMac(), e.getFingerprint()));
			}
		}
	}
	for (auto& kv : data) {
		// Check if the data has been read by all the boards
		if (kv.second.size() == nSchede) {
			if (devicePositionsTMP.count(kv.second[0].getMac()) == 0) {
				// The first time we have a position for a MAC
				devicePositionsTMP[kv.second[0].getMac()] = kv.second;
			}
			else {
				// Calculate the best entry of the vector based on the RSSI 
					// RSSI lower ---> Best accuracy
				double distNew = distanceToMediaRSSI(kv.second, mapMacMedia[kv.second[0].getMac()]);
				double distOld = distanceToMediaRSSI(devicePositionsTMP[kv.second[0].getMac()], mapMacMedia[kv.second[0].getMac()]);
				if (distNew < distOld) {
					devicePositionsTMP[kv.second[0].getMac()] = kv.second;
				}
			}
		}
	}

	for (auto& kv : devicePositionsTMP) {
		if (kv.second.size() == nSchede) {
			Position pos = calcPosition(kv.second);
			cout << "Mac:  " << kv.second[0].getMac() << "   ";
			cout << "X:  " << pos.getXpos() << "   ";
			cout << "Y:  " << pos.getYpos() << "   ";
			Position realPos = calcRealPosition(kv.second, pos);
			cout << "rX:  " << realPos.getXpos() << "   ";
			cout << "rY:  " << realPos.getYpos() << endl;
			realPos.setFingerprint(kv.second[0].getFingerprint());
			cout << "\t\t\t\t" << kv.second[0].getFingerprint() << endl;
			if (isInArea(pos, kv.second))
				cout << "       E' interna pos!  ";
			else
				cout << "       E' esterna pos!  ";
			if (isInArea(realPos, kv.second)) {
				cout << "   E' interna real!" << endl;
				if (devicePositions.count(kv.second[0].getMac()) == 0) {
					devicePositions.insert(std::pair<std::string, Position>(kv.second[0].getMac(), realPos));
				}
				else {
					devicePositions.erase(kv.second[0].getMac());
					devicePositions.insert(std::pair<std::string, Position>(kv.second[0].getMac(), realPos));
					//devicePositions[kv.second[0].getMac()] = pos;
				}
			}
			else {
				cout << "   E' esterna real!" << endl;
			}
		}
	}
	return devicePositions;
}

void TrilaterationHandler::loadDB(std::map <std::string, Position> macPos) {
	for (auto& kv : macPos) {
		ostringstream str;

		//Creation of actual timestamp
		time_t seconds = time(0);
		char actual_timestamp[N];
		struct tm timeinfo;

		errno_t result = localtime_s(&timeinfo, &seconds);
		strftime(actual_timestamp, N, "%F %X", &timeinfo);

	str << "INSERT INTO Points(MACSender, Fingerprint, Timestamp, CoordX, CoordY) VALUES (' "<< kv.first <<" ', '" << kv.second.getFingerprint() << "', '" << actual_timestamp <<"', '"<<kv.second.getXpos()<<"','"<<kv.second.getYpos()<<"' )";

		if (mysql_query(conn, str.str().c_str())) {
			cerr << mysql_error(conn) << endl;
		}

	}
}

TrilaterationHandler::~TrilaterationHandler() {
}