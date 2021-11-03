#ifndef DW1000_DEFINITIONS_HEADER
#define DW1000_DEFINITIONS_HEADER


// Maximal mögliche Paketgröße
#define DW1000_MAXPACKAGESIZE 				127
#define DW1000_MAXEXTENDEDPACKAGESIZE 		1023











// Extern zugreifbare Variablen
extern unsigned char dw1000_irqactive;
extern const unsigned char dw1000_chan_idx[];
extern const unsigned int dw1000_digital_bb_config[][4];

// Gerätekennung (zum Abgleich)
#define DW1000_DEVICEID 					0xDECA0130

// Speed of radio waves [m/s] * timestamp resolution [~15.65ps] of DW1000
#define DW1000_LIGHTDISTANCE_PER_TICK 		0.0046917639786159
#define DW1000_LIGHTDISTANCE_PER_TICK_INV 	213.139451293

// Normales Schreiben auf der SPI Schnittstelle
#define DW1000_WRITE 						0x80

// Schreiben auf der SPI Schnittstelle (mit Subadresse)
#define DW1000_WRITE_SUB 					0xc0

// Erweiterte Paketlänge
#define DW1000_EXTENDEDINDEX 				0x80

// Normales Lesen auf der SPI Schnittstelle
#define DW1000_READ 						0x00

// Lesen auf der SPI Schnittstelle (mit Subadresse)
#define DW1000_READ_SUB 					0x40

// enum to determine RX or TX mode of device
#define DW1000_IDLE_MODE 					0x00
#define DW1000_RX_MODE 						0x01
#define DW1000_TX_MODE 						0x02

// Auswahl aller möglichen Taktkonfigurationen
#define DW1000_CLOCK_SYS_RST				0b11111100
#define DW1000_CLOCK_SYS_MASK				0b00000000000000000000000000000111
#define DW1000_CLOCK_SYS_XTI				0b00000000000000000000000000000011
#define DW1000_CLOCK_SYS_AUTO				0b00000000000000000000000000000101
#define DW1000_CLOCK_SYS_PLL				0b00000000000000000000000000000111
#define DW1000_CLOCK_RX_RST					0b11110011
#define DW1000_CLOCK_RX_MASK				0b00000000000000000000000000111000
#define DW1000_CLOCK_RX_XTI					0b00000000000000000000000000001000
#define DW1000_CLOCK_RX_AUTO				0b00000000000000000000000000011000
#define DW1000_CLOCK_RX_PLL					0b00000000000000000000000000101000
#define DW1000_CLOCK_RX_OFF					0b00000000000000000000000000111000
#define DW1000_CLOCK_TX_RST					0b11001111
#define DW1000_CLOCK_TX_MASK				0b00000000000000000000000111000000
#define DW1000_CLOCK_TX_XTI					0b00000000000000000000000001000000
#define DW1000_CLOCK_TX_AUTO				0b00000000000000000000000011000000
#define DW1000_CLOCK_TX_PLL					0b00000000000000000000000101000000
#define DW1000_CLOCK_TX_OFF					0b00000000000000000000000111000000
#define DW1000_CLOCK_ENABLE_OTP				0b00000000000000000000001000000000
#define DW1000_CLOCK_DISABLE_OTP			0b00000000000000000000010000000000
#define DW1000_CLOCK_ENABLE_LDU				0b00000000000000000000100000000000
#define DW1000_CLOCK_DISABLE_LDU			0b00000000000000000001000000000000
#define DW1000_CLOCK_ENABLE_ACCUMULATOR		0b00000000000000000010000000000000
#define DW1000_CLOCK_DISABLE_ACCUMULATOR	0b00000000000000000100000000000000
#define DW1000_CLOCK_ENABLE_ADCCE			0b00000000000000001000000000000000
#define DW1000_CLOCK_DISABLE_ADCCE			0b00000000000000010000000000000000



// PAC size. */
#define DW1000_PAC_SIZE_8 					8
#define DW1000_PAC_SIZE_16 					16
#define DW1000_PAC_SIZE_32 					32
#define DW1000_PAC_SIZE_64 					64





/* frame length settings. */
#define DW1000_FRAME_LENGTH_NORMAL 			0x00
#define DW1000_FRAME_LENGTH_EXTENDED 		0x03


#define DW1000_PEAK_MULTPLIER 				0x60	//3 -> (0x3 * 32) & 0x00E0
#define DW1000_N_STD_FACTOR 				2





/*****************************/
/*                           */
/*  Register des UWB Moduls  */
/*                           */
/*****************************/


// Device Identifier – includes device type and revision info (R)
#define DW1000_DEV_ID 						0x00

// Extended Unique Identifier
#define DW1000_EUI 							0x01

// PAN Identifier and Short Address (RW)
#define DW1000_PANADR 						0x03

// System Configuration bitmap (RW)
#define DW1000_SYS_CFG 						0x04
#define DW1000_SYS_CFG_NOFRAMEFILTER		0b11111111111111111111111000000000
#define DW1000_SYS_CFG_FFEN 				0b00000000000000000000000000000001	/* Frame Filtering Enable */
#define DW1000_SYS_CFG_FFBC 				0b00000000000000000000000000000010  /* Frame Filtering Behave as a Coordinator */
#define DW1000_SYS_CFG_FFAB 				0b00000000000000000000000000000100  /* Frame Filtering Allow Beacon frame reception */
#define DW1000_SYS_CFG_FFAD 				0b00000000000000000000000000001000  /* Frame Filtering Allow Data frame reception */
#define DW1000_SYS_CFG_FFAA 				0b00000000000000000000000000010000  /* Frame Filtering Allow Acknowledgment frame reception */
#define DW1000_SYS_CFG_FFAM 				0b00000000000000000000000000100000  /* Frame Filtering Allow MAC command frame reception */
#define DW1000_SYS_CFG_FFAR 				0b00000000000000000000000001000000  /* Frame Filtering Allow Reserved frame types */
#define DW1000_SYS_CFG_FFA4 				0b00000000000000000000000010000000  /* Frame Filtering Allow frames with frame type field of 4 */
#define DW1000_SYS_CFG_FFA5 				0b00000000000000000000000100000000  /* Frame Filtering Allow frames with frame type field of 5 */
#define DW1000_SYS_CFG_HIRQ_POL 			0b00000000000000000000001000000000  /* Host interrupt polarity */
#define DW1000_SYS_CFG_SPI_EDGE 			0b00000000000000000000010000000000  /* SPI data launch edge */
#define DW1000_SYS_CFG_DIS_FCE 				0b00000000000000000000100000000000  /* Disable frame check error handling */
#define DW1000_SYS_CFG_DIS_DRXB 			0b00000000000000000001000000000000  /* Disable Double RX Buffer */
#define DW1000_SYS_CFG_DIS_PHE 				0b00000000000000000010000000000000  /* Disable receiver abort on PHR error */
#define DW1000_SYS_CFG_DIS_RSDE 			0b00000000000000000100000000000000  /* Disable Receiver Abort on RSD error */
#define DW1000_SYS_CFG_DIS_FCS_INIT2F		0b00000000000000001000000000000000  /* This bit allows selection of the initial seed value for the FCS generation and checking function that is set at the start of each frame transmission and reception */
#define DW1000_SYS_CFG_LONGFRAME     		0b00000000000000110000000000000000
#define DW1000_SYS_CFG_DIS_STXP 			0b00000000000001000000000000000000  /* Disable Smart TX Power control */
#define DW1000_SYS_CFG_RXM110K 				0b00000000010000000000000000000000  /* Receiver Mode 110 kbps data rate */
#define DW1000_SYS_CFG_RXWTOE 				0b00010000000000000000000000000000  /* Receive Wait Timeout Enable */
#define DW1000_SYS_CFG_RXAUTR 				0b00100000000000000000000000000000  /* Receiver Auto-Re-enable */
#define DW1000_SYS_CFG_AUTOACK 				0b01000000000000000000000000000000  /* Automatic Acknowledgement Enable */
#define DW1000_SYS_CFG_AACKPEND 			0b10000000000000000000000000000000  /* Automatic Acknowledgement Pending bit control */

// System Time Counter (40-bit) (R)
#define DW1000_SYS_TIME 					0x06

// Transmit Frame Control (RW)
#define DW1000_TX_FCTRL 0x08
#define DW1000_TX_FCTRL_RESET_TFLEN			0b11111111111111111111110000000000
#define DW1000_TX_FCTRL_RESET_TXBR			0b11111111111111111001111111111111
#define DW1000_TX_FCTRL_RESET_TXPRF			0b11111111111111001111111111111111
#define DW1000_TX_FCTRL_RESET_TXPSR			0b11111111111100111111111111111111
#define DW1000_TX_FCTRL_RESET_TXPE			0b11111111110011111111111111111111
#define DW1000_TX_FCTRL_RESET_TXBOFFS		0b00000000001111111111111111111111
#define DW1000_TX_FCTRL_MASK_TFLEN			0b00000000000000000000001111111111
#define DW1000_TX_FCTRL_MASK_TXBR           0b00000000000000000110000000000000
#define DW1000_TX_FCTRL_SHIFT_TXBR          13
#define DW1000_TX_FCTRL_TR          		0b00000000000000001000000000000000
#define DW1000_TX_FCTRL_MASK_TXPRF          0b00000000000000110000000000000000
#define DW1000_TX_FCTRL_SHIFT_TXPRF         16
#define DW1000_TX_FCTRL_MASK_PLEN         	0b00000000001111000000000000000000
#define DW1000_TX_FCTRL_SHIFT_PLEN         	18
#define DW1000_TX_FCTRL_MASK_PACKAGE_LEN    0b00000000000000000000001111111111

// Transmit Data Buffer (RW)
#define DW1000_TX_BUFFER 					0x09

// Delayed Send or Receive Time (40-bit) (RW)
#define DW1000_DX_TIME 						0x0A

// Receive Frame Wait Timeout Period (RW)
#define DW1000_RX_FWTO 						0x0c

// System Control Register (RW)
#define DW1000_SYS_CTRL 					0x0d
#define DW1000_SYS_CTRL_HRBT_OFFSET			3
#define DW1000_SYS_CTRL_IDLE				0b0000000001000000	/* TRXOFF */
#define DW1000_SYS_CTRL_RX					0b0000000100000000	/* RXENAB */
#define DW1000_SYS_CTRL_TX_DELAYED			0b0000000000000110  /* TXSTRT, TXDLYS und WAIT4RSP */
#define DW1000_SYS_CTRL_TX					0b0000000000000010  /* TXSTRT und WAIT4RSP */
#define DW1000_SYS_CTRL_WAIT4RSP			0b0000000010000000  /* TXSTRT und WAIT4RSP */
#define DW1000_SYS_CTRL_SFCST				0b0000000000000001	/* Supress auto-FCS-transmission */

// System Event Mask Register (RW)
#define DW1000_SYS_MASK 					0x0e
#define DW1000_SYS_MASK_TX					0b00000000000000000000000010000000	/* TXFRS */
#define DW1000_SYS_MASK_RX					0b00000000000000000010000000000000	/* RXDFR; RXFCG; RXPRD */
#define DW1000_SYS_MASK_RX_ERROR			0b00100100001001111001000000000000	/* RXPHE; RXFCE; RXRFSL; RXSFDTO; RXRFTO; RXPTO; AFFREJ; LDEERR */
#define DW1000_SYS_MASK_ALL 				(DW1000_SYS_MASK_TX | DW1000_SYS_MASK_RX | DW1000_SYS_MASK_RX_ERROR)

//  System Event Status Register (RW)
#define DW1000_SYS_STATUS 					0x0f
#define DW1000_SYS_STATUS_IRQS    			0b00000000000000000000000000000001
#define DW1000_SYS_STATUS_CPLOCK   			0b00000000000000000000000000000010
#define DW1000_SYS_STATUS_ESYNCR   			0b00000000000000000000000000000100
#define DW1000_SYS_STATUS_AAT   			0b00000000000000000000000000001000
#define DW1000_SYS_STATUS_TXFRB   			0b00000000000000000000000000010000
#define DW1000_SYS_STATUS_TXPRS   			0b00000000000000000000000000100000
#define DW1000_SYS_STATUS_TXPHS   			0b00000000000000000000000001000000
#define DW1000_SYS_STATUS_TXFRS   			0b00000000000000000000000010000000
#define DW1000_SYS_STATUS_RXPRD				0b00000000000000000000000100000000
#define DW1000_SYS_STATUS_RXSFDD			0b00000000000000000000001000000000
#define DW1000_SYS_STATUS_LDEDONE			0b00000000000000000000010000000000
#define DW1000_SYS_STATUS_RXPHD				0b00000000000000000000100000000000
#define DW1000_SYS_STATUS_RXPHE				0b00000000000000000001000000000000
#define DW1000_SYS_STATUS_RXDFR   			0b00000000000000000010000000000000
#define DW1000_SYS_STATUS_RXFCG   			0b00000000000000000100000000000000
#define DW1000_SYS_STATUS_RXFCE   			0b00000000000000001000000000000000
#define DW1000_SYS_STATUS_RXRFSL   			0b00000000000000010000000000000000
#define DW1000_SYS_STATUS_RXRFTO   			0b00000000000000100000000000000000
#define DW1000_SYS_STATUS_LDEERR   			0b00000000000001000000000000000000
#define DW1000_SYS_STATUS_RXOVRR   			0b00000000000100000000000000000000
#define DW1000_SYS_STATUS_RXPTO   			0b00000000001000000000000000000000
#define DW1000_SYS_STATUS_GPIOIRQ  			0b00000000010000000000000000000000
#define DW1000_SYS_STATUS_SLP2INIT 			0b00000000100000000000000000000000
#define DW1000_SYS_STATUS_RFPLL_LL 			0b00000001000000000000000000000000
#define DW1000_SYS_STATUS_CLKPLL_LL			0b00000010000000000000000000000000
#define DW1000_SYS_STATUS_RXSFDTO			0b00000100000000000000000000000000
#define DW1000_SYS_STATUS_HPDWARN			0b00001000000000000000000000000000
#define DW1000_SYS_STATUS_TXBERR			0b00010000000000000000000000000000
#define DW1000_SYS_STATUS_AFFREJ			0b00100000000000000000000000000000
#define DW1000_SYS_STATUS_HSRBP				0b01000000000000000000000000000000
#define DW1000_SYS_STATUS_ICRBP				0b10000000000000000000000000000000
#define DW1000_SYS_STATUS_TX				0b00000000000000000000000010000000	/* TXFRS */
#define DW1000_SYS_STATUS_TX_ALL 			0b00000000000000000000000011111010	/* AAT; TXFRB; TXPRS; TXPHS; TXFRS */
#define DW1000_SYS_STATUS_RX				0b00000000000000000110111100000000	/* RXDFR; RXFCG; RXPHD; RXSFDD; RXPHD; LDEDONE */
#define DW1000_SYS_STATUS_RX_ALL			0b00100100001001111110111100000000	/* RXDFR; RXFCG; RXPRD; RXSFDD; RXPHD; LDEDONE */
#define DW1000_SYS_STATUS_RX_ERROR			0b00100100001001111001000000000000	/* RXPHE; RXFCE; RXRFSL; RXSFDTO; RXRFTO; RXPTO; AFFREJ; LDEERR */
#define DW1000_SYS_STATUS_RX_BUG611			0b00000000000000000000111000000000  /* LDEDONE; RXPHD; RXSFDD */

// RX Frame Information (in double buffer set)
#define DW1000_RX_FINFO 					0x10

// Receive Data Buffer (in double buffer set)
#define DW1000_RX_BUFFER 					0x11

// Rx Frame Quality information (in double buffer set)
#define DW1000_RX_FQUAL 					0x12
#define DW1000_STD_NOISE_OFFSET 			0x00
#define DW1000_FP_AMPL2_OFFSET 				0x02
#define DW1000_FP_AMPL3_OFFSET 				0x04
#define DW1000_CIR_PWR_OFFSET 				0x06

// Receiver Time Tracking Interval (in double buffer set)
#define DW1000_RX_TTCKI 					0x13

// Receiver Time Tracking Offset (in double buffer set)
#define DW1000_RX_TTCKO 					0x14

// Receive Message Time of Arrival (in double buffer set)
#define DW1000_RX_TIME 						0x15
#define DW1000_RX_STAMP_OFFSET 				0x00
#define DW1000_FP_AMPL1_OFFSET 				0x07

// Transmit Message Time of Sending
#define DW1000_TX_TIME 						0x17
#define DW1000_TX_STAMP_OFFSET 				0

// 16-bit Delay from Transmit to Antenna
#define DW1000_TX_ANTD 						0x18

// System State information
#define DW1000_SYS_STATE 					0x19

// Acknowledgement Time and Response Time
#define DW1000_ACK_RESP_T 					0x1A

// Pulsed Preamble Reception Configuration
#define DW1000_RX_SNIFF 					0x1d
#define DW1000_RX_SNIFF_OFFSET				0x00

// TX Power Control
#define DW1000_TX_POWER 					0x1e

// Channel Control
#define DW1000_CHAN_CTRL 					0x1f
#define DW1000_CHAN_CTRL_REMOVE_CHANNELS	0b11111111111111111111111100000000
#define DW1000_CHAN_CTRL_REMOVE_PRF			0b11111111111100111111111111111111
#define DW1000_CHAN_CTRL_REMOVE_PCODE		0b00000000001111111111111111111111
#define DW1000_CHAN_CTRL_DWSFD				0b00000000000000100000000000000000
#define DW1000_CHAN_CTRL_TNSSFD				0b00000000000100000000000000000000
#define DW1000_CHAN_CTRL_RNSSFD				0b00000000001000000000000000000000

// User-specified short/long TX/RX SFD sequences
#define DW1000_USR_SFD 						0x21
#define DW1000_SFD_LENGTH_OFFSET 			0x00

// Automatic Gain Control configuration
#define DW1000_AGC_CTRL 					0x23

// External synchronisation control.
#define DW1000_EXT_SYNC 					0x24
#define DW1000_EC_CTRL_OFFSET 				0
#define DW1000_EC_CTRL_PLLLCK          		0x04            /* PLL lock detect enable */

// Read access to accumulator data
#define DW1000_ACC_MEM 						0x25

// Peripheral register bus 1 access - GPIO control
#define DW1000_GPIO_CTRL 					0x26

// Digital Receiver configuration
#define DW1000_DRX_CONF 					0x27
#define DW1000_DRX_SFDTOC_OFFSET			0x20

// Analog RF Configuration
#define DW1000_RF_CONF 						0x28
#define DW1000_RF_RXCTRLH_OFFSET 			0x0B
#define DW1000_RF_TXCTRL_OFFSET 			0x0C

// Transmitter calibration block
#define DW1000_TX_CAL 						0x2A
#define DW1000_TC_PGDELAY_OFFSET 			0x0B

// Frequency synthesiser control block
#define DW1000_FS_CTRL 						0x2b
#define DW1000_FS_PLLCFG_OFFSET 			0x07
#define DW1000_FS_PLLTUNE_OFFSET 			0x0B
#define DW1000_FS_XTALT_OFFSET 				0x0E
#define DW1000_FS_XTALT_MASK				0x1F

// Always-On register set
#define DW1000_AON 							0x2c
#define DW1000_AON_WCFG_OFFSET         		0x00    /* used to control what the DW1000 IC does as it wakes up from low-power SLEEP or DEEPSLEEPstates. */
#define DW1000_AON_CFG0_OFFSET         		0x06
#define DW1000_AON_CFG1_OFFSET				0x0a
#define DW1000_AON_CTRL_OFFSET       		0x02
#define DW1000_AON_WCFG 					0
#define DW1000_AON_WCFG_ONW_RADC       		0x0001  /*! On Wake-up Run the (temperature and voltage) Analog-to-Digital Convertors */
#define DW1000_AON_WCFG_ONW_RX         		0x0002  /*! On Wake-up turn on the Receiver */
#define DW1000_AON_WCFG_ONW_LEUI       		0x0008  /*! On Wake-up load the EUI from OTP memory into Register file: 0x01 – Extended Unique Identifier. */
#define DW1000_AON_WCFG_ONW_LDC        		0x0040  /*! On Wake-up load configurations from the AON memory into the host interface register set */
#define DW1000_AON_WCFG_ONW_L64P       		0x0080  /*! On Wake-up load the Length64 receiver operating parameter set */
#define DW1000_AON_WCFG_PRES_SLEEP     		0x0100  /*! Preserve Sleep. This bit determines what the DW1000 does with respect to the ARXSLP and ATXSLP sleep controls */
#define DW1000_AON_WCFG_ONW_LLDE       		0x0800  /*! On Wake-up load the LDE microcode. */
#define DW1000_AON_WCFG_ONW_LLDO       		0x1000  /*! On Wake-up load the LDO tune value. */
#define DW1000_WAKE_CS      				0x4 	/*! wake up on chip select */
#define DW1000_WAKE_WK      				0x2 	/*! wake up on WAKEUP PIN */
#define DW1000_SLP_EN       				0x1 	/*! enable sleep/deep sleep functionality */

// One Time Programmable Memory Interface
#define DW1000_OTP_IF 						0x2d
#define DW1000_OTP_ADDR_OFFSET 				0x04
#define DW1000_OTP_CTRL_OFFSET 				0x06
#define DW1000_OTP_RDAT_OFFSET 				0x0A
#define DW1000_PMSC_CTRL0_PLL2_SEQ_EN		0x01000000UL

// Leading edge detection control block
#define DW1000_LDE_CTRL 					0x2e

// Digital Diagnostics Interface
#define DW1000_DIG_DIAG 					0x2f

// Power Management System Control Block
#define DW1000_PMSC 						0x36
#define DW1000_PMSC_CTRL0_OFFSET 			0x00
#define DW1000_PMSC_CTRL1_OFFSET 			0x04
#define DW1000_PMSC_CTRL0_SOFTRESET_OFFSET 	3
#define DW1000_PMSC_CTRL1_PKTSEQ_DISABLE	0x00        /* writing this to PMSC CONTROL 1 register (bits 10-3) disables PMSC control of analog RF subsystems */
#define DW1000_PMSC_CTRL1_PKTSEQ_ENABLE		0xE7        /* writing this to PMSC CONTROL 1 register (bits 10-3) enables PMSC control of analog RF subsystems */

// AGC_TUNE1/2 (for re-tuning only)
#define DW1000_AGC_TUNE 					0x23
#define DW1000_AGC_TUNE1_OFFSET 			0x04
#define DW1000_AGC_TUNE2_OFFSET 			0x0C
#define DW1000_AGC_TUNE3_OFFSET 			0x12
/*
#define DW1000_AGC_TUNE1_MASK				0xFFFF
#define DW1000_AGC_TUNE1_16M				0x8870
#define DW1000_AGC_TUNE1_64M				0x889B
#define DW1000_AGC_TUNE2_LEN 				4
#define DW1000_AGC_TUNE2_MASK 				0xFFFFFFFFUL
#define DW1000_AGC_TUNE2_VAL 				0X2502A907UL
#define DW1000_AGC_TUNE3_MASK				0xFFFF
#define DW1000_AGC_TUNE3_VAL				0X0055
*/
// DRX_TUNE2 (for re-tuning only)
#define DW1000_DRX_TUNE 					0x27
#define DW1000_DRX_TUNE0b_OFFSET 			0x02
#define DW1000_DRX_TUNE1a_OFFSET 			0x04
#define DW1000_DRX_TUNE1b_OFFSET 			0x06
#define DW1000_DRX_TUNE2_OFFSET 			0x08
#define DW1000_DRX_TUNE4H_OFFSET			0x26

// LDE_CFG1 (for re-tuning only)
#define DW1000_LDE_IF 						0x2E
#define DW1000_LDE_THRESH_OFFSET			0x0000
#define DW1000_LDE_CFG1_OFFSET 				0x0806
#define DW1000_LDE_RXANTD_OFFSET 			0x1804
#define DW1000_LDE_CFG2_OFFSET 				0x1806
#define DW1000_LDE_REPC_OFFSET 				0x2804

#define DW1000_OTP               			0x2D            /* One Time Programmable Memory Interface */
/* offset from OTP_IF_ID in bytes */
#define DW1000_OTP_WDAT						0x00			/* 32-bit register. The data value to be programmed into an OTP location  */
/* offset from OTP_IF_ID in bytes */
#define DW1000_OTP_ADDR						0x04			/* 16-bit register used to select the address within the OTP memory block */
#define DW1000_OTP_ADDR_MASK				0x07FF			/* This 11-bit field specifies the address within OTP memory that will be accessed read or written. */
/* offset from OTP_IF_ID in bytes */
#define DW1000_OTP_CTRL                		0x06            /* used to control the operation of the OTP memory */
#define DW1000_OTP_CTRL_MASK           		0x8002
#define DW1000_OTP_CTRL_OTPRDEN				0x0001			/* This bit forces the OTP into manual read mode */
#define DW1000_OTP_CTRL_OTPREAD        		0x0002          /* This bit commands a read operation from the address specified in the OTP_ADDR register */
#define DW1000_OTP_CTRL_LDELOAD        		0x8000          /* This bit forces a load of LDE microcode */
#define DW1000_OTP_CTRL_OTPPROG        		0x0040          /* Setting this bit will cause the contents of OTP_WDAT to be written to OTP_ADDR. */
/* offset from OTP_IF_ID in bytes */
#define DW1000_OTP_STAT						0x08
#define DW1000_OTP_STAT_MASK				0x0003
#define DW1000_OTP_STAT_OTPPRGD				0x0001			/* OTP Programming Done */
//#define OTP_STAT_OTPVLTOK					0x0002			/* OTP Programming Voltage OK */   !!!!!!!!!!!!
/* offset from OTP_IF_ID in bytes */
#define	DW1000_OTP_RDAT						0x0A			/* 32-bit register. The data value read from an OTP location will appear here */
/* offset from OTP_IF_ID in bytes */
#define	DW1000_OTP_SRDAT					0x0E			/* 32-bit register. The data value stored in the OTP SR (0x400) location will appear here after power up */
/* offset from OTP_IF_ID in bytes */
#define	DW1000_OTP_SF						0x12			/*8-bit special function register used to select and load special receiver operational parameter */
#define DW1000_OTP_SF_MASK             		0x63
#define	DW1000_OTP_SF_OPS_KICK				0x01			/* This bit when set initiates a load of the operating parameter set selected by the OPS_SEL */
#define DW1000_OTP_SF_LDO_KICK         		0x02            /* This bit when set initiates a load of the LDO tune code */
#define	DW1000_OTP_SF_SEL_DEFAULT 			0b01000000		/* Operating parameter set selection: Default */
#define	DW1000_OTP_SF_SEL_L64				0b00000000		/* Operating parameter set selection: Length64 */
#define	DW1000_OTP_SF_SEL_TIGHT				0b00100000		/* Operating parameter set selection: Tight */
#define	DW1000_OTP_SF_SEL_MASK				0b01100000


//OTP addresses definitions
#define DW1000_LDOTUNE_ADDRESS 				0x04
#define DW1000_PARTID_ADDRESS 				0x06
#define DW1000_LOTID_ADDRESS  				0x07
#define DW1000_VBAT_ADDRESS   				0x08
#define DW1000_VTEMP_ADDRESS  				0x09
#define DW1000_TXCFG_ADDRESS  				0x10
#define DW1000_ANTDLY_ADDRESS 				0x1C
#define DW1000_XTRIM_ADDRESS  				0x1E


#endif
