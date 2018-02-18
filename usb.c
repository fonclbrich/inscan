/*
 * usb.c
 *
 *  Created on: 15 feb. 2018
 *      Author: erik.welander
 */

#include "app_spec.h"
#include <debug.h>
#include <usb.h>

static USB_EP_block_t endPoints[] =
{

};

#ifdef DEBUG_USB
void dropUSBSetupPacket(USB_setup_packet_t *setupPacket)
{
	char numBuf[0x10];
	numBuf[0xF] = 0;
	numBuf[0xE] = '\n';
	debugSendString("bmRequestType: ");
	debugSendString(hex2str(setupPacket->bmRequestType, &numBuf[0xD]));

	debugSendString("bRequest: ");
	debugSendString(hex2str(setupPacket->bRequest, &numBuf[0xD]));

	debugSendString("wValue: ");
	debugSendString(hex2str(setupPacket->wValue, &numBuf[0xD]));

	debugSendString("wIndex: ");
	debugSendString(hex2str(setupPacket->wIndex, &numBuf[0xD]));

	debugSendString("wLength: ");
	debugSendString(hex2str(setupPacket->wLength, &numBuf[0xD]));
}
#endif

void USBCallback(uint16_t event)
{
	switch (event)
	{
	case USBresetCmd :
		debugSendString("Got Reset Command.\n");

		USBconfigEPs( 0, 0);
		USBsetAddress(0);
		USBresume();
	    debugSendString("EP0R at reset: ");
	    debugSendString(Dhex2str(USBstatusReg(0)));
	    debugSendString(" ");
	    debugSendString(Dhex2str(USBaddr()));
	    debugSendString(" ");
	    debugSendString(Dhex2str(USBglobalReg()));

	    debugSendString("\n");
		break;

	case USBsetupCmd :
	{
		debugSendString("Got Setup Command.\n");

		USB_setup_packet_t setup;

		USBepRead(0, &setup, sizeof(setup));
#ifdef DEBUG_USB
		dropUSBSetupPacket(&setup);
#endif

		USBdisable();
		break;
	}

	case USBtransIn :
		debugSendString("Sent...\n");
		USBconfirmSent(0);
		break;

	default:
		debugSendString("Other USB event: ");
		debugSendString(Dhex2str(event));
		debugSendString("\n");
		USBdisable();
		break;
	}
}
