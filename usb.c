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
extern uint16_t *USBstringDesc[];
extern USB_EP_block_t EPConfig;

#ifdef DEBUG_USB
void dumpUSBSetupPacket(USB_setup_packet_t *setupPacket);
#endif

void USBCallback(uint16_t event)
{
	static uint8_t newAddress = 0;
	uint8_t what = event & 0x00FF;
	uint8_t EPid = event >> 8;

#ifdef DEBUG_USB
		debugSendString("New event: ");
		debugSendString(Dhex2str(event));
		debugSendString("\n");
#endif

	switch (what)
	{
	case USBresetCmd :
#ifdef DEBUG_USB
		debugSendString("Got Reset Command.\n");
#endif
	//	USBconfigEPs( 0, 0);
	//	USBsetAddress(0);
	//	USBresume();
		return;

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
					debugSendString("Sending device descriptor.\n");
#endif
					USBepSend(0, &USBdevDesc, sizeof(USBdevDesc));
					return;

				case USB_SETUP_DESC_CONFIGURATION :
#ifdef DEBUG_USB
					debugSendString("Sending configuration descriptor.\n");
					dumpUSBSetupPacket(&setup);
#endif
					USBepSend(0, &USBcomboMSdesc, setup.wLength < sizeof(USBcomboMSdesc) ? sizeof(USBcomboMSdesc.configDesc) : sizeof(USBcomboMSdesc));
					return;

				case USB_SETUP_DESC_STRING :
				{
					uint8_t stringIndex = setup.wValue & 0xFF;

					if (stringIndex <= 3)
					{
#ifdef DEBUG_USB
						debugSendString("Sending string descriptor (");
						debugSendString(Dhex2str(stringIndex));
						debugSendString("). Length: ");
						debugSendString(Dhex2str(*USBstringDesc[stringIndex] & 0xFF));
						debugSendString("\n");

#endif

						USBepSend(0, USBstringDesc[stringIndex], *USBstringDesc[stringIndex] & 0xFF);
						return;
					}
				}
				default:
					break;
				}

				break;

			default:
				break;
			}
			break;

		case USB_DIR_HOST_TO_DEVICE | USB_RECIPIENT_DEVICE :
			switch (setup.bRequest)
			{
			case USB_SETUP_SET_ADDRESS :
				newAddress = setup.wValue;
				USBacknowledge(0);
				return;

			case USB_SETUP_SET_CONFIGURATION :
	//			if (setup.wValue != 0x0001) break;
	//			USBconfigEPs(&EPConfig, 2);
				debugSendString("New Config set.\n");
				USBacknowledge(0);

				return;

			default:
				break;
			}

		default:
			break;
		}

		debugSendString("Unhandeled Setup Command.\n");
#ifdef DEBUG_USB
		dumpUSBSetupPacket(&setup);
#endif
		USBdisable();
		break;
	}

	case USBtransIn :
		if (newAddress != 0)
		{
			USBsetAddress(newAddress);
			newAddress = 0;
#ifdef DEBUG_USB
			debugSendString("New address set to ");
			debugSendString(Dhex2str(USBgetAddress()));
			debugSendString("\n");
		}
		else
		{
			debugSendString("Sent on EP");
			debugSendString(Dhex2str(EPid));
			debugSendString("...\n");
#endif
		}
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
void dumpUSBSetupPacket(USB_setup_packet_t *setupPacket)
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
