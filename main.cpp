/******************************************************************************
SparkFun_9DOF_Edison_Block_Example.cpp
Example code for the 9DOF Edison Block
14 Jul 2015 by Mike Hord
https://github.com/sparkfun/SparkFun_9DOF_Block_for_Edison_CPP_Library

Demonstrates the major functionality of the SparkFun 9DOF block for Edison.

** Supports only I2C connection! **

Development environment specifics:
  Code developed in Intel's Eclipse IOT-DK
  This code requires the Intel mraa library to function; for more
  information see https://github.com/intel-iot-devkit/mraa

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include "mraa.hpp"
#include <vector>
#include <iostream>
#include <unistd.h>
#include "SFE_LSM9DS0.h"
#include <fstream>

//#define GNUPLOT_ENABLE_PTY
#include "gnuplot-iostream.h"

using namespace std;

int main()
{
  //Gnuplot gp(stdout);
  //Gnuplot gp;
  Gnuplot gp("tee out.gp | gnuplot -persist");

  gp << "set terminal png"<<endl;
  gp << "set output out.png"<<endl;
  cout << "DEBUG: gp configured\n";
  int buffSize = 1000;
  std::vector<float> x_pts(buffSize, 0.0);

  LSM9DS0 *imu;
  imu = new LSM9DS0(0x6B, 0x1D);
  imu -> setAccelScale(imu -> A_SCALE_2G);
 
  cout << "DEBUG: IMU configured.\n";
  // The begin() function sets up some basic parameters and turns the device
  //  on; you may not need to do more than call it. It also returns the "whoami"
  //  registers from the chip. If all is good, the return value here should be
  //  0x49d4. Here are the initial settings from this function:
  //  Gyro scale:        245 deg/sec max
  //  Xl scale:          4g max
  //  Mag scale:         2 Gauss max
  //  Gyro sample rate:  95Hz
  //  Xl sample rate:    100Hz
  //  Mag sample rate:   100Hz
  // These can be changed either by calling appropriate functions or by
  //  pasing parameters to the begin() function. There are named constants in
  //  the .h file for all scales and data rates; I won't reproduce them here.
  //  Here's the list of fuctions to set the rates/scale:
  //  setMagScale(mag_scale mScl)      setMagODR(mag_odr mRate)
  //  setGyroScale(gyro_scale gScl)    setGyroODR(gyro_odr gRate)
  //  setAccelScale(accel_scale aScl)  setGyroODR(accel_odr aRate)
  // If you want to make these changes at the point of calling begin, here's
  //  the prototype for that function showing the order to pass things:
  //  begin(gyro_scale gScl, accel_scale aScl, mag_scale mScl, 
	//				gyro_odr gODR, accel_odr aODR, mag_odr mODR)
  uint16_t imuResult = imu->begin();
  //cout<<hex<<"Chip ID: 0x"<<imuResult<<dec<<" (should be 0x49d4)"<<endl;

  bool newAccelData = false;
  bool overflow = false;

  // Loop and report data
  while (1)
  {
    // First, let's make sure we're collecting up-to-date information. The
    //  sensors are sampling at 100Hz (for the accelerometer, magnetometer, and
    //  temp) and 95Hz (for the gyro), and we could easily do a bunch of
    //  crap within that ~10ms sampling period.
    while ((newAccelData) != true)
    {
      if (newAccelData != true)
      {
        newAccelData = imu->newXData();
      }
    }
    cout << "DEBUG: Hot data, come and get it!\n";
    newAccelData = false;

    // Of course, we may care if an overflow occurred; we can check that
    //  easily enough from an internal register on the part. There are functions
    //  to check for overflow per device.
    overflow = imu->xDataOverflow();

    if (overflow)
    {
      cerr<<"WARNING: DATA OVERFLOW!!!"<<endl;
    }

    // Calling these functions causes the data to be read from the IMU into
    //  10 16-bit signed integer public variables, as seen below. There is no
    //  automated check on whether the data is new; you need to do that
    //  manually as above. Also, there's no check on overflow, so you may miss
    //  a sample and not know it.
    
    cout << "DEBUG: Pulling the hot data...\n";
    imu->readAccel();
    cout << "DEBUG: Pulled the hot data.\n";

    cout << "DEBUG: Storing the new data...\n";
    x_pts.push_back(imu->calcAccel(imu->ax));
    cout << "DEBUG: Stored the new data (" <<x_pts.back() << ").\n";
    
    cout << "DEBUG: Resizing the vector...\n";
    x_pts.erase(x_pts.begin());
    cout << "DEBUG: Resized the vector.\n";

    cout << "DEBUG: Plotting data...\n";
    gp << "plot '-' using (column(0)):1 with lines"<<endl;
    gp.send1d(x_pts);
    
    cout << "DEBUG: Data plotted.\n";

    //cout<<"Accel y: "<<imu->calcAccel(imu->ay)<<" g"<<endl;
    //cout<<"Accel z: "<<imu->calcAccel(imu->az)<<" g"<<endl;
    //sleep(1);
  }

	return MRAA_SUCCESS;
}
