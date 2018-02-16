/*
 * scsi.c
 *
 *  Created on: 13 feb. 2018
 *      Author: erik.welander
 */
#include "scsi.h"
#include "stdint.h"

#include <usb.h>
#include <debug.h>

extern const uint8_t USB_MAX_LUN;

int SCSIhandleCommandBlock(uint8_t *CB, uint8_t len);

static const uint8_t inquiryData[0x24] =
{
		0x00,	/* Device connected, Direct access */
		0x80,	/* Removable */
		0x00, 	/* Version */
		0x02, 	/* Respose format etc. Has to be 2 */
		0x1F,	/* Additional length */
		0x00,	/* Reserved */
		0x00,	/* Special features ... */
		0x00,	/* ... see the docs */
		'S', 't', 'a', 'r', ' ', 'E', 'n', 0,
		'Q', 'u', 'a', 'n', 't', 'u', 'm', ' ', 'D', 'r', 'i', 'v', 'e', 0, 0, 0,
		'0', '.', '0', '1'
};

#ifdef DEBUG_USB
void dumpCBW(USB_command_block_wrapper *cbw)
{
	debugSendString("signature: ");
	debugSendString(Dhex2str(cbw->signature));

	debugSendString("\ntag: ");
	debugSendString(Dhex2str(cbw->tag));

	debugSendString("\ndataTransferLength: ");
	debugSendString(Dhex2str(cbw->dataTransferLength));

	debugSendString("\nflags: ");
	debugSendString(Dhex2str(cbw->flags));

	debugSendString("\nLUN: ");
	debugSendString(Dhex2str(cbw->LUN));

	debugSendString("\nCBlength ");
	debugSendString(Dhex2str(cbw->CBlength));

	debugSendString("\nCommandBlock: ");

	for (int n = 0; n != cbw->CBlength; n++)
	{
		debugSendString(Dhex2str(cbw->commandBlock[n]));
		debugSendString(" ");
	}

	debugSendString("\n");
}

#endif

int USBhandleCBW(USB_command_block_wrapper *cbw)
{
	if (CBW_SIGN != cbw->signature  || USB_MAX_LUN < cbw->LUN || 0x10 < cbw->CBlength  || 0 == cbw->CBlength)
	{
		debugSendString("Data makes no sense:\n");
#ifdef DEBUG_USB
		dumpCBW(cbw);
#endif

	return -1;
	}

	return SCSIhandleCommandBlock(cbw->commandBlock, cbw->CBlength);
}


int SCSIhandleCommandBlock(uint8_t *CB, uint8_t len)
{
	switch (CB[0])
	{
	case SCSI_CMD_INQUIRY:
		if ((CB[1] & 3) != 0 || CB[2] != 0)
		{
#ifdef DEBUG_USB
			debugSendString("Strange flags in SCSI INQUIRY request!");
#endif
			return 0x10;
		}

		USBbulkSend(&inquiryData, sizeof(inquiryData));
		return 0;

	default:
		return 1;
	}
}
