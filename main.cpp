/******************************************************************************
axelwrap
First commit 06 Apr 2016 by Mike Seery

https://github.com/roleohibachi/axlewrap

An Intel Edison motion tracking tool, intended for sports coaching. Uses Sparkfun 9DOF and gnuplot-iostream.

Development environment specifics:
  Uses i2c only, as it uses Sparkfun's library for comms.
  Code developed in vim, by rubbing two sticks together.
  This code requires the Intel mraa library to function; for more
  information see https://github.com/intel-iot-devkit/mraa

This code is protected by an Apache v2.0 license. See LICENSE for more information.
Distributed as-is; no warranty is given. Which is probably good, because I'm a rank amateur coder. If you can do better, please offer your help to the project!

******************************************************************************/

#include "mraa.hpp"
#include <vector>
#include <iostream>
#include <unistd.h>
#include "SFE_LSM9DS0.h"
#include <fstream>

#include "gnuplot-iostream.h"

using namespace std;

int buffSize = 100;
std::vector<float> x_pts(buffSize, 0.0);
LSM9DS0 *imu;
bool newAccelData = false;
const uint16_t chipID = 0x49d4;

void getData(int howMany){

  bool overflow = false;
    // Of course, we may care if an overflow occurred; we can check that
    //  easily enough from an internal register on the part. There are functions
    //  to check for overflow per device.
    overflow = imu->xDataOverflow();

  if (overflow)
  {
    cerr<<"WARNING: Data overflow has occurred."<<endl;
  }

  cout << "DEBUG: getting " << howMany << " datapoints.\n";

  for(int i=0;i<howMany;i++){
    while ((newAccelData) != true){
      if (newAccelData != true)
      {
        newAccelData = imu->newXData();
      }
    }
    newAccelData = false;

    cout << "." << flush;//DEBUG

    // Calling these functions causes the data to be read from the IMU into
    //  10 16-bit signed integer public variables, as seen below. There is no
    //  automated check on whether the data is new; you need to do that
    //  manually as above. Also, there's no check on overflow, so you may miss
    //  a sample and not know it.
    
    // cout << "DEBUG: Pulling the hot data...\n";
    imu->readAccel();
    // cout << "DEBUG: Pulled the hot data.\n";

    // cout << "DEBUG: Storing the new data...\n";
    x_pts.push_back(imu->calcAccel(imu->ax));
    // cout << "DEBUG: Stored the new data (" <<x_pts.back() << ").\n";
    
    // cout << "DEBUG: Resizing the vector...\n";
    x_pts.erase(x_pts.begin());
    // cout << "DEBUG: Resized the vector.\n";
  }
  cout << endl;
}


int main()
{
  //Gnuplot gp(stdout);
  //Gnuplot gp;
  //Gnuplot gp(fopen("script.gp", "w"));
  Gnuplot gp("tee out.gp | gnuplot");

  gp << "set terminal png\n";

  cout << "DEBUG: gp configured\n";

  imu = new LSM9DS0(0x6B, 0x1D); //the pinout addresses of the gyro and xm, respectively
  imu -> setAccelScale(imu -> A_SCALE_2G);
  imu -> setAccelODR(imu -> A_ODR_800); // 800 Hz (0x9)
  //cout << "DEBUG: IMU configured.\n";

  uint16_t imuResult = imu->begin();
  //cout<<hex<<"Chip ID: 0x"<<imuResult<<dec<<" (should be 0x49d4)"<<endl;
//TODO fix this test to exit if the chip is bad
  if(imuResult != chipID){
    cout << "The chip is enabled.\n";
  }else{
    cout << "The chip reported ID " << imuResult << endl;
  }

  //cout << "DEBUG: filling buffer...\n";
  getData(buffSize); //fill with data, be patient.
  //cout << "DEBUG: Buffer full!\n";

  // Loop and report data
  while (1)
  {
    getData(10);
    //cout << "DEBUG: Plotting data...\n";
    rename("/var/www/stage.png","/var/www/out.png");
    gp << "set output \"/var/www/stage.png\"\n";
    gp << "plot '-' using (column(0)):1 with lines\n";
    gp.send1d(x_pts);
    //gp << "plot" << gp.file1d(x_pts, "in.dat") << "with lines\n";

    //cout << "DEBUG: plotted up through datapoint " << x_pts.back() << endl;

    //cout<<"Accel y: "<<imu->calcAccel(imu->ay)<<" g"<<endl;
    //cout<<"Accel z: "<<imu->calcAccel(imu->az)<<" g"<<endl;
    //sleep(1);
  }

	return MRAA_SUCCESS;
}


