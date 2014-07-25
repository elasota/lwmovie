#define QUICKDIFF(n) (diffs[n]*diffs[n])

rc_inline uint YUVDifference2(sb3_yuvcluster2_t *a, sb3_yuvcluster2_t *b)
{
	uint diffs[6];

	diffs[0] = a->y[0] - b->y[0];
	diffs[1] = a->y[1] - b->y[1];
	diffs[2] = a->y[2] - b->y[2];
	diffs[3] = a->y[3] - b->y[3];
	// UV components are 2x2, so multiplying their diff by 2 now
	// means the square brings it to 4
	diffs[4] = (a->u - b->u)<<1;
	diffs[5] = (a->v - b->v)<<1;

	return QUICKDIFF(0) + QUICKDIFF(1) + QUICKDIFF(2) + QUICKDIFF(3) + QUICKDIFF(4) + QUICKDIFF(5);
}


#define PERTURB_ELEMENT(n)	\
	result = (int)a->n + diff;\
	if(result > 255) result = 255;\
	b->n = (u8)result;\
	result = (int)a->n - diff;\
	if(result < 0) result = 0;\
	a->n = (u8)result




rc_inline void YUVPerturb2(sb3_yuvcluster2_t *a, sb3_yuvcluster2_t *b, uint step)
{
	int diff;
	int result;
	int i;
	int pwr;

	int rmax;
	
	diff = 0;		// Silence compiler warning

	pwr = SB3_PERTURBATION_BASE_POWER - step;
	if(pwr < 1) pwr = 1;
	rmax = (1 << pwr);

	diff = rmax;
	PERTURB_ELEMENT(y[0]);
	PERTURB_ELEMENT(y[1]);
	PERTURB_ELEMENT(y[2]);
	PERTURB_ELEMENT(y[3]);

	diff = rmax / 2;
	PERTURB_ELEMENT(u);
	PERTURB_ELEMENT(v);
}
