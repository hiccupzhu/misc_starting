/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Forward DCT  -
 *
 ****************************************************************************/

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

/* This routine is a slow-but-accurate integer implementation of the
 * forward DCT (Discrete Cosine Transform). Taken from the IJG software
 *
 * A 2-D DCT can be done by 1-D DCT on each row followed by 1-D DCT
 * on each column.  Direct algorithms are also available, but they are
 * much more complex and seem not to be any faster when reduced to code.
 *
 * This implementation is based on an algorithm described in
 *   C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
 *   Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
 *   Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
 * The primary algorithm described there uses 11 multiplies and 29 adds.
 * We use their alternate method with 12 multiplies and 32 adds.
 * The advantage of this method is that no data path contains more than one
 * multiplication; this allows a very simple and accurate implementation in
 * scaled fixed-point arithmetic, with a minimal number of shifts.
 *
 * The poop on this scaling stuff is as follows:
 *
 * Each 1-D DCT step produces outputs which are a factor of sqrt(N)
 * larger than the true DCT outputs.  The final outputs are therefore
 * a factor of N larger than desired; since N=8 this can be cured by
 * a simple right shift at the end of the algorithm.  The advantage of
 * this arrangement is that we save two multiplications per 1-D DCT,
 * because the y0 and y4 outputs need not be divided by sqrt(N).
 * In the IJG code, this factor of 8 is removed by the quantization step
 * (in jcdctmgr.c), here it is removed.
 *
 * We have to do addition and subtraction of the integer inputs, which
 * is no problem, and multiplication by fractional constants, which is
 * a problem to do in integer arithmetic.  We multiply all the constants
 * by CONST_SCALE and convert them to integer constants (thus retaining
 * CONST_BITS bits of precision in the constants).  After doing a
 * multiplication we have to divide the product by CONST_SCALE, with proper
 * rounding, to produce the correct output.  This division can be done
 * cheaply as a right shift of CONST_BITS bits.  We postpone shifting
 * as long as possible so that partial sums can be added together with
 * full fractional precision.
 *
 * The outputs of the first pass are scaled up by PASS1_BITS bits so that
 * they are represented to better-than-integral precision.  These outputs
 * require 8 + PASS1_BITS + 3 bits; this fits in a 16-bit word
 * with the recommended scaling.  (For 12-bit sample data, the intermediate
 * array is INT32 anyway.)
 *
 * To avoid overflow of the 32-bit intermediate results in pass 2, we must
 * have 8 + CONST_BITS + PASS1_BITS <= 26.  Error analysis
 * shows that the values given below are the most effective.
 *
 * We can gain a little more speed, with a further compromise in accuracy,
 * by omitting the addition in a descaling shift.  This yields an incorrectly
 * rounded result half the time...
 */

#include "fdct.h"

#define USE_ACCURATE_ROUNDING

#define RIGHT_SHIFT(x, shft)  ((x) >> (shft))

#ifdef USE_ACCURATE_ROUNDING
#define ONE ((int) 1)
#define DESCALE(x, n)  RIGHT_SHIFT((x) + (ONE << ((n) - 1)), n)
#else
#define DESCALE(x, n)  RIGHT_SHIFT(x, n)
#endif

#define CONST_BITS  13
#define PASS1_BITS  2

#define FIX_0_298631336  ((int)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((int)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((int)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((int)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((int)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((int)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((int) 12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((int) 15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((int) 16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((int) 16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((int) 20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((int) 25172)	/* FIX(3.072711026) */

/* function pointer */
//fdctFuncPtr fdct;

/*
 * Perform an integer forward DCT on one block of samples.
 */

void
fdct(short *const block)//fdct_int32
{
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z1, z2, z3, z4, z5;
	short *blkptr;
	int *dataptr;
	int data[64];
	int i;

	/* Pass 1: process rows. */
	/* Note results are scaled up by sqrt(8) compared to a true DCT; */
	/* furthermore, we scale the results by 2**PASS1_BITS. */

	dataptr = data;
	blkptr = block;
	for (i = 0; i < 8; i++) {
		tmp0 = blkptr[0] + blkptr[7];
		tmp7 = blkptr[0] - blkptr[7];
		tmp1 = blkptr[1] + blkptr[6];
		tmp6 = blkptr[1] - blkptr[6];
		tmp2 = blkptr[2] + blkptr[5];
		tmp5 = blkptr[2] - blkptr[5];
		tmp3 = blkptr[3] + blkptr[4];
		tmp4 = blkptr[3] - blkptr[4];

		/* Even part per LL&M figure 1 --- note that published figure is faulty;
		 * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
		 */

		tmp10 = tmp0 + tmp3;
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;

		dataptr[0] = (tmp10 + tmp11) << PASS1_BITS;
		dataptr[4] = (tmp10 - tmp11) << PASS1_BITS;

		z1 = (tmp12 + tmp13) * FIX_0_541196100;
		dataptr[2] =
			DESCALE(z1 + tmp13 * FIX_0_765366865, CONST_BITS - PASS1_BITS);
		dataptr[6] =
			DESCALE(z1 + tmp12 * (-FIX_1_847759065), CONST_BITS - PASS1_BITS);

		/* Odd part per figure 8 --- note paper omits factor of sqrt(2).
		 * cK represents cos(K*pi/16).
		 * i0..i3 in the paper are tmp4..tmp7 here.
		 */

		z1 = tmp4 + tmp7;
		z2 = tmp5 + tmp6;
		z3 = tmp4 + tmp6;
		z4 = tmp5 + tmp7;
		z5 = (z3 + z4) * FIX_1_175875602;	/* sqrt(2) * c3 */

		tmp4 *= FIX_0_298631336;	/* sqrt(2) * (-c1+c3+c5-c7) */
		tmp5 *= FIX_2_053119869;	/* sqrt(2) * ( c1+c3-c5+c7) */
		tmp6 *= FIX_3_072711026;	/* sqrt(2) * ( c1+c3+c5-c7) */
		tmp7 *= FIX_1_501321110;	/* sqrt(2) * ( c1+c3-c5-c7) */
		z1 *= -FIX_0_899976223;	/* sqrt(2) * (c7-c3) */
		z2 *= -FIX_2_562915447;	/* sqrt(2) * (-c1-c3) */
		z3 *= -FIX_1_961570560;	/* sqrt(2) * (-c3-c5) */
		z4 *= -FIX_0_390180644;	/* sqrt(2) * (c5-c3) */

		z3 += z5;
		z4 += z5;

		dataptr[7] = DESCALE(tmp4 + z1 + z3, CONST_BITS - PASS1_BITS);
		dataptr[5] = DESCALE(tmp5 + z2 + z4, CONST_BITS - PASS1_BITS);
		dataptr[3] = DESCALE(tmp6 + z2 + z3, CONST_BITS - PASS1_BITS);
		dataptr[1] = DESCALE(tmp7 + z1 + z4, CONST_BITS - PASS1_BITS);

		dataptr += 8;			/* advance pointer to next row */
		blkptr += 8;
	}

	/* Pass 2: process columns.
	 * We remove the PASS1_BITS scaling, but leave the results scaled up
	 * by an overall factor of 8.
	 */

	dataptr = data;
	for (i = 0; i < 8; i++) {
		tmp0 = dataptr[0] + dataptr[56];
		tmp7 = dataptr[0] - dataptr[56];
		tmp1 = dataptr[8] + dataptr[48];
		tmp6 = dataptr[8] - dataptr[48];
		tmp2 = dataptr[16] + dataptr[40];
		tmp5 = dataptr[16] - dataptr[40];
		tmp3 = dataptr[24] + dataptr[32];
		tmp4 = dataptr[24] - dataptr[32];

		/* Even part per LL&M figure 1 --- note that published figure is faulty;
		 * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
		 */

		tmp10 = tmp0 + tmp3;
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;

		dataptr[0] = DESCALE(tmp10 + tmp11, PASS1_BITS);
		dataptr[32] = DESCALE(tmp10 - tmp11, PASS1_BITS);

		z1 = (tmp12 + tmp13) * FIX_0_541196100;
		dataptr[16] =
			DESCALE(z1 + tmp13 * FIX_0_765366865, CONST_BITS + PASS1_BITS);
		dataptr[48] =
			DESCALE(z1 + tmp12 * (-FIX_1_847759065), CONST_BITS + PASS1_BITS);

		/* Odd part per figure 8 --- note paper omits factor of sqrt(2).
		 * cK represents cos(K*pi/16).
		 * i0..i3 in the paper are tmp4..tmp7 here.
		 */

		z1 = tmp4 + tmp7;
		z2 = tmp5 + tmp6;
		z3 = tmp4 + tmp6;
		z4 = tmp5 + tmp7;
		z5 = (z3 + z4) * FIX_1_175875602;	/* sqrt(2) * c3 */

		tmp4 *= FIX_0_298631336;	/* sqrt(2) * (-c1+c3+c5-c7) */
		tmp5 *= FIX_2_053119869;	/* sqrt(2) * ( c1+c3-c5+c7) */
		tmp6 *= FIX_3_072711026;	/* sqrt(2) * ( c1+c3+c5-c7) */
		tmp7 *= FIX_1_501321110;	/* sqrt(2) * ( c1+c3-c5-c7) */
		z1 *= -FIX_0_899976223;	/* sqrt(2) * (c7-c3) */
		z2 *= -FIX_2_562915447;	/* sqrt(2) * (-c1-c3) */
		z3 *= -FIX_1_961570560;	/* sqrt(2) * (-c3-c5) */
		z4 *= -FIX_0_390180644;	/* sqrt(2) * (c5-c3) */

		z3 += z5;
		z4 += z5;

		dataptr[56] = DESCALE(tmp4 + z1 + z3, CONST_BITS + PASS1_BITS);
		dataptr[40] = DESCALE(tmp5 + z2 + z4, CONST_BITS + PASS1_BITS);
		dataptr[24] = DESCALE(tmp6 + z2 + z3, CONST_BITS + PASS1_BITS);
		dataptr[8] = DESCALE(tmp7 + z1 + z4, CONST_BITS + PASS1_BITS);

		dataptr++;				/* advance pointer to next column */
	}
	/* descale */
	for (i = 0; i < 64; i++)
		block[i] = (short int) DESCALE(data[i], 3);
}

void IMG_fdct_8x8_c(short *dct_data, unsigned num_fdcts)
{
    /* -------------------------------------------------------------------- */
    /*  Set up the cosine coefficients.                                     */
    /* -------------------------------------------------------------------- */
    const unsigned short c1 = 0x1F62, c3 = 0x1A9B;      /* Q13   coeffs     */
    const unsigned short c5 = 0x11C7, c7 = 0x063E;      /* Q13   coeffs     */
    const unsigned short c2 = 0x29CF, c6 = 0x1151;      /* Q13.5 coeffs     */
    const unsigned short C1 = 0xFB15, C3 = 0xD4DB;      /* Q16   coeffs     */
    const unsigned short C5 = 0x8E3A, C7 = 0x31F1;      /* Q16   coeffs     */
    const unsigned short C2 = 0xA73D, C6 = 0x4546;      /* Q15.5 coeffs     */
    const unsigned short C4 = 0xB505;                   /* Q16   coeff      */

    /* -------------------------------------------------------------------- */
    /*  Intermediate calculations.                                          */
    /* -------------------------------------------------------------------- */
    short f0, f1, f2, f3, f4, f5, f6, f7;   /* Spatial domain samples.      */
    short g0, g1, h0, h1, p0, p1;           /* Even-half intermediate.      */
    short r0, r1, r0_,r1_;                  /* Even-half intermediate.      */
    short P0, P1, R0, R1;                   /* Even-half intermediate.      */
    short g2, g3, h2, h3;                   /* Odd-half intermediate.       */
    short q1a,s1a,q0, q1, s0, s1;           /* Odd-half intermediate.       */
    short Q0, Q1, S0, S1;                   /* Odd-half intermediate.       */
    short F0, F1, F2, F3, F4, F5, F6, F7;   /* Freq. domain results.        */

    /* -------------------------------------------------------------------- */
    /*  Input and output pointers, loop control.                            */
    /* -------------------------------------------------------------------- */
    unsigned i, j;
    short (*dct)[8][8] = (short (*)[8][8])dct_data;

    if (!num_fdcts) return;

    /* -------------------------------------------------------------------- */
    /*  Outer vertical loop -- Process each 8x8 block.                      */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < num_fdcts; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Perform Vertical 1-D FDCT on columns within each block.         */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < 8; j++)
        {
            /* ------------------------------------------------------------ */
            /*  Load the spatial-domain samples.                            */
            /*  The incoming terms start at Q0 precision.                   */
            /* ------------------------------------------------------------ */
            f0 = dct[i][0][j];
            f1 = dct[i][1][j];
            f2 = dct[i][2][j];
            f3 = dct[i][3][j];
            f4 = dct[i][4][j];
            f5 = dct[i][5][j];
            f6 = dct[i][6][j];
            f7 = dct[i][7][j];

            /* ------------------------------------------------------------ */
            /*  Stage 1:  Separate into even and odd halves.                */
            /*                                                              */
            /*  The results of this stage are implicitly in Q1, since we    */
            /*  do not explicitly multiply by 0.5.                          */
            /* ------------------------------------------------------------ */
            g0 = f0 + f7;               g1 = f1 + f6;   /* Results in Q1    */
            h1 = f2 + f5;               h0 = f3 + f4;   /* Results in Q1    */
            g3 = f2 - f5;               g2 = f3 - f4;   /* Results in Q1    */
            h2 = f0 - f7;               h3 = f1 - f6;   /* Results in Q1    */

            /* ------------------------------------------------------------ */
            /*  Stage 2                                                     */
            /*                                                              */
            /*  Note, on the odd-half, the results are in Q1.5 since those  */
            /*  values are scaled upwards by sqrt(2) at this point.         */
            /* ------------------------------------------------------------ */
            p0 = g0 + h0;               r0 = g0 - h0;   /* Results in Q1    */
            p1 = g1 + h1;               r1 = g1 - h1;   /* Results in Q1    */

            q1a = g2 + g2;                              /* q1a is now Q2    */
            s1a = h2 + h2;                              /* s1a is now Q2    */
            q1  = (q1a * C4 + 0x8000) >> 16;            /* Results in Q1.5  */
            s1  = (s1a * C4 + 0x8000) >> 16;            /* Results in Q1.5  */

            s0 = h3 + g3;                               /* Results in Q1.5  */
            q0 = h3 - g3;                               /* Results in Q1.5  */

            /* ------------------------------------------------------------ */
            /*  Stage 3                                                     */
            /*                                                              */
            /*  Now, the even-half ends up in Q1.5.  On P0 and P1, this     */
            /*  happens because the multiply-by-C4 was canceled with an     */
            /*  upward scaling by sqrt(2).  On R0 and R1, this happens      */
            /*  because C2 and C6 are at Q15.5, and we scale r0 and r1 to   */
            /*  Q2 before we multiply.                                      */
            /* ------------------------------------------------------------ */
            P0 = p0 + p1;                               /* Results in Q1.5  */
            P1 = p0 - p1;                               /* Results in Q1.5  */

            r0_= r0 + r0;                               /* r0_ is now Q2    */
            r1_= r1 + r1;                               /* r1_ is now Q2    */
            R1 = (C6 * r1_+ C2 * r0_+ 0x8000) >> 16;    /* Results in Q1.5  */
            R0 = (C6 * r0_- C2 * r1_+ 0x8000) >> 16;    /* Results in Q1.5  */

            Q1 = q1 + q0;               Q0 = q1 - q0;
            S1 = s1 + s0;               S0 = s1 - s0;

            /* ------------------------------------------------------------ */
            /*  Stage 4                                                     */
            /*  No further changes in Q-point happen here.                  */
            /* ------------------------------------------------------------ */
            F0 = P0;                    F4 = P1;
            F2 = R1;                    F6 = R0;

            F1 = (C7 * Q1 + C1 * S1 + 0x8000) >> 16;    /* Results in Q1.5  */
            F7 = (C7 * S1 - C1 * Q1 + 0x8000) >> 16;    /* Results in Q1.5  */
            F5 = (C3 * Q0 + C5 * S0 + 0x8000) >> 16;    /* Results in Q1.5  */
            F3 = (C3 * S0 - C5 * Q0 + 0x8000) >> 16;    /* Results in Q1.5  */

            /* ------------------------------------------------------------ */
            /*  Store the frequency domain results.                         */
            /*  These values are all at Q1.5 precision.                     */
            /* ------------------------------------------------------------ */
            dct[i][0][j] = F0;
            dct[i][1][j] = F1;
            dct[i][2][j] = F2;
            dct[i][3][j] = F3;
            dct[i][4][j] = F4;
            dct[i][5][j] = F5;
            dct[i][6][j] = F6;
            dct[i][7][j] = F7;
        }
    }

    /* -------------------------------------------------------------------- */
    /*  Perform Horizontal 1-D FDCT on each 8x8 block.                      */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < num_fdcts; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Perform Vertical 1-D FDCT on columns within each block.         */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < 8; j++)
        {
            /* ------------------------------------------------------------ */
            /*  Load the spatial-domain samples.                            */
            /*  The incoming terms are at Q1.5 precision from first pass.   */
            /* ------------------------------------------------------------ */
            f0 = dct[i][j][0];
            f1 = dct[i][j][1];
            f2 = dct[i][j][2];
            f3 = dct[i][j][3];
            f4 = dct[i][j][4];
            f5 = dct[i][j][5];
            f6 = dct[i][j][6];
            f7 = dct[i][j][7];

            /* ------------------------------------------------------------ */
            /*  Stage 1:  Separate into even and odd halves.                */
            /*                                                              */
            /*  The results of this stage are implicitly in Q2.5, since we  */
            /*  do not explicitly multiply by 0.5.                          */
            /* ------------------------------------------------------------ */
            g0 = f0 + f7;               g1 = f1 + f6;   /* Results in Q2.5  */
            h1 = f2 + f5;               h0 = f3 + f4;   /* Results in Q2.5  */
            h2 = f0 - f7;               h3 = f1 - f6;   /* Results in Q2.5  */
            g3 = f2 - f5;               g2 = f3 - f4;   /* Results in Q2.5  */

            /* ------------------------------------------------------------ */
            /*  Stage 2                                                     */
            /*                                                              */
            /*  Note, on the odd-half, the results are in Q3 since those    */
            /*  values are scaled upwards by sqrt(2) at this point.  The    */
            /*  order of operations differs in this pass as compared to     */
            /*  the first due to overflow concerns.                         */
            /*                                                              */
            /*  We also inject a rounding term into the DC term which will  */
            /*  also round the Nyquist term, F4.  This trick works despite  */
            /*  the fact that we are technically still at Q2.5 here, since  */
            /*  the step from Q2.5 to Q3 later is done implicitly, rather   */
            /*  than with a multiply.  (This is due to the sqrt(2) terms    */
            /*  cancelling on the P0/P1 butterfly.)                         */
            /* ------------------------------------------------------------ */
            p0 = g0 + h0 + 4;           p1 = g1 + h1;   /* Results in Q2.5  */
            r0 = g0 - h0;               r1 = g1 - h1;   /* Results in Q2.5  */

            q1a= (g2 * C4 + 0x8000) >> 16;              /* q1a now in Q2    */
            s1a= (h2 * C4 + 0x8000) >> 16;              /* s1a now in Q2    */
            q1 = q1a + q1a;                             /* Results in Q3    */
            s1 = s1a + s1a;                             /* Results in Q3    */

            s0 = h3 + g3;                               /* Results in Q3    */
            q0 = h3 - g3;                               /* Results in Q3    */

            /* ------------------------------------------------------------ */
            /*  Stage 3                                                     */
            /*                                                              */
            /*  Now, the even-half ends up in Q0.  This happens on P0 and   */
            /*  P1 because the multiply-by-c4 was canceled with an upward   */
            /*  scaling by sqrt(2), yielding Q3 intermediate value.  The    */
            /*  final >> 3 leaves these at Q0.  On R0 and R1, this happens  */
            /*  because c2 and c6 are at Q13.5, giving a Q16 intermediate   */
            /*  value.  The final >> 16 then leaves those values at Q0.     */
            /* ------------------------------------------------------------ */
            P0 = ((short)(p0 + p1)) >> 3;               /* Results in Q0    */
            P1 = ((short)(p0 - p1)) >> 3;               /* Results in Q0    */
            R1 = (c6 * r1 + c2 * r0 + 0x8000) >> 16;    /* Results in Q0    */
            R0 = (c6 * r0 - c2 * r1 + 0x8000) >> 16;    /* Results in Q0    */

            Q1 = q1 + q0;               Q0 = q1 - q0;   /* Results in Q3    */
            S1 = s1 + s0;               S0 = s1 - s0;   /* Results in Q3    */

            /* ------------------------------------------------------------ */
            /*  Stage 4                                                     */
            /*                                                              */
            /*  Next, the odd-half ends up in Q0.  This happens because     */
            /*  our values are in Q3 and our cosine terms are in Q13,       */
            /*  giving us Q16 intermediate values. The final >> 16 leaves   */
            /*  us a Q0 result.                                             */
            /* ------------------------------------------------------------ */
            F0 = P0;                    F4 = P1;
            F2 = R1;                    F6 = R0;

            F1 = (c7 * Q1 + c1 * S1 + 0x8000) >> 16;    /* Results in Q0    */
            F7 = (c7 * S1 - c1 * Q1 + 0x8000) >> 16;    /* Results in Q0    */
            F5 = (c3 * Q0 + c5 * S0 + 0x8000) >> 16;    /* Results in Q0    */
            F3 = (c3 * S0 - c5 * Q0 + 0x8000) >> 16;    /* Results in Q0    */

            /* ------------------------------------------------------------ */
            /*  Store the results                                           */
            /* ------------------------------------------------------------ */
            dct[i][j][0] = F0;
            dct[i][j][1] = F1;
            dct[i][j][2] = F2;
            dct[i][j][3] = F3;
            dct[i][j][4] = F4;
            dct[i][j][5] = F5;
            dct[i][j][6] = F6;
            dct[i][j][7] = F7;
        }
    }

    return;
}