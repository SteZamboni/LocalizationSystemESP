/*
 * RSSIHandler.h
 *
 *  Created on: 23 mag 2018
 *      Author: stefa
 */

#ifndef RSSIHANDLER_H_
#define RSSIHANDLER_H_

class RSSIHandler {
	double SignalStrength1Meter;
public:
	RSSIHandler();
	virtual ~RSSIHandler();
	double CalculateDistanceRSSI(double);
};

#endif /* RSSIHANDLER_H_ */
