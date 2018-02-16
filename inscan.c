#include <blinkLED.h>
#include <debug.h>
#include <timer.h>
#include <clock.h>
#include <usb.h>

int main(void)
{
	clockInit();

	blinkLEDinit();

	debugInit();

	timerInit();

	USBinit();

    while(1)
    {
    	waitSyncTimer();
    	blinkLEDset();

    	waitSyncTimer();
    	blinkLEDreset();
    }
}
