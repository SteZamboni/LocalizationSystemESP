/*
 * RSSIHandler.cpp
 *
 *  Created on: 23 mag 2018
 *      Author: stefa
 */

#include "RSSIHandler.h"
#include "math.h"


RSSIHandler::RSSIHandler() {
	// TODO Auto-generated constructor stub
	SignalStrength1Meter = -42;

}

RSSIHandler::~RSSIHandler() {
	// TODO Auto-generated destructor stub
}

double RSSIHandler::CalculateDistanceRSSI(double RSSI){
	double n = 2.7;
	double dTemp = (RSSI - SignalStrength1Meter)/(-10*n);
	double d = pow(10, dTemp);
	return d;
}

