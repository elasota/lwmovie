#include <stdlib.h>
#include <string.h>

#include "sb3_internal.h"

int SwitchBlade3_InitializeEncoder(sb3_encoder_t *enc, const sb3_config_t *config)
{
	int fail = 0;

	memcpy(&enc->config, config, sizeof(sb3_config_t));

	if(config->width % 16) return 0;
	if(config->height % 16) return 0;
	if(config->width > 65535) return 0;
	if(config->height > 65535) return 0;
	if(config->width == 0) return 0;
	if(config->height == 0) return 0;

	if(!config->refinementPasses) enc->config.refinementPasses = 1;

	enc->framesSinceKeyframe = 0;
	enc->frameHistory1 = malloc(config->width * config->height * sizeof(sb3_rgbapixel_t));
	enc->frameHistory2 = malloc(config->width * config->height * sizeof(sb3_rgbapixel_t));

	if(!enc->frameHistory1) fail = 1;
	if(!enc->frameHistory2) fail = 1;

	if(fail)
	{
		SwitchBlade3_DestroyEncoder(enc);
		return 0;
	}

	return 1;
}


int SwitchBlade3_DestroyEncoder(sb3_encoder_t *enc)
{
	if(enc->frameHistory1) free(enc->frameHistory1);
	if(enc->frameHistory2) free(enc->frameHistory2);

	return 1;
}


int SwitchBlade3_IFrame(sb3_encoder_t *enc)
{
	enc->framesSinceKeyframe = 0;

	return 1;
}


int SwitchBlade3_BeginFile(sb3_encoder_t *enc, const sb3_hostapi_t *hapi, void *handle)
{
	unsigned char introHeader[8];

	// ROQ intro chunk
	introHeader[0] = 0x84;
	introHeader[1] = 0x10;
	introHeader[2] = 0xff;
	introHeader[3] = 0xff;
	introHeader[4] = 0xff;
	introHeader[5] = 0xff;
	introHeader[6] = 0x1e;
	introHeader[7] = 0x00;

	hapi->writebytes(handle, introHeader, 8);

	return 1;
}

int SwitchBlade3_BeginVideo(sb3_encoder_t *enc, const sb3_hostapi_t *hapi, void *handle)
{
	unsigned char chunk[16];

	// 0x1001 - ROQ info chunk
	chunk[0] = 0x01;
	chunk[1] = 0x10;

	// Size: 8 bytes
	chunk[2] = 0x08;
	chunk[3] = 0x00;
	chunk[4] = 0x00;
	chunk[5] = 0x00;

	// Unused argument
	chunk[6] = 0x00;
	chunk[7] = 0x00;

	// Width
	chunk[8] = enc->config.width;
	chunk[9] = enc->config.width >> 8;

	// Height
	chunk[10] = enc->config.height;
	chunk[11] = enc->config.height >> 8;

	// Who knows?
	chunk[12] = 0x08;
	chunk[13] = 0x00;
	chunk[14] = 0x04;
	chunk[15] = 0x00;

	hapi->writebytes(handle, chunk, 16);

	return 1;
}
