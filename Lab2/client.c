#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/adxl345.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#define ACCM_READ_INTERVAL    CLOCK_SECOND
#define LED_INT_ONTIME        CLOCK_SECOND/2

static process_event_t ledOff_event;

/* Declare our "main" process, the client process*/
PROCESS(accel_process, "Accelorometer process");
PROCESS(client_process, "Clicker client");
PROCESS(led_process, "LED handling process");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&accel_process,&client_process,&led_process);

/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
}

void
print_int(uint16_t reg){
#define ANNOYING_ALWAYS_THERE_ANYWAY_OUTPUT 0
#if ANNOYING_ALWAYS_THERE_ANYWAY_OUTPUT
  if(reg & ADXL345_INT_OVERRUN) {
    printf("Overrun ");
  }
  if(reg & ADXL345_INT_WATERMARK) {
    printf("Watermark ");
  }
  if(reg & ADXL345_INT_DATAREADY) {
    printf("DataReady ");
  }
#endif
  if(reg & ADXL345_INT_FREEFALL) {
    printf("Freefall ");
  }
  if(reg & ADXL345_INT_INACTIVITY) {
    printf("InActivity ");
  }
  if(reg & ADXL345_INT_ACTIVITY) {
    printf("Activity ");
  }
  if(reg & ADXL345_INT_DOUBLETAP) {
    printf("DoubleTap ");
  }
  if(reg & ADXL345_INT_TAP) {
    printf("Tap ");
  }
  printf("\n");
}
/*---------------------------------------------------------------------------*/
/* accelerometer free fall detection callback */

void
accm_ff_cb(uint8_t reg){
  leds_on(LEDS_BLUE);
  process_post(&led_process, ledOff_event, NULL);
  printf("~~[%u] Freefall detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
  print_int(reg);
}
/*---------------------------------------------------------------------------*/
/* accelerometer tap and double tap detection callback */

void
accm_tap_cb(uint8_t reg){
  process_post(&led_process, ledOff_event, NULL);
  if(reg & ADXL345_INT_DOUBLETAP){
    leds_on(LEDS_GREEN);
    printf("~~[%u] DoubleTap detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
  } else {
    leds_on(LEDS_RED);
    printf("~~[%u] Tap detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
  }
  print_int(reg);
}

static struct etimer ledETimer;
PROCESS_THREAD(led_process, ev, data) {
  PROCESS_BEGIN();
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev == ledOff_event);
    etimer_set(&ledETimer, LED_INT_ONTIME);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&ledETimer));
    leds_off(LEDS_RED + LEDS_GREEN + LEDS_BLUE);
  }
  PROCESS_END();
}

/* Our main process. */
PROCESS_THREAD(client_process, ev, data) {
	static char payload[] = "Button_Press";

	PROCESS_BEGIN();

	/* Activate the button sensor. */
	SENSORS_ACTIVATE(button_sensor);

	/* Initialize NullNet */
	nullnet_buf = (uint8_t *)&payload;
	nullnet_len = sizeof(payload);
	nullnet_set_input_callback(recv);

	/* Loop forever. */
	while (1) {
		/* Wait until an event occurs. If the event has
		 * occured, ev will hold the type of event, and
		 * data will have additional information for the
		 * event. In the case of a sensors_event, data will
		 * point to the sensor that caused the event.
		 * Here we wait until the button was pressed. */
		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			data == &button_sensor);

		/* Copy the string "hej" into the packet buffer. */
		memcpy(nullnet_buf, &payload, sizeof(payload));
    nullnet_len = sizeof(payload);

		/* Send the content of the packet buffer using the
		 * broadcast handle. */
		NETSTACK_NETWORK.output(NULL);
		printf("Button Alarm Sent to Base Station \n");
	}

	PROCESS_END();
}

static struct etimer et;
uint16_t error=100; //Set the error value

PROCESS_THREAD(accel_process, ev, data) {
  PROCESS_BEGIN();
  {
    int16_t x, y, z;
	int16_t x_old,y_old,z_old;
	x_old =0;
	y_old = 0;
	z_old = 0;
	static char payload[] = " _AXIS";
	//static char payloadx[] = "X_AXIS";
	//static char payloady[] = "Y_AXIS";
	//static char payloadz[] = "Z_AXIS";

	/* Register the event used for lighting up an LED when interrupt strikes. */
    ledOff_event = process_alloc_event();

    /* Start and setup the accelerometer with default values, eg no interrupts enabled. */
    accm_init();

    /* Register the callback functions for each interrupt */
    ACCM_REGISTER_INT1_CB(accm_ff_cb);
    ACCM_REGISTER_INT2_CB(accm_tap_cb);

    /* Set what strikes the corresponding interrupts. Several interrupts per pin is 
      possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */
    accm_set_irq(ADXL345_INT_FREEFALL, ADXL345_INT_TAP + ADXL345_INT_DOUBLETAP);
	/* Initialize NullNet */
	nullnet_buf = (uint8_t *)&payload;
	nullnet_len = sizeof(payload);
	nullnet_set_input_callback(recv);

    while (1) {
	    x = accm_read_axis(X_AXIS);
	    y = accm_read_axis(Y_AXIS);
	    z = accm_read_axis(Z_AXIS);
	    printf("x: %d y: %d z: %d\n", x, y, z);
		
      etimer_set(&et, ACCM_READ_INTERVAL);
	  if (abs(x_old - x) > error) 
    	{
		 strcpy(payload, "X_AXIS");
		 		leds_toggle(LEDS_RED);

		}
		if (abs(y_old - y) > error) 
    	{
		 strcpy(payload, "Y_AXIS");
		 		leds_toggle(LEDS_GREEN);

		}
			  if (abs(z_old - z) > error) 
    	{
		 strcpy(payload, "Z_AXIS");
		 		leds_toggle(LEDS_RED);

		}
				
		x_old = x;
		y_old = y;
		z_old = z;
		
	  memcpy(nullnet_buf, &payload, sizeof(payload));
    nullnet_len = sizeof(payload);

		/* Send the content of the packet buffer using the
		 * broadcast handle. */
		NETSTACK_NETWORK.output(NULL);
		printf("Accelerometer Alarm Sent to Base Station \n");

      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }
  }
  PROCESS_END();
}
