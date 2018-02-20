/*
 * usb.c
 *
 *  Created on: 15 feb. 2018
 *      Author: erik.welander
 */

#include "app_spec.h"
#include <debug.h>
#include <usb.h>

typedef struct
{
	USB_configuration_descriptor_t configDesc;
	USB_interface_descriptor_t interfDesc;
	USB_endpoint_descriptor_t inEndpoint;
	USB_endpoint_descriptor_t outEndpoint;
} USB_combined_MS_descriptor_t;


extern const USB_device_descriptor_t USBdevDesc;
extern const USB_combined_MS_descriptor_t USBcomboMSdesc;
extern uint16_t *USBstrings[];

static USB_EP_block_t endPoints[] =
{

};

#ifdef DEBUG_USB
void dropUSBSetupPacket(USB_setup_packet_t *setupPacket);
#endif

void USBCallback(uint16_t event)
{
	uint8_t what = event & 0x00FF;
	uint8_t EPid = event >> 8;

	switch (what)
	{
	case USBresetCmd :
#ifdef DEBUG_USB
		debugSendString("Got Reset Command.\n");
#endif
		USBconfigEPs( 0, 0);
		USBsetAddress(0);
		USBresume();
		break;

	case USBsetupCmd :
	{
		if (0 != EPid)
		{
			debugSendString("Incorrect SETUP on EP");
			debugSendString(Dhex2str(EPid));
			debugSendString(" !\n");
			USBdisable();
			break;
		}
		USB_setup_packet_t setup;

		USBepRead(0, &setup, sizeof(setup));

		switch (setup.bmRequestType)
		{

		case USB_DIR_DEVICE_TO_HOST | USB_RECIPIENT_DEVICE :

			switch (setup.bRequest)
			{
			case USB_SETUP_GET_DESCRIPTOR :

				switch (setup.wValue >> 8)
				{
				case USB_SETUP_DESC_DEVICE :
#ifdef DEBUG_USB
					debugSendString("Sending device descriptor.");
#endif
					USBepSend(0, &USBdevDesc, sizeof(USBdevDesc));
					return;

				case USB_SETUP_DESC_CONFIGURATION :
#ifdef DEBUG_USB
					debugSendString("Sending configuration descriptor.");
#endif
					USBepSend(0, &USBcomboMSdesc, setup.wLength < sizeof(USBcomboMSdesc) ? sizeof(USBcomboMSdesc.configDesc) : sizeof(USBcomboMSdesc));
					return;

				case USB_SETUP_DESC_STRING : ;
					uint8_t stringIndex = setup.wValue & 0xFF;

					if (stringIndex <= 3)
					{
#ifdef DEBUG_USB
						debugSendString("Sending string descriptor(");
						debugSendString(Dhex2str(stringIndex));
						debugSendString(").\n");
#endif
						USBepSend(0, USBstrings[stringIndex], *USBstrings[stringIndex] & 0xFF);
						return;
					}

				default:
					break;
				}

				break;

			default:
				break;
			}
			break;

		default:
			break;
		}

		debugSendString("Unhandeled Setup Command.\n");
#ifdef DEBUG_USB
		dropUSBSetupPacket(&setup);
#endif
		USBdisable();
		break;
	}

	case USBtransIn :
#ifdef DEBUG_USB
		debugSendString("Sent on EP");
		debugSendString(Dhex2str(EPid));
		debugSendString("...\n");
#endif
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
