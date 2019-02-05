#include "Position.h"


Position::Position(double t, double u) {
	this->x = t;
	this->y = u;
}

Position::Position(double t, double u, std::string fingerprint) {
	this->fingerprint = fingerprint;
	this->x = t;
	this->y = u;
}

double Position::getXpos() {
	return this->x;
}

double Position::getYpos() {
	return this->y;
}

std::string Position::getFingerprint() {
	return this->fingerprint;
}

void Position::setFingerprint(std::string fingerprint)
{
	this->fingerprint = fingerprint;
}

Position::~Position()
{
}
