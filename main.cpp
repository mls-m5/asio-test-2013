//Licens http://www.boost.org/LICENSE_1_0.txt

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/array.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>
#include "serialclass.h"

using std::endl; using std::cout;
using std::stringstream;

using boost::asio::ip::tcp;

int main(int argc, char **argv) {
	SerialClass sc;

	try
	{
		if (argc != 2){
			std::cerr << "usage: client <host>" << std::endl;
			return 1;
		}
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], "31415");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);

		stringstream ss;
		for (;;){
			boost::array<char, 128> buf;
			boost::system::error_code error;
			size_t len = socket.read_some(boost::asio::buffer(buf), error);

			if (error == boost::asio::error::eof){
				break;
			}
			else if(error){
				throw boost::system::system_error(error);
			}

			ss.write(&buf.front(), len);
		}

		boost::archive::binary_iarchive ia(ss);
		Serialize::serialize(ia, sc, 1.3);
		cout << "klassen " << endl;
		cout << "x: " << sc.getX() << " y: " << sc.getY() << endl;

	}
	catch (std::exception& e){
		std::cerr << e.what() << std::endl;
	}
}
