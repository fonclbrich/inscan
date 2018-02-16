
#ifndef STM_USB_def
#define STM_USB_def

/* Endpoint values */
#define USB_EP_DISABLED     0x00
#define USB_EP_STALL        0x01
#define USB_RX_NAK          (0x02 << 12)
#define USB_TX_NAK          (0x02 << 4)
#define USB_RX_VALID        (0x03 << 12)
#define USB_TX_VALID		(0x03 << 4)
#define USB_EP0             0x00
#define USB_EP1             0x01
#define USB_EP2             0x02
#define USB_EP3             0x03
#define USB_EP4             0x04
#define USB_EP5             0x05
#define USB_EP6             0x06
#define USB_EP7             0x07


/* EndPoint Registers Bit Definitions */
#define USB_EP_CTR_RX       0x8000      /* Correct RX Transfer */
#define USB_EP_DTOG_RX      0x4000      /* RX Data Toggle */
#define USB_EP_FLIP_RX		0x1000		/* Toggle TX VALID / NAK */
#define USB_EP_STAT_RX      0x3000      /* RX Status */
#define USB_EP_SETUP        0x0800      /* EndPoint Setup */
#define USB_EP_TYPE         0x0600      /* EndPoint Type */
#define USB_EP_KIND         0x0100      /* EndPoint Kind */
#define USB_EP_CTR_TX       0x0080      /* Correct TX Transfer */
#define USB_EP_DTOG_TX      0x0040      /* TX Data Toggle */
#define USB_EP_FLIP_TX		0x0010		/* Toggle TX VALID / NAK */
#define USB_EP_STAT_TX      0x0030      /* TX Status */
#define USB_EP_EA           0x000F      /* EndPoint Address */

#define USB_TOGGLE_MASK (USB_EP_DTOG_RX | USB_EP_STAT_RX |  USB_EP_DTOG_TX | USB_EP_STAT_TX)
#define USB_CLEAR_MASK (USB_EP_CTR_RX | USB_EP_CTR_TX)



typedef struct {
    __IO uint16_t EP0R;
    uint16_t RESERVED0;
    __IO uint16_t EP1R;
    uint16_t RESERVED1;
    __IO uint16_t EP2R;
    uint16_t RESERVED2;
    __IO uint16_t EP3R;
    uint16_t RESERVED3;
    __IO uint16_t EP4R;
    uint16_t RESERVED4;
    __IO uint16_t EP5R;
    uint16_t RESERVED5;
    __IO uint16_t EP6R;
    uint16_t RESERVED6;
    __IO uint16_t EP7R;
    uint16_t RESERVED7;
    uint32_t RESERVED8[8];
    __IO uint16_t CNTR;
    uint16_t RESERVED9;
    __IO uint16_t ISTR;
    uint16_t RESERVED10;
    __IO uint16_t FNR;
    uint16_t RESERVED11;
    __IO uint16_t DADDR;
    uint16_t RESERVED12;
    __IO uint16_t BTABLE;
    uint16_t RESERVED13;
} USB_TypeDef;

#define USB_BASE        ((uint32_t)0x40005C00)
#define USB_EP(n)       (*((uint16_t *) (USB_BASE + 4*n)))
#define USB             ((USB_TypeDef *) USB_BASE)

typedef struct {
    __IO uint16_t ADDR_TX;
    uint16_t RESERVED0;
    __IO uint16_t COUNT_TX;
    uint16_t RESERVED1;
    __IO uint16_t ADDR_RX;
    uint16_t RESERVED2;
    __IO uint16_t COUNT_RX;
    uint16_t RESERVED3;
} BDT_TypeDef;

#define PMA_BASE        ((uint32_t)0x40006000) /* Buffer Descriptor Table */
#define USB_BDT(n)      ((BDT_TypeDef *) (PMA_BASE + 16UL*n))
#define USB0_BDT        ((BDT_TypeDef *) (PMA_BASE + 16*0))
#define USB1_BDT        ((BDT_TypeDef *) (PMA_BASE + 16*1))
#define USB2_BDT        ((BDT_TypeDef *) (PMA_BASE + 16*2))
#define USB3_BDT        ((BDT_TypeDef *) (PMA_BASE + 16*3))
#define USB4_BDT        ((BDT_TypeDef *) (PMA_BASE + 16*4))
#define USB5_BDT        ((BDT_TypeDef *) (PMA_BASE + 16*5))
#define USB6_BDT        ((BDT_TypeDef *) (PMA_BASE + 16*6))
#define USB7_BDT        ((BDT_TypeDef *) (PMA_BASE + 16*7))

#define	USB_SETUP_DEV2HOST				0x80
#define USB_SETUP_INTERFACE				0x01

#define USB_SETUP_SET_ADDRESS			0x05
#define USB_SETUP_GET_DESCRIPTOR		0x06
#define USB_SETUP_SET_CONFIG			0x09
#define USB_SETUP_SET_HID_IDLE			0x0A
#define USB_SETUP_CLEAR_FEATURE			0x01

#define USB_SETUP_DESC_DEVICE			0x01
#define USB_SETUP_DESC_CONFIG			0x02
#define USB_SETUP_DESC_STRING			0x03
#define USB_SETUP_DESC_HID				0x22


#define USB_CLEAR_FEATURE		0x0102
#define USB_GET_DESCRIPTOR		0x0680
#define USB_SET_ADDRESS			0x0500
#define USB_SET_CONFIGURATION	0x0900
#define USB_GET_MAX_LUN			0xFEA1

/***********************************************************/
/*********   DESCRIPTOR STRUCTURE DEFINITIONS   ************/
/***********************************************************/

#endif

