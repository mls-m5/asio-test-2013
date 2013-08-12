/*
 * chat_message.h
 *
 *  Created on: 12 aug 2013
 *      Author: mattias
 */

#ifndef CHAT_MESSAGE_H_
#define CHAT_MESSAGE_H_
#include <string>

using std::string;

class chat_message{
public:
	enum {
		mtText = 1,
		mtUnit = 2
	};
	chat_message(){
		header.type = 0;
		header.size = 0;
	}

	char * body(){
//		return &mess.c_str()[header_length + 1];
		return &mess[header_length + 1];
	}

	char * begin(){
		return (char*) &header;
	}

	char * data(){
//		return mess.c_str();
		return mess;
	}

	size_t length(){
		return header.size + sizeof(header);
	}

	void setString(string str){
		header.size = str.size();
		const char* c = str.c_str();
		for (int i = 0; i < header.size; ++i){
			mess[i] = c[i];
		}
		header.type = mtText;
	}

	string getString(){
		return string(mess, mess + header.size);
	}

	//string mess;
	struct{
		int type;
		int size;
	} header;
	char mess[1000];

	enum {header_length = sizeof(header)};
	enum {max_buffer_size = 1000};
};


#endif /* CHAT_MESSAGE_H_ */
