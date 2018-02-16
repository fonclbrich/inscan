/*
 * usb.c
 *
 *  Created on: 15 feb. 2018
 *      Author: erik.welander
 */

#include "app_spec.h"
#include <debug.h>
#include <usb.h>

void USBCallback(uint16_t event)
{
	switch (event)
	{
	case USBresetCmd :
		debugSendString("Got Reset Command.\n");
		USBdisable();
		break;

	case USBsetupCmd :
		debugSendString("Got Setup Command.\n");
		USBdisable();
		break;

	default:
		break;
	}
}
