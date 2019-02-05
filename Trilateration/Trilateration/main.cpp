/*
 * main.cpp
 *
 *  Created on: 23 mag 2018
 *      Author: stefa
 */

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <map>
#include <chrono>
#include <stdlib.h>
#include <Windows.h>
#include "RSSIHandler.h"
#include "TrilaterationHandler.h"
#include "Position.h"
#include "DataSensor.h"
#include <mariadb\mysql.h>

using namespace std;

int main(){

	TrilaterationHandler tril;
	while (true) {
		cout << "Calcolo posizione dispositivi" << endl;
		std::map <std::string, Position> posit = tril.getDevices();
		for (auto& kv : posit) {
			cout << "Mac::  " << kv.first << "   ";
			cout << "X::  " << kv.second.getXpos() << "   ";
			cout << "Y::  " << kv.second.getYpos() << "   "<<endl;
		}
		tril.loadDB(posit);
		Sleep(20000);
	}

	//TrilaterationHandler tril(n_SCHEDE);
	//tril.addStation(0, 0, 0);
	//tril.addStation(1, 4, 0);
	//tril.addStation(3, 5, 3);
	//tril.addStation(2, 1, 2);
	//tril.addStation(5, 3, 5);
	//tril.addStation(4, 1, 5);
	//vector<DataSensor> data;
	//data.push_back(DataSensor(0, -64, ""));	//-47 1,00 metri
	//data.push_back(DataSensor(1, -61, ""));	//-55 2,00 metri
	//data.push_back(DataSensor(2, -57, ""));	//-60 3,03 metri
	//data.push_back(DataSensor(3, -66, ""));	//-63 3.91 metri
	//									//-66 5.05 metri
	//data.push_back(DataSensor(4, -60, ""));
	//data.push_back(DataSensor(5, -55, ""));

	//Position pos = tril.calcPosition(data);

	//cout << "X:::  " << pos.getXpos() << endl;
	//cout << "Y:::  " << pos.getYpos() << endl;
	//if (tril.isInArea(pos, data))	cout << "Il punto e' interno all'area" << endl;
	//else	cout << "Il punto e' esterno all'area" << endl;


	/*RSSIHandler *r;
	r = new RSSIHandler();
	float tmp = -57;
	printf("%.2f metri\n", r->CalculateDistanceRSSI(tmp));*/
	printf("Hello World!");
	return 0;

}



