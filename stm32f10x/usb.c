/*
 * usb.c
 *
 *  Created on: 30 jan. 2018
 *      Author: erik.welander
 */
#include <stm32f10x.h>
#include "usb_def.h"
#include <usb.h>
#include <debug.h>
#include <app_spec.h>

extern const USB_device_descriptor_t USBdevDesc;
extern const USB_combined_MS_descriptor_t USBcomboMSdesc;

// extern uint16_t USBstringLangs[];
extern char *USBstrings[];

uint8_t USBstringIndex[] =
{
		0x01,
		0x02,
		0x03
};

extern const uint8_t USB_MAX_LUN;

void USBmemRead(void *src, void *dest, int length)
{
	uint32_t *si = (uint32_t *)src;
	uint16_t *di = (uint16_t *)dest;

	while (length-- != 0) *di++ = *si++;
}

void USBinit()
{
    /* Enable necessary USB clocks */
    /* USB pins are located on port A. Enable the port A clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    /* Set USB prescaler to 1.5 (72 MHz / 3 * 2 = 48 MHz */
    RCC->CFGR &= ~RCC_CFGR_USBPRE;
    /* Enable USB clock */
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;

    /* Configure pins as push-pull outputs */
    GPIOA->CRH |= GPIO_CRH_MODE11;   //Output mode, max 50 MHz
    GPIOA->CRH &= ~GPIO_CRH_CNF11;   //Push-pull
    GPIOA->CRH |= GPIO_CRH_MODE12;   //Output mode, max 50 MHz
    GPIOA->CRH &= ~GPIO_CRH_CNF12;   //Push-pull

    /* Pull Data lines low */
    GPIOA->BRR = (1U << 11U) | (1U << 12U);

    for (uint16_t i = 0x100; i > 0; i--) {
        /* Minimal delay to allow unconnected appearance */
    }

    /* Setup the USB pins */
    /* PA11 is USB Data Minus */
    GPIOA->CRH |= GPIO_CRH_CNF11_1;  //Alternate function push-pull
    /* PA12 is USB Data Plus */
    GPIOA->CRH |= GPIO_CRH_CNF12_1;  //Alternate function push-pull
    /* Set pins high */
    GPIOA->BSRR = (1 << 11U) | (1 << 12U);

    USB->CNTR &= ~USB_CNTR_PDWN;     //Exit Power Down mode
    for (uint16_t i = 0x100; i > 0; i--) {
        /* Delay more than 1 us to allow startup time */
    }
    USB->CNTR &= ~USB_CNTR_FRES;     //Clear forced USB reset
    USB->ISTR = 0x00U;               //Clear any spurious interrupts

	/* Setup the stuff that wont change... */
	USB_BDT(USB_EP0)->ADDR_TX = USB_BDT(USB_EP0)->ADDR_RX= 0x0040;
	USB_BDT(USB_EP0)->COUNT_RX = USB_COUNT0_RX_BLSIZE | USB_COUNT0_RX_NUM_BLOCK_1;

    USB->BTABLE = 0x0000U;
    USB->EP0R = ( ((USB->EP0R & (USB_EP_STAT_TX | USB_EP_STAT_RX)) ^ (USB_RX_VALID | USB_TX_NAK))
    		|  USB_EP_CONTROL | USB_CLEAR_MASK ) & ~(USB_EP_DTOG_RX | USB_EP_DTOG_TX);

    /* Enable CTRM and RESET interrupts */
    USB->CNTR |= (USB_CNTR_CTRM | USB_CNTR_RESETM);
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn); //Enable the USB NVIC interrupts
}

void USBdisable()
{
	/* Should do a proper shotdown. Fix later */
#ifdef DEBUG_USB
	debugSendString("Shutting Down USB Services.\n");
#endif
	NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
}

void USBresume()
{
	 USB->DADDR |= USB_DADDR_EF;
}

void USBpause()
{
	USB->DADDR ^= (USB->DADDR & USB_DADDR_EF);
}

void USBsetAddress(uint8_t newAddress)
{
	USB->DADDR = (USB->DADDR & USB_DADDR_EF) | newAddress;
}

void USBconfigEPs(USB_EP_block_t *EPs, int nEP)
{
	int addrOffs = 0x0040;

	while (--nEP != 0)
	{
		uint16_t epr = EPs[nEP].EPid | EPs[nEP].features | USB_CLEAR_MASK;

		if (EPs[nEP].bufSizeRX != 0)
		{
			USB_BDT(EPs[nEP].EPid)->ADDR_RX = (addrOffs += 0x0040);
			USB_BDT(EPs[nEP].EPid)->COUNT_RX = USB_COUNT0_RX_BLSIZE | USB_COUNT0_RX_NUM_BLOCK_1;

			/* Allow incoming data */
			epr |= USB_RX_VALID;
		}

		if (EPs[nEP].bufSizeTX != 0)
		{
			USB_BDT(EPs[nEP].EPid)->ADDR_TX = (addrOffs += 0x0040);

			/* Set to not acknowledge, since we do not know what to send yet. */
			epr |= USB_RX_NAK;
		}

		USB_EP(EPs[nEP].EPid) = (USB_EP(EPs[nEP].EPid) ^ (USB_TOGGLE_MASK & epr)) | epr;
	}

}

int USBepRead(int EPid, void *buf, int len)
{
	if ((USB_EP(EPid) & USB_EP_STAT_RX) != USB_RX_NAK) return 0;

	int N = len < USB_COUNT0_RX_COUNT0_RX ? len : USB_COUNT0_RX_COUNT0_RX;

	uint32_t *src = (uint32_t *)(PMA_BASE + (USB_BDT(EPid)->ADDR_RX << 1U));
	uint16_t *dst = (uint16_t *) buf;

	for (int n = (N + 1) / 2; 0 != n; n--) *dst++ = *src++;

	/* Allow new incoming data */
	USB_EP(EPid) = (USB_EP(EPid) & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_RX;

	return N;
}

int USBepSend(int EPid, void *buf, int len)
{
	if ((USB_EP(EPid) & USB_EP_STAT_TX) != USB_TX_NAK )
	{
		debugSendString("Waiting to send on EP ");
		debugSendString(Dhex2str(EPid));
		debugSendString("\n");
		while ((USB_EP(EPid) & USB_EP_STAT_TX) != USB_TX_NAK);
	}

	uint32_t *dst = (uint32_t *) (PMA_BASE + (USB_BDT(EPid)->ADDR_TX << 1U));
	uint16_t *src = (uint16_t *) buf;

	for (int n = (len + 1) / 2; 0 != n; n--) *dst++ = *src++;

	USB_BDT(EPid)->COUNT_TX = len;
	USB_EP(EPid) = (USB_EP(EPid) & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;

	return len;
}

void USBacknowledge(int EPid)
{
	USB_BDT(EPid)->COUNT_TX = 0;
	USB_EP(EPid) = (USB_EP(EPid) & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;
}

void USB_LP_CAN1_RX0_IRQHandler()
{
	uint16_t ISTR = USB->ISTR;

    if (ISTR & USB_ISTR_RESET)
    {
        /* Reset Request */
#ifdef USBAppCallback
    	USBAppCallback(USBresetCmd);
#else
#error "Implementation required for USB Reset without Callback."
#endif
    	USB->ISTR = ~USB_ISTR_RESET; // Clear interrupt
    	return;
    }

    uint16_t event = (ISTR & USB_EP_EA) << 8;

    if (event == 0x0000 && (USB_EP(0) & USB_EP_SETUP) != 0)
    {
    	event |= USBsetupCmd;
    }
    else if ((ISTR & USB_ISTR_DIR) != 0)
    {
    	event |= USBtransOut;
    }
    else
    {
    	event |= USBtransIn;
    }
#ifdef USBAppCallback
    USBAppCallback(event);
#else
#error "Implementation required for USB Reset without Callback."
#endif
    USB->ISTR = ~USB_ISTR_RESET; // Clear interrupt
}

#ifdef DEBUG_USB
void dropUSBSetupPacket(Padded_USB_setup_packet_t *setupPacket)
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

#ifdef SVENBERTIL
static uint8_t newUSBaddress = 0;

void USB_EP0_Handler(uint16_t istr)
{
	if (istr & USB_ISTR_DIR)
	{
		/* Received data indicated (RX bit set) */

		if ((USB->EP0R & USB_EP_SETUP) != 0)
		{
			/* Setup requested. */

			/* Clear RX bit */
			USB->EP0R = USB->EP0R & ~(USB_TOGGLE_MASK | USB_EP_CTR_RX | USB_EP_CTR_TX);
			USB->EP0R = USB->EP0R  ^ (USB_RX_NAK | USB_TX_NAK  | USB_EP_DTOG_TX);

			/* Start by reading info from the EP Buffer Descriptor */

			Padded_USB_setup_packet_t *setupPacket = (Padded_USB_setup_packet_t *) (PMA_BASE + (USB_BDT(USB_EP0)->ADDR_RX << 1U));

			uint16_t requestID = *((uint16_t *) (PMA_BASE + (USB_BDT(USB_EP0)->ADDR_RX << 1U)));

			switch (requestID)
			{

			case USB_GET_DESCRIPTOR:
				switch (setupPacket->wValue >> 8)
				{
				case USB_SETUP_DESC_DEVICE: ;
#ifdef DEBUG_USB
					debugSendString("Device descriptor requested.\n");
#endif
					/* Request for us to send the device descriptor */

					/* Begin with copying the descriptor to the USB EP0 TX buffer */ ;

					uint32_t *EPTXBufPtr = (uint32_t *)(PMA_BASE + (USB_BDT(USB_EP0)->ADDR_TX << 1U));
					uint16_t *devDesc = (uint16_t *) &USBdevDesc;
					uint32_t n = USBdevDesc.bLength / 2;

					while (0 != n--) *(EPTXBufPtr++) = *(devDesc++);

					/* Load the size of the packet into the EP0 descriptor */

					USB_BDT(0)->COUNT_TX = USBdevDesc.bLength;

					/* Allow transmission */

					USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;
					break;

				case USB_SETUP_DESC_CONFIG: ;
#ifdef DEBUG_USB
					debugSendString("Configuration descriptor requested.\n");
#endif
					/* Request for the configuration descriptor. */

					/* If size allows, send all the descriptors in one transmission, otherwise, just the configuration descriptor. */

					n = ( (uint16_t) setupPacket->wLength >= USBcomboMSdesc.configDesc.wTotalLength) ? USBcomboMSdesc.configDesc.wTotalLength : USBcomboMSdesc.configDesc.bLength;
					uint16_t *confDesc =  (uint16_t *) &USBcomboMSdesc;
					EPTXBufPtr = (uint32_t *)(PMA_BASE + (USB_BDT(USB_EP0)->ADDR_TX << 1U));

					USB_BDT(0)->COUNT_TX = n++;

					for (n /= 2; 0 != n; n--) *(EPTXBufPtr++) = *(confDesc++);

					/* Allow transmission */

					USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;

					break;

				case USB_SETUP_DESC_STRING: ;
#ifdef DEBUG_USB
					debugSendString("Request for string descriptor, index: ");
					debugSendString(Dhex2str(setupPacket->wValue & 0xFF));
					debugSendString("\n");
#endif

					/* Request to send a string */
					uint8_t stringIndex = setupPacket->wValue & 0xFF;

					if (stringIndex == 0)
					{

						EPTXBufPtr = ((uint32_t *)(PMA_BASE + (USB_BDT(USB_EP0)->ADDR_TX << 1U)));

						*EPTXBufPtr++ = 0x304;
						*EPTXBufPtr =0x0409;
						USB_BDT(0)->COUNT_TX = 4;

						USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;

						break;
					}

					for (n = 0; n != sizeof(USBstringIndex); n++)
					{
						if (USBstringIndex[n] == stringIndex)
						{
							char *strSrc = USBstrings[n];
							EPTXBufPtr = ((uint32_t *)(PMA_BASE + (USB_BDT(USB_EP0)->ADDR_TX << 1U))) + 1;

#ifdef DEBUG_USB
							debugSendString("Sending: ");
							debugSendString(strSrc);
							debugSendString("\n\n");
#endif

							for (n = 1; (0 != *strSrc); n++) *(EPTXBufPtr++) = *(strSrc++);

							*((uint32_t *)(PMA_BASE + (USB_BDT(USB_EP0)->ADDR_TX << 1U))) = (n *= 2) | 0x300;

							USB_BDT(0)->COUNT_TX = n;

							devDesc =  (uint16_t *)(PMA_BASE + (USB_BDT(USB_EP0)->ADDR_TX << 1U));

							USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;
#ifdef DEBUG_USB
							debugSendString(Dhex2str(USB->EP0R));
#endif

							break;
						}

					}

					if (n == sizeof(USBstringIndex))
					{
						/* We do not know what is requested; stall the transmission */
						USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_STAT_TX;
#ifdef DEBUG_USB
						debugSendString("Unhandeled string descriptor requested, index: ");
						debugSendString(Dhex2str(setupPacket->wValue && 0xFF));
						debugSendString("\n");
#endif
					}

					break;
					/* No break statement here since the loop is through and we still do not know what is the matter */

				default:
					/* We do not know what is requested; stall the transmission */
					USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_STAT_TX;
#ifdef DEBUG_USB
					debugSendString("USB: Unhandled GET_DESCRIPTOR\n");
					dropUSBSetupPacket(setupPacket);
					debugSendString("\n");
#endif

					break;
				}

				break;

			case USB_SET_ADDRESS:
#ifdef DEBUG_USB
				debugSendString("New address assigned: ");
				debugSendString(Dhex2str(setupPacket->wValue & 0x7F));
				debugSendString("\n");
#endif
				/* Save the new address */
				newUSBaddress = setupPacket->wValue & 0x7F;

				/*
				 * Send acknowledgment (0 bytes).
				 * When finished, this will cause an In-transaction interrupt, which will have to be caught
				 * */

				USB_BDT(0)->COUNT_TX = 0;
				USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;
				break;

			case USB_SET_CONFIGURATION:
#ifdef DEBUG_USB
				debugSendString("New configuration assigned: ");
				debugSendString(Dhex2str(setupPacket->wValue & 0x7F));
				debugSendString("\n");

				debugSendString("EP1R: ");
				debugSendString(Dhex2str(USB->EP1R));
				debugSendString("    EP2R: ");
				debugSendString(Dhex2str(USB->EP2R));
				debugSendString("\n");
#endif

				/* We only have one configuration, so just acknowledge */

				USB_BDT(0)->COUNT_TX = 0;
				USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;
				break;

			case USB_CLEAR_FEATURE:
#ifdef DEBUG_USB
				debugSendString("Clear Feature Requested.\n");
#endif
				if (setupPacket->wValue == 0)
				{
					/* Endpoint Halt : Acknowledge */
					USB_BDT(0)->COUNT_TX = 0;
					USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;
				}
				else
				{
					/* Otherwise: Stall */
					USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_STAT_TX;
				}
			break;

			case USB_GET_MAX_LUN:
#ifdef DEBUG_USB
				debugSendString("Get Max LUN Requested.\n");
#endif
				*((uint32_t *)(PMA_BASE + (USB_BDT(USB_EP0)->ADDR_TX << 1U))) = USB_MAX_LUN;
				USB_BDT(0)->COUNT_TX = 1;
				USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;

				break;

			default:
				/* We do not know what is requested; stall the transmission */
				USB->EP0R = (USB->EP0R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_STAT_TX;
#ifdef DEBUG_USB
				debugSendString("USB: Unhandled SETUP (");
				debugSendString(Dhex2str(requestID));
				debugSendString(").\n");
				dropUSBSetupPacket(setupPacket);
#endif
				break;
			}

		}

		else
		{
			/* Normal out transaction. */
			debugSendString("EP 0 Out-transaction. Should not happen - surrendering!\n");
			NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
		}

	}
	else
	{
		/*
		 * In transaction.
		 * This simply means we finished sending something
		 * */
#ifdef DEBUG_USB
		debugSendString("EP0 In.\n");
#endif
		if (newUSBaddress != 0)
		{
			/* New address obtained and acknowledged */

			/* Change the address in the register */
			USB->DADDR = newUSBaddress | USB_DADDR_EF;

			/* Restore newUSBaddress to avoid this section code to be executed for next packet */
			newUSBaddress = 0;
		}

		USB->EP0R = USB->EP0R & ~USB_TOGGLE_MASK & ~USB_EP_CTR_TX;

	}
}

enum EndpointState
{
	idle,
	sending
};

volatile enum EndpointState EP1state = idle;

void USB_EP1_Handler(uint16_t istr)
{
	if (istr & USB_ISTR_DIR)
	{
		debugSendString("Unexpected call RX for USB_EP1!!.\n");
		NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
	}
	else
	{
		EP1state = idle;
	}

}

void USB_EP2_Handler(uint16_t istr)
{
	if (istr & USB_ISTR_DIR)
	{
		/* Data received, now parse the command block wrapper */

		USB_command_block_wrapper CBW;

		USBmemRead((void *)(PMA_BASE + (USB_BDT(USB_EP2)->ADDR_RX << 1U)), &CBW, 0x10);


		/* Call application Handler to decide what to do */

		if (USBhandleCBW(&CBW) != 0)
		{
			NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);

		}
	}
	else
	{
		debugSendString("WTF.\n");
		NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
	}
}



void USBbulkSend(void *data, int length)
{
	/* Protect with simple guard for now. Later we might change this to a buffer system */

	if (EP1state == sending)
	{
#ifdef DEBUG_USB
		debugSendString("Waiting for clearance to send on EP1");
#endif
		while(EP1state == sending);
	}

	uint32_t *sendBuf = (uint32_t *) (PMA_BASE + (USB_BDT(USB_EP1)->ADDR_TX << 1U));

	uint16_t *src = (uint16_t *) data;

	for (int n = (length  + 1) / 2; n != 0; n--) *sendBuf++ = *src++;

	USB->EP1R = (USB->EP1R & ~USB_TOGGLE_MASK) | USB_CLEAR_MASK | USB_EP_FLIP_TX;
}
#endif
