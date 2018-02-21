/*
 * usb.c
 *
 *  Created on: 14 feb. 2018
 *      Author: erik.welander
 */
#include <usb.h>

const USB_device_descriptor_t USBdevDesc = {
    .bLength            = sizeof(USB_device_descriptor_t),
    .bDescriptorType    = 0x01,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize     = 0x40,
    .idVendor           = 0xDEAD,
    .idProduct          = 0xBEEF,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// uint16_t USBstringLangs[] = {0x0409};

uint8_t USBstringIndex[] =
{
		0x00,
		0x01,
		0x02,
		0x03
};


uint16_t LanguageString[] = {0x0304, 0x0409};
uint16_t ManufacturerString[] = {0x0300 | (21 * 2 + 2), 'S', 't', 'a', 'r', 'f', 'l', 'e', 'e', 't', ' ', 'E', 'n', 'g', 'i', 'n', 'e', 'e', 'r', 'i', 'n', 'g'};
uint16_t ProductString[] = {0x0300 | (22 * 2 + 2), 'H', 'e', 'i', 's', 'e', 'n', 'b', 'e', 'r', 'g', ' ', 'C', 'o', 'm', 'p', 'e', 'n', 's', 'a', 't', 'o', 'r'};
uint16_t SerialNumberString[] = {0x0300 | (6 * 2 + 2) ,'1', '1', '2', '3', '5', '8'};

uint16_t *USBstrings[] =
{
	&LanguageString[0],
	&ManufacturerString[0],
	&ProductString[0],
	&SerialNumberString[0]
};

typedef struct
{
	USB_configuration_descriptor_t configDesc;
	USB_interface_descriptor_t interfDesc;
	USB_endpoint_descriptor_t inEndpoint;
	USB_endpoint_descriptor_t outEndpoint;

} USB_combined_MS_descriptor_t;

const USB_combined_MS_descriptor_t USBcomboMSdesc = {
    .configDesc = {
        .bLength            = sizeof(USB_configuration_descriptor_t),
        .bDescriptorType    = 0x02,
        .wTotalLength      	= sizeof(USB_combined_MS_descriptor_t),
        .bNumInterfaces    	= 0x01,
        .bConfigValue 		= 0x01,
        .iConfig     		= 0x00,
        .bmAttributes       = 0x80,
        .bMaxPower          = 50,
    },
    .interfDesc = {
        .bLength            = sizeof(USB_interface_descriptor_t),
        .bDescriptorType    = 0x04,
        .bInterfaceNumber   = 0x00,
        .bAlternateSetting  = 0x00,
        .bNumEndpoints      = 0x02, // Endpoint zero is excluded
        .bInterfaceClass    = 0x08,	// Mass Storage
        .bInterfaceSubClass = 0x06, // SCSI Transparent
        .bInterfaceProtocol = 0x50,	// BBB
        .iInterface         = 0x00
    },
    .inEndpoint = {
    	.bLength            = sizeof(USB_endpoint_descriptor_t),
    	.bDescriptorType    = 0x05,
    	.bEndpointAddress   = 0x81, // Endpoint 1, direction IN
    	.bmAttributes       = 0x02, // Bulk endpoint
    	.wMaxPacketSize     = 0x40,
    	.bInterval          = 10    // polling interval / 2 ms
    },
    .outEndpoint = {
        .bLength            = sizeof(USB_endpoint_descriptor_t),
        .bDescriptorType    = 0x05,
        .bEndpointAddress   = 0x02,	// Endpoint 2, direction OUT
        .bmAttributes       = 0x02, // Bulk endpoint
        .wMaxPacketSize     = 0x40,
        .bInterval          = 10    // polling interval / 2 ms
    }
};

const uint8_t USB_MAX_LUN = 0;
