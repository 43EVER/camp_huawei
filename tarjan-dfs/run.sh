#!/bin/bash

g++ final.cpp -lpthread
time ./a.out $1 < data/test_data.txt
