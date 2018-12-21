base: stringplus/stringplus.o encoding/utf8.o encoding/url.o encoding/base64.o encoding/tcencrypt.o general/gpio.o
	ar r libtc.a stringplus/stringplus.o
	ar r libtc.a encoding/utf8.o
	ar r libtc.a encoding/url.o
	ar r libtc.a encoding/base64.o
	ar r libtc.a encoding/tcencrypt.o
	ar r libtc.a general/gpio.o
	ar s libtc.a

networking: base network/endpoint.o network/client.o network/acceptor.o h2ep/event.o h2ep/xendpoint.o h2ep/xclient.o h2ep/xacceptor.o
	ar r libtc.a network/endpoint.o
	ar r libtc.a network/client.o
	ar r libtc.a network/acceptor.o
	ar r libtc.a h2ep/event.o
	ar r libtc.a h2ep/xendpoint.o
	ar r libtc.a h2ep/xclient.o
	ar r libtc.a h2ep/xacceptor.o
	ar s libtc.a
	
webcap: networking webcap/buffer.o webcap/jpeg.o webcap/capture.o
	ar r libtc.a webcap/buffer.o
	ar r libtc.a webcap/jpeg.o
	ar r libtc.a webcap/capture.o
	ar s libtc.a
	

stringplus/stringplus.o: stringplus/stringplus.cpp stringplus/stringplus.h
	g++ stringplus/stringplus.cpp -std=c++11 -c -o stringplus/stringplus.o
	
encoding/utf8.o: encoding/utf8.h encoding/utf8.cpp
	g++ encoding/utf8.cpp -std=c++11 -c -o encoding/utf8.o
	
encoding/url.o: encoding/url.h encoding/url.cpp
	g++ encoding/url.cpp -std=c++11 -c -o encoding/url.o
	
encoding/base64.o: encoding/base64.h encoding/base64.cpp
	g++ encoding/base64.cpp -std=c++11 -c -o encoding/base64.o

encoding/tcencrypt.o: encoding/tcencrypt.h encoding/tcencrypt.cpp
	g++ encoding/tcencrypt.cpp -std=c++11 -c -o encoding/tcencrypt.o
	
general/gpio.o: general/gpio.h general/gpio.cpp
	g++ general/gpio.cpp -std=c++11 -c -o general/gpio.o
	
	
network/endpoint.o: network/endpoint.h network/endpoint.cpp
	g++ network/endpoint.cpp -std=c++11 -c -o network/endpoint.o

network/client.o: network/client.h network/client.cpp
	g++ network/client.cpp -std=c++11 -c -o network/client.o
	
network/acceptor.o: network/acceptor.h network/acceptor.cpp
	g++ network/acceptor.cpp -std=c++11 -c -o network/acceptor.o

h2ep/event.o: h2ep/event.h h2ep/event.cpp
	g++ h2ep/event.cpp -std=c++11 -c -o h2ep/event.o
	
h2ep/xendpoint.o: h2ep/endpoint.h h2ep/endpoint.cpp
	g++ h2ep/endpoint.cpp -std=c++11 -c -o h2ep/xendpoint.o
	
h2ep/xclient.o: h2ep/client.h h2ep/client.cpp
	g++ h2ep/client.cpp -std=c++11 -c -o h2ep/xclient.o
	
h2ep/xacceptor.o: h2ep/acceptor.h h2ep/acceptor.cpp
	g++ h2ep/acceptor.cpp -std=c++11 -c -o h2ep/xacceptor.o
	
webcap/buffer.o: webcap/buffer.h webcap/buffer.cpp
	g++ webcap/buffer.cpp -std=c++11 -c -o webcap/buffer.o
	
webcap/jpeg.o: webcap/jpeg.h webcap/jpeg.cpp
	g++ webcap/jpeg.cpp -std=c++11 -c -o webcap/jpeg.o
	
webcap/capture.o: webcap/capture.h webcap/capture.cpp
	g++ webcap/capture.cpp -std=c++11 -c -o webcap/capture.o