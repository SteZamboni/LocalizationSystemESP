#pragma once
#include <string>


class Position
{
	std::string fingerprint;
	double x;
	double y;
public:
	Position(double t, double u);
	Position(double t, double u, std::string);
	double getXpos();
	double getYpos();
	std::string getFingerprint();
	void setFingerprint(std::string);
	~Position();
};

