int YUVGenerateCodebooks4(sb3_encoder_t *handle, sb3_yuvcluster4_t *input, u32 inputCount, u32 goalCells, u32 *resultCount, sb3_yuvcluster4_t **resultElements);
int YUVGenerateCodebooks2(sb3_encoder_t *handle, sb3_yuvcluster2_t *input, u32 inputCount, u32 goalCells, u32 *resultCount, sb3_yuvcluster2_t **resultElements);
int YUVGenerateCodebooks4_NQ(sb3_encoder_t *handle, sb3_yuvcluster4_t *input, u32 inputCount, u32 goalCells, u32 *resultCount, sb3_yuvcluster4_t **resultElements);
int YUVGenerateCodebooks2_NQ(sb3_encoder_t *handle, sb3_yuvcluster2_t *input, u32 inputCount, u32 goalCells, u32 *resultCount, sb3_yuvcluster2_t **resultElements);

int BasicQuant(unsigned char *c, int count, int step, unsigned char *result);
int QuickRemakeYUV(unsigned char *rgb, unsigned char *yuv);
int QuickRemakeRGB(sb3_yuvcluster2_t *cluster, unsigned char *rgb);
