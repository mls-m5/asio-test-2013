/*
 * serialclass.h
 *
 *  Created on: 11 aug 2013
 *      Author: mattias
 */

#ifndef SERIALCLASS_H_
#define SERIALCLASS_H_

#include <ostream>
#include <istream>

class SerialClass {
public:
	SerialClass();
	virtual ~SerialClass();

	double getX(){
		return x;
	}

	void setX(double v){
		x = v;
	}

	double getY(){
		return y;
	}

	friend class Serialize;

protected:
	double x;
	double y;
};


class Serialize{
public:
	template <class Archive>
	static void serialize(Archive& ar, SerialClass& t, const unsigned int file_version){
		ar & t.x;
		ar & t.y;
	}
};

#endif /* SERIALCLASS_H_ */
