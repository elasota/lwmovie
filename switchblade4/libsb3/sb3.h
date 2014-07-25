#ifndef __SWITCHBLADE_3_H__
#define __SWITCHBLADE_3_H__

typedef struct
{
	unsigned char r,g,b,a;
} sb3_rgbapixel_t;

typedef struct
{
	unsigned int width, height;
	unsigned int refinementPasses;	// default 1
	int useNeuQuant;
} sb3_config_t;

typedef struct
{
	sb3_config_t config;
	unsigned int framesSinceKeyframe;

	unsigned long maxBytes;

	sb3_rgbapixel_t *frameHistory1;
	sb3_rgbapixel_t *frameHistory2;
} sb3_encoder_t;

#define SB3_CODEBOOK_CACHE_SIZE		2560

#define SB3_CODEBOOK_DEBUG_SIZE		16384

typedef struct
{
	void (*writebytes) (void *handle, const void *data, unsigned long numBytes);
	int (*readCodebookCache) (void *handle, void *dest);
	void (*writeCodebookCache) (void *handle, const void *data);
} sb3_hostapi_t;




int SwitchBlade3_InitializeEncoder(sb3_encoder_t *enc, const sb3_config_t *config);

int SwitchBlade3_BeginFile(sb3_encoder_t *enc, const sb3_hostapi_t *hapi, void *handle);
int SwitchBlade3_BeginVideo(sb3_encoder_t *enc, const sb3_hostapi_t *hapi, void *handle);

int SwitchBlade3_SetRefinementPasses(sb3_encoder_t *enc);
int SwitchBlade3_EncodeVideo(sb3_encoder_t *enc, const sb3_rgbapixel_t *rgbData, const sb3_hostapi_t *hapi, void *handle);
int SwitchBlade3_DestroyEncoder(sb3_encoder_t *enc);

int SwitchBlade3_IFrame(sb3_encoder_t *enc);

#endif
