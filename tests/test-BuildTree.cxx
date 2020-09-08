#define CATCH_CONFIG_MAIN

#include  "catch.hpp"
#include <eicsmear/functions.h>

TEST_CASE( "BuildTree (fail)", "[single-file]" ) {
    REQUIRE( BuildTree("beagle_eD.txt", ".", -1) == 1 );
}
TEST_CASE( "BuildTree (pass)", "[single-file]" ) {
    REQUIRE( BuildTree("beagle_eD.txt", ".", -1) == 0 );
}

//g++ -std=c++11 -Wall -pthread -std=c++11 -m64 -I/home/samuel/fnal/root-6.20/install/include -I/home/samuel/eic/eic-smear/include -o test-BuildTree test-BuildTree.cxx -L/home/samuel/fnal/root-6.20/install/lib -lGui -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -pthread -lm -ldl -rdynamic -L/home/samuel/eic/eic-smear-install/lib -leicsmear