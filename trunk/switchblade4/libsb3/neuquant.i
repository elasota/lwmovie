/* NeuQuant Neural-Net Quantization Algorithm
 * ------------------------------------------
 *
 * Copyright (c) 1994 Anthony Dekker
 *
 * NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 * See "Kohonen neural networks for optimal colour quantization"
 * in "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 * for a discussion of the algorithm.
 * See also  http://members.ozemail.com.au/~dekker/NEUQUANT.HTML
 *
 * Any party obtaining a copy of these files from the author, directly or
 * indirectly, is granted, free of charge, a full and unrestricted irrevocable,
 * world-wide, paid up, royalty-free, nonexclusive right and license to deal
 * in this software and documentation files (the "Software"), including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 */

// This is a modified version of NeuQuant designed to operate on an arbitrary number
// of color elements, in effect, turning it into a codebook generator.  Also made it
// thread-safe.
//
// Modifications (c)2004 Eric Lasota/Orbiter Productions


#include "neuquant.h"


/* Network Definitions
   ------------------- */
   
#define maxnetpos	(netsize-1)
#define netbiasshift	4			/* bias for colour values */
#define ncycles		100			/* no. of learning cycles */

/* defs for freq and bias */
#define intbiasshift    16			/* bias for fractions */
#define intbias		(((int) 1)<<intbiasshift)
#define gammashift  	10			/* gamma = 1024 */
#define gamma   	(((int) 1)<<gammashift)
#define betashift  	10
#define beta		(intbias>>betashift)	/* beta = 1/1024 */
#define betagamma	(intbias<<(gammashift-betashift))

/* defs for decreasing radius factor */
#define initrad		(netsize>>3)		/* for 256 cols, radius starts */
#define radiusbiasshift	6			/* at 32.0 biased by 6 bits */
#define radiusbias	(((int) 1)<<radiusbiasshift)
#define initradius	(initrad*radiusbias)	/* and decreases by a */
#define radiusdec	30			/* factor of 1/30 each cycle */ 

/* defs for decreasing alpha factor */
#define alphabiasshift	10			/* alpha starts at 1.0 */
#define initalpha	(((int) 1)<<alphabiasshift)

/* radbias and alpharadbias used for radpower calculation */
#define radbiasshift	8
#define radbias		(((int) 1)<<radbiasshift)
#define alpharadbshift  (alphabiasshift+radbiasshift)
#define alpharadbias    (((int) 1)<<alpharadbshift)



struct neuquant_instance_s
{
	int alphadec;					/* biased by 10 bits */

	unsigned char *thepicture;		/* the input image itself */
	int lengthcount;				/* lengthcount = H*W*3 */

	int samplefac;				/* sampling factor 1..30 */

	pixel network[netsize];			/* the network itself */

	int netindex[256];			/* for network lookup - really 256 */

	int bias [netsize];			/* bias and freq arrays for learning */
	int freq [netsize];
	int radpower[initrad];			/* radpower for precomputation */
};


/* Initialise network in range (0,0,0) to (255,255,255) and set parameters
   ----------------------------------------------------------------------- */

rc_inline void initnet(neuquant_instance_t *nqi, unsigned char *thepic, int len, int sample)
{
	register int i,j;
	register int *p;
	
	nqi->thepicture = thepic;
	nqi->lengthcount = len;
	nqi->samplefac = sample;

	for (i=0; i<netsize; i++) {
		p = nqi->network[i];
		for(j=0;j<NQ_NUM_COLORS;j++)
			p[j] = (i << (netbiasshift+8))/netsize;
		for(j=0;j<NQ_NUM_COLORS;j+=6)
		{
			p[j+4] *= NQ_CHROMA_BIAS;
			p[j+5] *= NQ_CHROMA_BIAS;
		}
		nqi->freq[i] = intbias/netsize;	/* 1/netsize */
		nqi->bias[i] = 0;
	}
}

	
/* Unbias network to give byte values 0..255 and record position i to prepare for sort
   ----------------------------------------------------------------------------------- */

rc_inline void unbiasnet(neuquant_instance_t *nqi)
{
	int i,j,temp;

	for (i=0; i<netsize; i++) {
		for(j=0;j<NQ_NUM_COLORS;j+=6)
		{
			nqi->network[i][j+4] /= NQ_CHROMA_BIAS;
			nqi->network[i][j+5] /= NQ_CHROMA_BIAS;
		}
		for (j=0; j<NQ_NUM_COLORS; j++) {
			/* OLD CODE: network[i][j] >>= netbiasshift; */
			/* Fix based on bug report by Juergen Weigert jw@suse.de */
			temp = (nqi->network[i][j] + (1 << (netbiasshift - 1))) >> netbiasshift;
			if (temp > 255) temp = 255;
			nqi->network[i][j] = temp;
		}
		nqi->network[i][NQ_NUM_COLORS] = i;			/* record colour no */
	}
}


/* Output colour map
   ----------------- */

rc_inline void dumpcolormap(neuquant_instance_t *nqi, unsigned char *c)
{
	int i,j;

	for(i=0;i<netsize;i++)
	{
		for(j=0;j<NQ_NUM_COLORS;j++)
		{
			*c = nqi->network[i][j];
			c++;
		}
	}
}


/* Search for biased BGR values
   ---------------------------- */

rc_inline int contest(neuquant_instance_t *nqi, int *colors)
{
	/* finds closest neuron (min dist) and updates freq */
	/* finds best neuron (min dist-bias) and returns position */
	/* for frequently chosen neurons, freq[i] is high and bias[i] is negative */
	/* bias[i] = gamma*((1/netsize)-freq[i]) */

	register int i,dist,a,biasdist,betafreq,j;
	int bestpos,bestbiaspos,bestd,bestbiasd;
	register int *p,*f, *n;

	bestd = ~(((int) 1)<<31);
	bestbiasd = bestd;
	bestpos = -1;
	bestbiaspos = bestpos;
	p = nqi->bias;
	f = nqi->freq;

	for (i=0; i<netsize; i++) {
		n = nqi->network[i];
		dist = n[0] - colors[0];   if (dist<0) dist = -dist;

		for(j=1;j<NQ_NUM_COLORS;j++)
		{
			a = n[j] - colors[j];   if (a<0) a = -a;
			dist += a;
		}
		if (dist<bestd) {bestd=dist; bestpos=i;}
		biasdist = dist - ((*p)>>(intbiasshift-netbiasshift));
		if (biasdist<bestbiasd) {bestbiasd=biasdist; bestbiaspos=i;}
		betafreq = (*f >> betashift);
		*f++ -= betafreq;
		*p++ += (betafreq<<gammashift);
	}
	nqi->freq[bestpos] += beta;
	nqi->bias[bestpos] -= betagamma;
	return(bestbiaspos);
}


/* Move neuron i towards biased (b,g,r) by factor alpha
   ---------------------------------------------------- */

rc_inline void altersingle(neuquant_instance_t *nqi, int alpha, int i, int *colors)
{
	register int *n;
	int j;

	n = nqi->network[i];				/* alter hit neuron */

	for(j=0;j<NQ_NUM_COLORS;j++)
	{
		*n -= (alpha*(*n - colors[j])) / initalpha;
		n++;
	}
}


/* Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
   --------------------------------------------------------------------------------- */

rc_inline void alterneigh(neuquant_instance_t *nqi, int rad, int i, int *colors)
{
	register int j,k,lo,hi,a;
	register int *p, *q;
	int c;

	lo = i-rad;   if (lo<-1) lo=-1;
	hi = i+rad;   if (hi>netsize) hi=netsize;

	j = i+1;
	k = i-1;
	q = nqi->radpower;
	while ((j<hi) || (k>lo)) {
		a = (*(++q));
		if (j<hi) {
			p = nqi->network[j];
			for(c=0;c<NQ_NUM_COLORS;c++)
			{
				*p -= (a*(*p - colors[c])) / alpharadbias;
				p++;
			}
			j++;
		}
		if (k>lo) {
			p = nqi->network[k];
			for(c=0;c<NQ_NUM_COLORS;c++)
			{
				*p -= (a*(*p - colors[c])) / alpharadbias;
				p++;
			}
			k--;
		}
	}
}


/* Main Learning Loop
   ------------------ */

rc_inline void learn(neuquant_instance_t *nqi)
{
	register int i,j;
	int radius,rad,alpha,step,delta,samplepixels;
	register unsigned char *p;
	unsigned char *lim;
	int colors[NQ_NUM_COLORS];
	int c;

	nqi->alphadec = 30 + ((nqi->samplefac-1)/3);
	p = nqi->thepicture;
	lim = nqi->thepicture + nqi->lengthcount;
	samplepixels = nqi->lengthcount/(NQ_NUM_COLORS*nqi->samplefac);
	delta = samplepixels/ncycles;
	alpha = initalpha;
	radius = initradius;
	
	rad = radius >> radiusbiasshift;
	if (rad <= 1) rad = 0;
	for (i=0; i<rad; i++) 
		nqi->radpower[i] = alpha*(((rad*rad - i*i)*radbias)/(rad*rad));
	
	//fprintf(stderr,"beginning 1D learning: initial radius=%d\n", rad);

	if ((nqi->lengthcount%prime1) != 0) step = NQ_NUM_COLORS*prime1;
	else {
		if ((nqi->lengthcount%prime2) !=0) step = NQ_NUM_COLORS*prime2;
		else {
			if ((nqi->lengthcount%prime3) !=0) step = NQ_NUM_COLORS*prime3;
			else step = NQ_NUM_COLORS*prime4;
		}
	}
	
	i = 0;
	while (i < samplepixels) {
		for(c=0;c<NQ_NUM_COLORS;c++)
			colors[c] = p[c] << netbiasshift;
		for(c=0;c<NQ_NUM_COLORS;c+=6)
		{
			colors[c+4] *= NQ_CHROMA_BIAS;
			colors[c+5] *= NQ_CHROMA_BIAS;
		}
		j = contest(nqi, colors);

		altersingle(nqi, alpha,j,colors);
		if (rad) alterneigh(nqi, rad,j,colors);   /* alter neighbours */

		p += step;
		if (p >= lim) p -= nqi->lengthcount;
	
		i++;
		if (i%delta == 0) {	
			alpha -= alpha / nqi->alphadec;
			radius -= radius / radiusdec;
			rad = radius >> radiusbiasshift;
			if (rad <= 1) rad = 0;
			for (j=0; j<rad; j++) 
				nqi->radpower[j] = alpha*(((rad*rad - j*j)*radbias)/(rad*rad));
		}
	}
	//fprintf(stderr,"finished 1D learning: final alpha=%f !\n",((float)alpha)/initalpha);
}
