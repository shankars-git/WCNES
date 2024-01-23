configuration T1AppC
{
}
implementation
{
  components MainC, T1C, LedsC;

  T1C -> MainC.Boot;

  T1C.Leds -> LedsC;

  components new TimerMilliC() as TimerAccel;
	T1C.TimerAccel -> TimerAccel;

	components new ADXL345C();
  T1C.xyzaxis -> ADXL345C.XYZ;
  T1C.AccelControl -> ADXL345C.SplitControl;

}

