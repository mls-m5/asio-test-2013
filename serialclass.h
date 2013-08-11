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

	class oarchive{
	public:
		oarchive(std::ostream &s){
			stream = &s;
		}

		template <class t>
		void operator & (t& data){
			stream->write((char*) &data, sizeof(data));
		}

	private:
		std::ostream * stream;
	};


	class iarchieve{
	public:
		iarchieve(std::istream &s){
			stream = &s;
		}

		template <class t>
		void operator & (t& data){
			stream->read((char*)&data, sizeof(data));
		}

	private:
		std::istream * stream;
	};

	class textoarchive{
	public:
		textoarchive(std::ostream &s){
			stream = &s;
		}

		template <class t>
		void operator & (t& data){
			(*stream) <<  data << " ";
		}

	private:
		std::ostream * stream;
	};


	class textiarchieve{
	public:
		textiarchieve(std::istream &s){
			stream = &s;
		}

		template <class t>
		void operator & (t& data){
			(*stream) >>  data;
		}

	private:
		std::istream * stream;
	};
};

#endif /* SERIALCLASS_H_ */
