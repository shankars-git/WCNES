/* A Simple Accelerometer Example
 *
 * Values only in the x-axis are detected in the following example.
 * For your Lab1, use extend the code for Y and Z axes.
 * Finally, interface them with a button so that the sensing starts onlt after the press of a button.
 *
 */
 
#include "UserButton.h"
#include "printfZ1.h"
#include "ADXL345.h"

module T1C @safe()
{
  	uses interface Leds;
  	uses interface Boot;

  	/* We use millisecond timer to check the shaking of client.*/
	uses interface Timer<TMilli> as TimerAccel;

  	/*Accelerometer Interface*/
	uses interface Read<adxl345_readxyt_t> as xyzaxis;
	uses interface SplitControl as AccelControl;
}


implementation
{
    uint16_t error=100; //Set the error value
    uint16_t x,y,z;

    event void Boot.booted() 
    {
		call AccelControl.start(); //Starts accelerometer
   		call TimerAccel.startPeriodic(1000); //Starts timer

    }

    event void AccelControl.startDone(error_t err)
	{
		printfz1("  +  Accelerometer Started\n");
		x = 0;
		y = 0;
		z = 0;
		printfz1_init();
	}

	event void AccelControl.stopDone(error_t err) 
	{
		printfz1("Accelerometer Stopped\n");

	}

	event void TimerAccel.fired()
	{
		call xyzaxis.read(); //Takes input from the x y z axis of the accelerometer
	}
	

    event void xyzaxis.readDone(error_t result, adxl345_readxyt_t data)
	{
		int16_t signedDatax = (int16_t)data.x_axis;
		int16_t signedDatay = (int16_t)data.y_axis;
		int16_t signedDataz = (int16_t)data.z_axis;
		printfz1("  +  X (%d) ", signedDatax);
		printfz1("  +  Y (%d) ", signedDatay);
		printfz1("  +  Z (%d) ", signedDataz);
		
		if (abs(x - signedDatax) > error) 
    	{
      		call Leds.led0On(); //LED correponding to the x-axis
		}
    
    	else
    	{
      		call Leds.led0Off(); //If difference is less than the error turn the LED off.
    	}
			if (abs(y - signedDatay) > error) 
    	{
      		call Leds.led1On(); //LED correponding to the x-axis
		}
    
    	else
    	{
      		call Leds.led1Off(); //If difference is less than the error turn the LED off.
    	}
			if (abs(z - signedDataz) > error) 
    	{
      		call Leds.led2On(); //LED correponding to the x-axis
		}
    
    	else
    	{
      		call Leds.led2Off(); //If difference is less than the error turn the LED off.
    	}
		x = signedDatax; //Store current sensor input to compare with the next.  
		y = signedDatay; 
		z = signedDataz; 
	}
}