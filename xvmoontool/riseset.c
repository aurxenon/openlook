/*
 * $Id: riseset.c,v 1.1.1.1 2005/07/21 14:22:40 arkenoi Exp $
 */
/*
 * riseset.c
 *
 * Compute various useful times
 *
 * Written by Marc T. Kaufman
 *            14100 Donelson Place
 *            Los Altos Hills, CA 94022
 *            (415) 948-3777
 *
 * Based on : "Explanatory Supplement to the Astronomical Ephemeris
 *             and the American Ephemeris and Nautical Almanac",
 *             H.M. Nautical Almanac Office, London.  Updated from
 *             equations in the 1985 Astronomical Almanac.
 *
 * Copyright 1986 by Marc Kaufman
 *
 * Permission to use this program is granted, provided it is not sold.
 *
 * This program was originally written on a VAX, under 4.2bsd.
 *  it was then ported to a 68000 system under REGULUS (Alcyon's version
 *  of UNIX system III).  Major differences included: no 'double' and
 *  a default integer length of 'short'.  Having been through all that,
 *  porting to your machine should be easy.  Watch out for 'time' related
 *  functions and make sure your 'atan2' program works right.
 *
 *	850210	revised to 1985 Ephemeris - mtk
 *
 * Modified 1/26/89 by Bill Randle, Tektronix, Inc., to interface
 * to the calentool program. Also removed some of the REGULUS stuff.
 * Modifications Copyright (C) 1989, Tektronix, Inc.  All Rights Reserved
 *
 * Permission is hereby granted to use and modify this code in source
 * or binary form as long as it is not sold for profit and this copyright
 * notice remains intact.
 */


#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include "riseset.h"
#include "config.h"

long	UTC, TDT, tim;
double	Julian_Day, MJD, Tu, Ru, T70, GMST, LST;
double	Eqt, Tua, L, G, e, eps, g, alpha, delta, sd, cd, lha, lhr, sh, ch;
double	la, lf, S, C, sp, cp, tp, Az, alt;
double	Lm, lm, px, SD, am, dm;
double	zs, x;
double	fabs(), fmod(), asin(), acos();
struct	tm *ltm, *t, *gmtime(), *localtime();
char	*tdate, *gmctime(), *asctime();
char	riseset_buf[24][64];
char	rbuf[24];

#define Pi			3.1415926535
#define Degree_to_Radian	((2.0 * Pi)/ 360.)
#define Asec_Radian		((2.0 * Pi)/(360. * 60. * 60.))
#define Tsec_to_Radian		((2.0 * Pi)/( 24. * 60.* 60.))
#define Asec_to_Tsec		(24./360.)
#define Sec_per_day		(24 * 60 * 60)
#define Round			0.5		/* for rounding to integer */

#define J1900	/* 24 */15020.0	/* Julian Day number at Epoch 1900.0 */
#define	J1970	/* 24 */40587.5	/* VAX clock Epoch 1970 Jan 1 (0h UT) */
#define	J1985	/* 24 */46065.5	/* Epoch 1985 Jan 1 (0h UT) */
#define	J2000	/* 24 */51545.0	/* Epoch 2000 Jan 1 (12h UT) */
#define Delta_T		(54.6 + 0.9*(Julian_Day - J1985)/365.)	/* TDT - UT */
/* (This is the position of my house ) */


#ifndef LONG_DEGREES
#	define Longitude	0.
#else
#	define Longitude	(((LONG_DEGREES)*60. +  LONG_MINUTES)*60. +  LONG_SECONDS)	/* Arc-seconds West */
#endif
#ifndef LAT_DEGREES
#	define Latitude		37.*360.
#else
#	define Latitude	(((LAT_DEGREES)*60. + LAT_MINUTES)*60. + LAT_SECONDS)	/* Arc-seconds North */
#endif

#define f1			(1. - (1./298.25))	/* 1 - flattening of Earth */
/* the following alternate values are useful when debugging */
#if 0
#define Longitude	(((000.)*60. +  0.)*60. +  0.)	/* Arc-seconds West */
#define Latitude	((( 35.)*60. +  0.)*60. +  0.)	/* Arc-seconds North */
#define f1		1.			/* 1 - flattening of Earth */
#endif

long	Local;

#ifdef __STDC__
void stuff (long int tim);
void moondata (long int tim);
void timedata (long int tim);
#else
void stuff ();
void moondata ();
void timedata ();
#endif

void
riseset(epoch_seconds)

long epoch_seconds;
{
	/*long epoch_seconds;*/

	long rise, set;
	int len_hr, len_min;
	char *moonrise();
	struct tm *ltm, *sunrise();
	
	long	tloc;
	struct tm	*tptp;

/* correct apparent latitude for shape of Earth */

	lf= atan(f1*f1 * tan(Latitude * Asec_Radian));
	sp= sin(lf);
	cp= cos(lf);
	tp= sp/cp;
	
	UTC = epoch_seconds;	/*tloc;*/
#ifdef LONG_DEGREES
	Local = - Longitude/15.;	/* Local apparent time correction */
#else
	Local = (long)0;		/* no correction */
#endif

	/* compute sunrise and sunset */
	t= localtime(&UTC);		/* compute start of day */
	tim = UTC - (3600L * t->tm_hour + 60L * t->tm_min + t->tm_sec)
			+ Sec_per_day/2;	/* about noon */

	/* compute moonrise and moonset */
	tim = tim - Sec_per_day/2 - 31;	/* about start of day */

	zs = 90. + 34./60.;		/* zenith angle of rise/set */
	strcpy(riseset_buf[B_MRD], moonrise(tim, -1.0, zs));
	strcpy(riseset_buf[B_MSD], moonrise(tim, 1.0, zs));
	strcpy(riseset_buf[B_MRT], moonrise((long)(tim+Sec_per_day), -1.0, zs));
	strcpy(riseset_buf[B_MST], moonrise((long)(tim+Sec_per_day), 1.0, zs));
}

char *
moonrise(t0, rs, z)
	long t0;
	double rs, z;
{
#define SRATE	1.033863192	/* ratio of Moon's motion to Sun's motion */
	double	cz, dh, sd, cd;
	long	t1, dt;
	struct tm *ltm;

	moondata(t0);	/* get starting declination of Moon */

	/* compute zenith distance of phenomonon */
	cz = cos(z * Degree_to_Radian + SD /* -px */);

	/* first iteraton is forward only (to approx. phenom time) */
	sd = sin(dm);
	cd = cos(dm);
	dh= -tp*sd/cd + cz/(cp*cd);
	if ((dh < -1.0) || (dh > 1.0))
		return("none");
	dh= acos(dh)*rs;
	dt= fmod((dh - am), 2.0*Pi) * SRATE / Tsec_to_Radian;
	t1 = t0 + dt;

	do {	/* iterate */
		moondata(t1);	/* compute declination and current hour angle */
		cz = cos(z * Degree_to_Radian + SD /* -px */);
		sd = sin(dm);
		cd = cos(dm);

		dh= -tp*sd/cd + cz/(cp*cd);
		if ((dh < -1.0) || (dh > 1.0))
			return("none");
		dh= acos(dh)*rs;
		dt= (dh - am) * SRATE / Tsec_to_Radian;
		t1 += dt;
	} while (dt);

	if ((t1 - t0) >= Sec_per_day)
		return("none");
	t1 += 30 /* seconds, rounding to nearest minute */;
	ltm = localtime(&t1);

	(void)strftime(rbuf, 64, "%H:%M %Z", ltm);

	return(rbuf);
}

void
moondata(tim)
long	tim;
{
	double	lst, beta, rm, sa, ca, sl, cl, sb, cb, x, y, z, l, m, n;

/* compute location of the moon */
/* Ephemeris elements from 1985 Almanac */

	timedata(tim);

	Lm= 218.32 + 481267.883*Tu
		+ 6.29 * sin((134.9 + 477198.85*Tu)*Degree_to_Radian)
		- 1.27 * sin((259.2 - 413335.38*Tu)*Degree_to_Radian)
		+ 0.66 * sin((235.7 + 890534.23*Tu)*Degree_to_Radian)
		+ 0.21 * sin((269.9 + 954397.70*Tu)*Degree_to_Radian)
		- 0.19 * sin((357.5 +  35999.05*Tu)*Degree_to_Radian)
		- 0.11 * sin((186.6 + 966404.05*Tu)*Degree_to_Radian);

	beta=	  5.13 * sin(( 93.3 + 483202.03*Tu)*Degree_to_Radian)
		+ 0.28 * sin((228.2 + 960400.87*Tu)*Degree_to_Radian)
		- 0.28 * sin((318.3 +   6003.18*Tu)*Degree_to_Radian)
		- 0.17 * sin((217.6 - 407332.20*Tu)*Degree_to_Radian);

	px= 0.9508
		+ 0.0518 * cos((134.9 + 477198.85*Tu)*Degree_to_Radian)
		+ 0.0095 * cos((259.2 - 413335.38*Tu)*Degree_to_Radian)
		+ 0.0078 * cos((235.7 + 890534.23*Tu)*Degree_to_Radian)
		+ 0.0028 * cos((269.9 + 954397.70*Tu)*Degree_to_Radian);

/*	SD= 0.2725 * px;	*/

	rm= 1.0 / sin(px * Degree_to_Radian);

	lst= (100.46 + 36000.77*Tu) * Degree_to_Radian
		+ ((tim % Sec_per_day) + Local) * Tsec_to_Radian;

/* form geocentric direction cosines */

	sl= sin(Lm * Degree_to_Radian);
	cl= cos(Lm * Degree_to_Radian);
	sb= sin(beta* Degree_to_Radian);
	cb= cos(beta * Degree_to_Radian);

	l= cb * cl;
	m= 0.9175 * cb * sl - 0.3978 * sb;
	n= 0.3978 * cb * sl + 0.9175 * sb;

/* R.A. and Dec of Moon, geocentric*/

	am= atan2(m, l);
	dm= asin(n);

/* topocentric rectangular coordinates */

	cd= cos(dm);
	sd= n;
	ca= cos(am);
	sa= sin(am);
	sl= sin(lst);
	cl= cos(lst);

	x= rm * cd *ca - cp * cl;
	y= rm * cd * sa - cp * sl;
	z= rm * sd - sp;

/* finally, topocentric Hour-Angle and Dec */

	am = lst - atan2(y, x);
	ca = cos(am);
	sa = sin(am);
	am = atan2(sa,ca);
	rm = sqrt(x*x + y*y + z*z);
	dm = asin(z/rm);
	px = asin(1.0 / rm);
	SD = 0.2725 * px;
}

void
timedata(tim)
long	tim;
{

/* compute seconds from 2000 Jan 1.5 UT (Ephemeris Epoch) */
/* the VAX Epoch is     1970 Jan 1.0 UT (Midnight on Jan 1) */

	Julian_Day = (tim/Sec_per_day) +
				(double)(tim % Sec_per_day)/Sec_per_day + J1970;
	MJD= Julian_Day -J2000;	/* Julian Days past Epoch */
	Tu = MJD/36525.;		/* Julian Centuries past Epoch */

/* compute Sidereal time */

	Ru= 24110.54841 + Tu * (8640184.812866
		+ Tu * (0.09304 - Tu * 6.2e-6));	/* seconds */
	GMST = (tim % Sec_per_day) + Sec_per_day + fmod(Ru, (double)Sec_per_day);
	LST  = GMST + Local;
}


/* double precision modulus, put in range 0 <= result < m */
double fmod(x, m)
double	x, m;
{
	long i;

	i = fabs(x)/m;		/* compute integer part of x/m */
	if (x < 0)	return( x + (i+1)*m);
	else		return( x - i*m);
}
