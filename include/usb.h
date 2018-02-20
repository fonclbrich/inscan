/*
 * usb.h
 *
 *  Created on: 30 jan. 2018
 *      Author: erik.welander
 */

#ifndef USB_H_
#define USB_H_

#include <stdint.h>

#define USBidle		0x00
#define USBresetCmd	0x01
#define USBsetupCmd	0x02
#define USBtransOut	0x03
#define USBtransIn	0x04

#define USB_EP_BULK         (0x00 << 9)
#define USB_EP_CONTROL      (0x01 << 9)
#define USB_EP_ISO          (0x02 << 9)
#define USB_EP_INTERRUPT    (0x03 << 9)

#define USB_CLEAR_FEATURE		0x0102
#define USB_GET_DESCRIPTOR		0x0680
#define USB_SET_ADDRESS			0x0500
#define USB_SET_CONFIGURATION	0x0900
#define USB_GET_MAX_LUN			0xFEA1

#define USB_RECIPIENT_DEVICE	0x00
#define USB_RECIPIENT_INTERFACE	0x01
#define USB_RECIPIENT_ENDPOINT	0x02
#define USB_RECIPIENT_OTHER		0x03
#define USB_TYPE_STANDARD		0x00
#define USB_TYPE_CLASS			0x20
#define USB_TYPE_VENDOR			0x40
#define USB_TYPE_RESERVED		0x60
#define USB_DIR_DEVICE_TO_HOST	0x80
#define USB_DIR_HOST_TO_DEVICE	0x80

#define USB_SETUP_CLEAR_FEATURE			0x01
#define USB_SETUP_SET_FEATURE			0x03
#define USB_SETUP_SET_ADDRESS			0x05
#define USB_SETUP_GET_DESCRIPTOR		0x06
#define USB_SETUP_SET_DESCRIPTOR		0x07
#define USB_SETUP_GET_CONFIGURATION		0x08
#define USB_SETUP_SET_CONFIGURATION		0x09
#define USB_SETUP_SET_HID_IDLE			0x0A

#define USB_SETUP_DESC_DEVICE			0x01
#define USB_SETUP_DESC_CONFIGURATION	0x02
#define USB_SETUP_DESC_STRING			0x03
#define USB_SETUP_DESC_HID				0x22
typedef struct
{
	uint16_t	features;
	uint8_t		EPid;
	uint8_t		bufSizeRX;
	uint8_t		bufSizeTX;
} USB_EP_block_t;

typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed)) USB_setup_packet_t;


/* Device Descriptor */
typedef struct {
    uint8_t 	bLength;
    uint8_t 	bDescriptorType;
    uint16_t 	bcdUSB;
    uint8_t 	bDeviceClass;
    uint8_t 	bDeviceSubClass;
    uint8_t 	bDeviceProtocol;
    uint8_t 	bMaxPacketSize;
    uint16_t 	idVendor;
    uint16_t 	idProduct;
    uint16_t 	bcdDevice;
    uint8_t 	iManufacturer;
    uint8_t 	iProduct;
    uint8_t 	iSerialNumber;
    uint8_t 	bNumConfigurations;
} __attribute__((packed)) USB_device_descriptor_t;

/* Configuration Descriptor */
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigValue;
    uint8_t iConfig;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} __attribute__((packed)) USB_configuration_descriptor_t;

/* Interface Descriptor */
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} USB_interface_descriptor_t;

/* Endpoint Descriptor */
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __attribute__((packed)) USB_endpoint_descriptor_t;

typedef struct
{
	uint32_t	signature;
	uint32_t	tag;
	uint32_t	dataTransferLength;
	uint8_t		flags;
	uint8_t		LUN;
	uint8_t		CBlength;
	uint8_t		commandBlock[0x10];
} __attribute__((packed)) USB_command_block_wrapper;


void USBinit();
void USBdisable();
void USBresume();
void USBpause();
void USBsetAddress(uint8_t newAddress);
void USBconfigEPs(USB_EP_block_t *EPs, int nEP);
int USBepRead(int EPid, void *buf, int len);
int USBepSend(int EPid, const void *src, int len);
void USBacknowledge(int EPid);
void USBconfirmSent(int EPid);

// #ifndef USBAppCallback
// #error "Implementation required for USB Reset without Callback."
// uint16_t USBgetEvent();
// #endif

uint16_t USBstatusReg(int EPid);
uint16_t USBglobalReg();
uint16_t USBaddr();
#endif /* USB_H_ */
