#include <iostream>
#include <hash_map>
using namespace std;

hash_map<unsigned long,unsigned long> sockets_map;
void addtomap(unsigned long socket,unsigned long ovl){
	sockets_map[socket] = ovl;
}
unsigned long getfrommap(unsigned long socket){
	return sockets_map[socket];
}
void removefrommap(unsigned long socket){
	sockets_map.erase(socket);
}