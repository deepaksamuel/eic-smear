#define CATCH_CONFIG_MAIN

#include  "catch.hpp"
#include <eicsmear/functions.h>






// TEST_CASE( "BuildTree-Beagle (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("dis_hepmc2.dat", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-dis_hepmc2 (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("dis_hepmc2.dat", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-dis_hepmc3 (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("dis_hepmc3.dat", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-nc_hepmc2 (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("nc_hepmc2.dat", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-nc_hepmc3 (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("nc_hepmc3.dat", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-sartre (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("ep_sartre.txt", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-hiQ2 (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("ep_hiQ2.20x250.small.txt.gz", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-lowQ2 (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("ep_lowQ2.20x250.small.txt.gz", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-input-hiQ2 (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("input.data.ep_hiQ2.20x250.small.eic", ".", -1) == -1 );
// }
// TEST_CASE( "BuildTree-input-lowQ2 (pass)", "[single-file]" ) {
//      REQUIRE( BuildTree("input.data.ep_lowQ2.20x250.small.eic", ".", -1) == -1 );
// }
// TEST_CASE( "BuildTree-empty", "[single-file]" ) {
//      REQUIRE( BuildTree("beagle_eD.txt", ".", -1) == 0 );
// }
// TEST_CASE( "BuildTree-empty", "[single-file]" ) {
//      REQUIRE( BuildTree("beagle_eD.txt", ".", -2) == 0 );
// }



//the file has only 1000 events but the suffix in the file name will be misleading in this case
// TEST_CASE( "BuildTree-empty", "[single-file]" ) {

//      REQUIRE( BuildTree("beagle_eD.txt", ".", 1000000) == 0 );
// }

//when an empty file name is given, BuildTree should exit gracefully but it seems to hang somewhere...
// TEST_CASE( "BuildTree-empty", "[single-file]" ) {
//      REQUIRE( BuildTree("", ".", -1) == 0 );
// }

//when an empty file is given, BuildTree should exit gracefully but it seems to hang somewhere...
// TEST_CASE( "BuildTree-empty", "[single-file]" ) {
//      REQUIRE( BuildTree("empty.txt", ".", -1) == -1 );
// }

//g++ -std=c++11 -Wall -pthread -std=c++11 -m64 -I/home/samuel/fnal/root-6.20/install/include -I/home/samuel/eic/eic-smear/include -o test-BuildTree test-BuildTree.cxx -L/home/samuel/fnal/root-6.20/install/lib -lGui -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -pthread -lm -ldl -rdynamic -L/home/samuel/eic/eic-smear-install/lib -leicsmear
