#!/bin/bash

rm obj/*.o
cd obj
g++ -c ../*/*.cpp ../*.cpp -std=c++23
g++ *.o -o ../API.app -L/usr/include/mysql -lmysqlclient -std=c++23
cd ..
./API.app
