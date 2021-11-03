#include "dw1000_def.h"


const unsigned int dw1000_digital_bb_config[][4] =
{
	{
		0x311A002D,	/* PRF16; PAC 8 */
		0x331A0052,	/* PRF16; PAC 16 */
		0x351A009A,	/* PRF16; PAC 32 */
		0x371A011D	/* PRF16; PAC 64 */
	},
	{
		0x313B006B,	/* PRF64; PAC 8 */
		0x333B00BE,	/* PRF64; PAC 16 */
		0x353B015E,	/* PRF64; PAC 32 */
		0x373B0296	/* PRF64; PAC 64 */
	}
};


// map the channel number to the index in the configuration arrays below
const unsigned char dw1000_chan_idx[] = {0, 0, 1, 2, 3, 4, 0, 5};
