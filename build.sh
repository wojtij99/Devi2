#!/bin/bash

rm build/*.o
cd build
#g++ -c ../*.cpp ../*/*.cpp ../*/*/*.cpp ../main.cpp -std=c++23
g++ -c ../*.cpp ../*/*.cpp -std=c++23
g++ *.o -o ../API.app -L/usr/include/mysql -lmysqlclient -std=c++23
cd ..
./API.app