/* $Header: /home/kmurphy/rcrpanel-3.6/rcrpanel.c,v 3.6 2018/04/27 16:02:08 krm Exp krm $ */
/*
    rcrpanel - draw the front panel for a radio
    Copyright (C) 2001, 2009 John J. McDonough, WB8RCR
    Modified to include rectangles April 2018, Kevin Murphy, W8VOS

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* Draw a panel - platform independent version
 *
 *
 * Application creates PostScript to draw a dial according to specifications
 * in a script file.
 *
 * Usage:
 *
 *    rcrpanel [filename] > [psfile]
 *
 *    where [filename] is the name of a text file containing the description
 *    of the panel to be drawn and [psfile] is the PostScript output.
 *
 *    If the argument is omitted, a default panel is drawn.
 *
 * The input file is a text file containing lines each describing a
 * particular aspect of the panel.  If a particular aspect is not provided,
 * a default value is used.
 *
 * The program is quite picky about the format of the lines.  Each line must
 * begin precisely as shown below, including capitalization and spaces.
 *
 * The following is an example:
 *
 *     Radius = 300.0
 *     Span = 270.0
 *     NumTicks = 25
 *     BigPer = 4
 *     SizeTicks = 19.0
 *     SizeBig = 100.0
 *     ColorCircle = 0x000000
 *     ColorTickMarks = 0x000000
 *     ColorBigTickMarks = 0x00003f
 *     ColorText = 0x000000
 *     SizeFont = 60.0
 *     StartingIndicator = 7.0
 *     IncrementPerBigTick = 0.01
 *
 *  The measurements are in units of millimeters.
 *
 *  Colors are hexadecimal, with 2 nybbles each for red, green and blue
 *  respectively.  For example, a medium purple would be coded 0x808000.
 *
 *  Any unrecognized lines in the file are ignored.  As lines are understood,
 *  the result is displayed on stderr.
 *
 *
 *      $Author: jjmcd $
 *      $Date: 2009/04/08 23:56:08 $
 *      $Author: krm $
 *      $Date: 2018/04/27 16:02:08 $
 *
 *      $Revision: 3.6 $
 *      $State: Exp $
 *
 *      $Log: rcrpanel.c,v $
 *      Revision 3.6  2018/04/27 16:02:08  krm
 *      Add Rectangle objects
 *      Add comment ignore
 *
 *      Revision 3.5  2009/10/19 19:40:08  jjmcd
 *		Use source from the web
 *		Update README to eliminate references to Dial
 *		Include Changelog
 *		Reflect GPLV2+ in specfile

 *      Revision 3.4  2009/04/08 23:56:08  jjmcd
 *      Add GPL statement
 *
 *      Revision 3.3  2009-04-08 19:48:40-04  jjmcd
 *      Rename to avoid name conflicts
 *
 *      Revision 3.2  2003-05-04 16:27:38-04  jjmcd
 *      Still more tweaks in PostScript header
 *
 *      Revision 3.1  2003-05-04 16:22:24-04  jjmcd
 *      A few tweaks in PostScript header
 *
 *      Revision 3.0  2003-05-04 15:07:54-04  jjmcd
 *      Center on page, deal with panels larger than letter size
 *
 *      Revision 2.0  2003-04-26 12:15:26-04  jjmcd
 *      Add ability to rotate dial starting point
 *
 *      Revision 1.9  2001-08-07 19:57:39-04  jjmcd
 *      Add color on text strings
 *
 *      Revision 1.8  2001-08-06 22:25:15-04  jjmcd
 *      This version is likely stable for a day or two
 *
 *      Revision 1.7  2001-08-06 20:55:29-04  jjmcd
 *      Get text working
 *
 *      Revision 1.6  2001-08-06 19:14:12-04  jjmcd
 *      Move dial code to a subroutine, add support for multiple dials, controls.
 *
 *      Revision 1.5  2001-08-05 16:03:32-04  jjmcd
 *      Change scaling to millimeters and add background, controls
 *
 *      Revision 1.4  2001-08-05 09:47:23-04  jjmcd
 *      Dial program commandeered for Panel
 *
 *      Revision 1.4  2001-08-05 08:24:01-04  jjmcd
 *      Fix space problem on display, add signon banner
 *
 *      Revision 1.3  2001-08-05 08:19:30-04  jjmcd
 *      Add Reverse keyword and drilling crosshair
 *
 *      Revision 1.2  2001-08-03 14:06:52-04  jjmcd
 *      Additional comments, error handling
 *
 *      Revision 1.1  2001-08-03 13:51:37-04  jjmcd
 *      Initial version
 *
 */
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAXDIALS 20
#define MAXCONTROLS 50
#define MAXLINES 100
#define MAXSTRINGS 50
#define MAXRECTANGLES 50

#define COLORREF long

int nNumControls;
struct
{
	double x;
	double y;
	double diam;
} rcControl[MAXCONTROLS];

int nNumDials;
struct
{
	double x0;
	double y0;
	double fRadius;
	double fSpan;
	int nNumTicks;
	int nBigPer;
	double fSizeTicks;
	double fSizeBig;
	COLORREF crCircle;
	COLORREF crTickMarks;
	COLORREF crBigTickMarks;
	COLORREF crText;
	double fSizeFont;
	double fStartingIndicator;
	double fIncrementPerBigTick;
	double fStartAngle;
} rcDial[MAXDIALS];

int nNumRectangles;
struct
{
	double x0;
	double y0;
	double fRectangleHeight;
	double fRectangleWidth;
} rcRectangle[MAXRECTANGLES];

int nNumCtrRectangles;
struct
{
	double x;
	double y;
	double fRectangleHeight;
	double fRectangleWidth;
} rcRectCenter[MAXRECTANGLES];

int nNumStrings;
struct
{
	double x;
	double y;
	double size;
	COLORREF crColor;
	char szFont[32];
	char szString[128];
} rcText[MAXSTRINGS];

double fPanelHeight;
double fPanelWidth;
double fPrintableWidth;
double fPrintableHeight;
char szPagesize[32];
char szFilename[256];
COLORREF crBackground;
int nReverse;

char szBuffer[2048];

/* Bump up the control number, error if too big */
void incrementControlCount( void )
{
	nNumControls++;
	if ( nNumControls >= MAXCONTROLS )
	{
		fprintf(stderr, "*** Error, maximum of %d controls exceeded. ***\n", MAXCONTROLS);
		nNumControls = MAXCONTROLS - 1;
	}
}

/* Bump up the rectangle number, error if too big */

void incrementRectangleCount( void )
{
	nNumRectangles++;
	if ( (nNumRectangles + nNumCtrRectangles) >= MAXRECTANGLES )
	{
		fprintf(stderr, "*** Error, maximum of %d rectangles exceeded. ***\n", MAXRECTANGLES);
		nNumRectangles = MAXRECTANGLES - 1;
	}
}

/* Read the input file */
void getFile( FILE *f )
{
	int nProcessedOK;

	while ( !feof(f) )
	{
		fgets(szBuffer, sizeof(szBuffer), f);
		if ( !feof(f) )
		{
			if ( szBuffer[strlen(szBuffer) - 1] < '!' )
				szBuffer[strlen(szBuffer) - 1] = '\0';
			if ( szBuffer[strlen(szBuffer) - 1] < '!' )
				szBuffer[strlen(szBuffer) - 1] = '\0';
			nProcessedOK = 0;
			if ( nNumDials > 0 )
			{
				nProcessedOK = 1;
				if ( !strncmp("Radius", szBuffer, 6) )
				{
					sscanf(&szBuffer[8], "%lf", &rcDial[nNumDials - 1].fRadius);
					fprintf(stderr, "                     Radius: %lf\n", rcDial[nNumDials - 1].fRadius);
				}
				else if ( !strncmp("Span", szBuffer, 4) )
				{
					sscanf(&szBuffer[6], "%lf", &rcDial[nNumDials - 1].fSpan);
					fprintf(stderr, "                       Span: %lf\n", rcDial[nNumDials - 1].fSpan);
				}
				else if ( !strncmp("NumTicks", szBuffer, 8) )
				{
					sscanf(&szBuffer[10], "%d", &rcDial[nNumDials - 1].nNumTicks);
					fprintf(stderr, "       Number of Tick marks: %d\n", rcDial[nNumDials - 1].nNumTicks);
				}
				else if ( !strncmp("BigPer", szBuffer, 6) )
				{
					sscanf(&szBuffer[8], "%d", &rcDial[nNumDials - 1].nBigPer);
					fprintf(stderr, "        Small ticks per big: %d\n", rcDial[nNumDials - 1].nBigPer);
				}
				else if ( !strncmp("SizeTicks", szBuffer, 9) )
				{
					sscanf(&szBuffer[11], "%lf", &rcDial[nNumDials - 1].fSizeTicks);
					fprintf(stderr, "   Size of small tick marks: %lf\n", rcDial[nNumDials - 1].fSizeTicks);
				}
				else if ( !strncmp("SizeBig", szBuffer, 7) )
				{
					sscanf(&szBuffer[9], "%lf", &rcDial[nNumDials - 1].fSizeBig);
					fprintf(stderr, "     Size of big tick marks: %lf\n", rcDial[nNumDials - 1].fSizeBig);
				}
				else if ( !strncmp("StartingIndicator", szBuffer, 17) )
				{
					sscanf(&szBuffer[19], "%lf", &rcDial[nNumDials - 1].fStartingIndicator);
					fprintf(stderr, "         Starting indicator: %lf\n", rcDial[nNumDials - 1].fStartingIndicator);
				}
				else if ( !strncmp("IncrementPerBigTick", szBuffer, 19) )
				{
					sscanf(&szBuffer[21], "%lf", &rcDial[nNumDials - 1].fIncrementPerBigTick);
					fprintf(stderr, "Increment per big tick mark: %lf\n", rcDial[nNumDials - 1].fIncrementPerBigTick);
				}
				else if ( !strncmp("SizeFont", szBuffer, 8) )
				{
					sscanf(&szBuffer[11], "%lf", &rcDial[nNumDials - 1].fSizeFont);
					fprintf(stderr, "            Size of numbers: %lf\n", rcDial[nNumDials - 1].fSizeFont);
				}
				else if ( !strncmp("ColorCircle", szBuffer, 11) )
				{
					sscanf(&szBuffer[13], "%lx", &rcDial[nNumDials - 1].crCircle);
					fprintf(stderr, "            Color of circle: 0x%06lx\n", rcDial[nNumDials - 1].crCircle);
				}
				else if ( !strncmp("ColorTickMarks", szBuffer, 14) )
				{
					sscanf(&szBuffer[16], "%lx", &rcDial[nNumDials - 1].crTickMarks);
					fprintf(stderr, "  Color of small tick marks: 0x%06lx\n", rcDial[nNumDials - 1].crTickMarks);
				}
				else if ( !strncmp("ColorBigTickMarks", szBuffer, 17) )
				{
					sscanf(&szBuffer[19], "%lx", &rcDial[nNumDials - 1].crBigTickMarks);
					fprintf(stderr, "    Color of big tick marks: 0x%06lx\n", rcDial[nNumDials - 1].crBigTickMarks);
				}
				else if ( !strncmp("ColorText", szBuffer, 9) )
				{
					sscanf(&szBuffer[11], "%lx", &rcDial[nNumDials - 1].crText);
					fprintf(stderr, "           Color of numbers: 0x%06lx\n", rcDial[nNumDials - 1].crText);
				}
				else if ( !strncmp("StartAngle", szBuffer, 4) )
				{
					sscanf(&szBuffer[12], "%lf", &rcDial[nNumDials - 1].fStartAngle);
					fprintf(stderr, "                Start Angle: %lf\n", rcDial[nNumDials - 1].fStartAngle);
				}
				else
					nProcessedOK = 0;
			}
			if ( !strncmp("Reverse", szBuffer, 7) )	/* Print in reverse? */
			{
				nReverse = 1;
				fprintf(stderr, "                    Reverse: TRUE\n");
			}
			else if ( !strncmp("/*", szBuffer, 2) || !strncmp("/**", szBuffer, 3) ) /* Comments */
			{
				// fprintf (stderr, "*** here a comment ***\r\n");
			}
			else if ( !strncmp("Dial", szBuffer, 4) ) /* Dials */
			{
				sscanf(&szBuffer[6], "%lf %lf", &rcDial[nNumDials].x0, &rcDial[nNumDials].y0);
				fprintf(stderr, "===New dial (%d) at %f,%f\r\n", nNumDials + 1, rcDial[nNumDials].x0, rcDial[nNumDials].y0);
				nNumDials++;
			}
			else if ( !strncmp("ControlLarge", szBuffer, 12) ) /* Large pots etc. */
			{
				sscanf(&szBuffer[14], "%lf %lf", &rcControl[nNumControls].x, &rcControl[nNumControls].y);
				rcControl[nNumControls].diam = 9.35;
				incrementControlCount();
			}
			else if ( !strncmp("ControlPhone", szBuffer, 12) ) /* 1/4" phone jack */
			{
				sscanf(&szBuffer[14], "%lf %lf", &rcControl[nNumControls].x, &rcControl[nNumControls].y);
				rcControl[nNumControls].diam = 8.8;
				incrementControlCount();
			}
			else if ( !strncmp("ControlLED", szBuffer, 10) ) /* 5mm LED holder */
			{
				sscanf(&szBuffer[12], "%lf %lf", &rcControl[nNumControls].x, &rcControl[nNumControls].y);
				rcControl[nNumControls].diam = 6.0;
				incrementControlCount();
			}
			else if ( !strncmp("ControlSmall", szBuffer, 12) ) /* 3.5 mm phone jack */
			{
				sscanf(&szBuffer[14], "%lf %lf", &rcControl[nNumControls].x, &rcControl[nNumControls].y);
				rcControl[nNumControls].diam = 5.75;
				incrementControlCount();
			}
			else if ( !strncmp("ControlTiny", szBuffer, 11) ) /* 2.5 mm phone jack */
			{
				sscanf(&szBuffer[13], "%lf %lf", &rcControl[nNumControls].x, &rcControl[nNumControls].y);
				rcControl[nNumControls].diam = 3.8;
				incrementControlCount();
			}
			else if ( !strncmp("ControlMicro", szBuffer, 12) ) /* 3 mm LED hole */
			{
				sscanf(&szBuffer[14], "%lf %lf", &rcControl[nNumControls].x, &rcControl[nNumControls].y);
				rcControl[nNumControls].diam = 2.6;
				incrementControlCount();
			}
			else if (!strncmp("Rectangle", szBuffer, 9) )	/* Rectangle */
			{
				sscanf(&szBuffer[11], "%lf %lf %lf %lf", &rcRectangle[nNumRectangles].x0, &rcRectangle[nNumRectangles].y0, &rcRectangle[nNumRectangles].fRectangleWidth, &rcRectangle[nNumRectangles].fRectangleHeight);
				fprintf(stderr, "====New rectangle (%d) at %f,%f with size of %f by %f\r\n", nNumRectangles + 1, rcRectangle[nNumRectangles].x0, rcRectangle[nNumRectangles].y0, rcRectangle[nNumRectangles].fRectangleHeight, rcRectangle[nNumRectangles].fRectangleWidth);
				incrementRectangleCount();
			}
			else if (!strncmp("CenterRectangle", szBuffer, 15) )	/* Rectangle given center */
			{
				sscanf(&szBuffer[17], "%lf %lf %lf %lf", &rcRectCenter[nNumCtrRectangles].x, &rcRectCenter[nNumCtrRectangles].y, &rcRectCenter[nNumCtrRectangles].fRectangleWidth, &rcRectCenter[nNumCtrRectangles].fRectangleHeight);
				fprintf(stderr, "-+-New Centered rectangle (%d) at %f,%f with size of %f by %f\r\n", nNumCtrRectangles + 1, rcRectCenter[nNumCtrRectangles].x, rcRectCenter[nNumCtrRectangles].y, rcRectCenter[nNumCtrRectangles].fRectangleHeight, rcRectCenter[nNumCtrRectangles].fRectangleWidth);
				nNumCtrRectangles++;
			}
			else if ( !strncmp("Panel", szBuffer, 5) )     /* Panel dimensions */
			{
				sscanf(&szBuffer[7], "%lf %lf", &fPanelWidth, &fPanelHeight);
				fprintf(stderr, "                 Panel size: %f by %f mm.\n", fPanelWidth, fPanelHeight);
			}
			else if ( !strncmp("Background", szBuffer, 10) )     /* Panel dimensions */
			{
				sscanf(&szBuffer[12], "%lx", &crBackground);
				fprintf(stderr, "     Panel background color: 0x%06lx\n", crBackground);
			}
			else if ( !strncmp("Text", szBuffer, 4) )
			{
				sscanf(&szBuffer[6], "%lf %lf %lf %lx %s", &rcText[nNumStrings].x, &rcText[nNumStrings].y,
				       &rcText[nNumStrings].size, &rcText[nNumStrings].crColor, rcText[nNumStrings].szFont);
				fgets(rcText[nNumStrings].szString, 128, f);
				if ( rcText[nNumStrings].szString[strlen(rcText[nNumStrings].szString) - 1] < '!' )
					rcText[nNumStrings].szString[strlen(rcText[nNumStrings].szString) - 1] = '\0';
				if ( rcText[nNumStrings].szString[strlen(rcText[nNumStrings].szString) - 1] < '!' )
					rcText[nNumStrings].szString[strlen(rcText[nNumStrings].szString) - 1] = '\0';
				fprintf(stderr, "                      Text : at %f %f size %f face %s\n",
				        rcText[nNumStrings].x, rcText[nNumStrings].y, rcText[nNumStrings].size,
				        rcText[nNumStrings].szFont);
				fprintf(stderr, "                           : [%s]\n", rcText[nNumStrings].szString);
				nNumStrings++;
			}
			else if ( (strlen(szBuffer) > 1) && !nProcessedOK )
			{
				if ( szBuffer[strlen(szBuffer) - 1] < '!' )
					szBuffer[strlen(szBuffer) - 1] = '\0';
				fprintf(stderr, "***[%s] NOT UNDERSTOOD***\n", szBuffer);
			}
		}
	}
}

/* Set default parameters in case no file or missing from file */
void initParams( void )
{
	int i;

	nNumDials = 0;
	for ( i = 0; i < MAXDIALS; i++ )
	{
		rcDial[i].fRadius = 10.0;
		rcDial[i].fSpan = 270.0;
		rcDial[i].nNumTicks = 81;
		rcDial[i].nBigPer = 10;
		rcDial[i].fSizeTicks = 1.0;
		rcDial[i].fSizeBig = 4.0;
		rcDial[i].crCircle = 0x000000;
		rcDial[i].crTickMarks = 0x000000;
		rcDial[i].crBigTickMarks = 0x000000;
		rcDial[i].crText = 0x000000;
		rcDial[i].fSizeFont = 1.5;
		rcDial[i].fStartingIndicator = 14.0;
		rcDial[i].fIncrementPerBigTick = 0.01;
		rcDial[i].fStartAngle = 0.0;
	}
	fPanelHeight = 53.975;
	fPanelWidth = 193.675;
	nReverse = 0;
	nNumControls = 0;
	memset(rcControl, 0, sizeof(rcControl));
	crBackground = 0xdfefff;
	nNumStrings = 0;
	memset(rcText, 0, sizeof(rcText));
}

/* Dump the PostScript from the buffer */
void purgeBuffer( void )
{
	if ( szBuffer[strlen(szBuffer) - 1] == ' ' )
		szBuffer[strlen(szBuffer) - 1] = '\0';
	printf("%s\r\n", szBuffer);
	memset(szBuffer, 0, sizeof(szBuffer));
}

/* Add a string to the buffer */
void addBuffer( char *p )
{
	if ( strlen(szBuffer) > 72 )
		purgeBuffer();
	strcat(szBuffer, p);
}

/* Add a setrgbcolor command to the buffer */
void addColor( long rgb )
{
	double x;
	char szWork[32];

	x = ( (double) ( (rgb & 0xff0000) >> 16 ) ) / 256.0;
	sprintf(szWork, "%4.2f ", x);
	addBuffer(szWork);
	x = ( (double) ( (rgb & 0xff00) >> 8 ) ) / 256.0;
	sprintf(szWork, "%4.2f ", x);
	addBuffer(szWork);
	x = ( (double) ( (rgb & 0xff) ) ) / 256.0;
	sprintf(szWork, "%4.2f ", x);
	addBuffer(szWork);
	addBuffer("C ");
}

/* Add a lineto command to the buffer */
void addLine( double x, double y )
{
	char szWork[32];
	sprintf(szWork, "%f ", x);
	addBuffer(szWork);
	sprintf(szWork, "%f ", y);
	addBuffer(szWork);
	addBuffer("L ");
}

/* Add a moveto command to the buffer */
void addMove( double x, double y )
{
	char szWork[32];
	sprintf(szWork, "%f ", x);
	addBuffer(szWork);
	sprintf(szWork, "%f ", y);
	addBuffer(szWork);
	addBuffer("M ");
}

/* Calculate the format for display of values */
void setFormat( int nDialNum, char *szFormat )
{
	int left, right;

	strcpy(szFormat, "%lf");
	if ( rcDial[nDialNum].fStartingIndicator < 9 )
		left = 1;
	else if ( rcDial[nDialNum].fStartingIndicator < 90 )
		left = 2;
	else if ( rcDial[nDialNum].fStartingIndicator < 900 )
		left = 3;
	else if ( rcDial[nDialNum].fStartingIndicator < 9000 )
		left = 4;
	else
		left = 5;
	if ( rcDial[nDialNum].fIncrementPerBigTick > 0.9 )
		right = 0;
	else if ( rcDial[nDialNum].fIncrementPerBigTick > 0.09 )
		right = 1;
	else if ( rcDial[nDialNum].fIncrementPerBigTick > 0.009 )
		right = 2;
	else if ( rcDial[nDialNum].fIncrementPerBigTick > 0.0009 )
		right = 3;
	else if ( rcDial[nDialNum].fIncrementPerBigTick > 0.00009 )
		right = 4;
	else if ( rcDial[nDialNum].fIncrementPerBigTick > 0.000009 )
		right = 5;
	else if ( rcDial[nDialNum].fIncrementPerBigTick > 0.0000009 )
		right = 6;
	else
		right = 7;
	sprintf(szFormat, "%%%d.%dlf", left, right);
}

/* Add the PostScript header to the output stream */
void addHeader( void )
{
	printf("%%!PS-Adobe-2.0\r\n");
	printf("%%%%Title: (Panel from file %s)\r\n", szFilename);
	printf("%%%%Creator: Panel $Revision: 3.4 $\r\n");
	printf("%%%%Author: (Panel $Revision: 3.4 $)\r\n");
	printf("%%%%Pages: 1\r\n");
	printf("%%%%Orientation: Landscape\r\n");
	printf("%%%%DocumentPaperSizes: %s\r\n", szPagesize);
	printf("%%%%EndComments\r\n");
	printf("%%%%BeginProlog\r\n");
	printf("%%%%BeginProcSet: TextProcs 1.0 0\r\n");
	printf("/F { findfont exch scalefont setfont } bind def /L { lineto } bind def /C\r\n");
	printf("{ setrgbcolor } bind def /M { moveto } bind def /S { stroke } bind def /W\r\n");
	printf("{ setlinewidth } bind def /T { show } bind def /TR { dup stringwidth exch\r\n");
	printf("neg exch rmoveto show } bind def /TC { dup stringwidth exch 2 div neg\r\n");
	printf("exch rmoveto show } bind def\r\n");
	printf("%%%%EndProcSet\r\n");
	printf("%%%%EndProlog\r\n");
	printf("%%%%Page: 1 1\r\n");
}

void addControl( double x0, double y0, double diam )
{
	double theta, r, x, y;
	int i;

	r = diam / 2.0;
	addBuffer("0.1 W ");

	for ( i = 0; i < 50; i++ )
	{
		theta = 6.28 * (double) i / 49.0;
		x = x0 + r * sin( theta);
		y = y0 + r * cos( theta );
		if ( i )
			addLine(x, y);
		else
			addMove(x, y);
	}
	/* Fill the circle with white */
	addColor(0xffffff);
	addBuffer("closepath fill ");

	/* Outline it in black */
	for ( i = 0; i < 100; i++ )
	{
		theta = 6.28 * (double) i / 99.0;
		x = x0 + r * sin( theta);
		y = y0 + r * cos( theta );
		if ( i )
			addLine(x, y);
		else
			addMove(x, y);
	}
	addColor(0);
	addBuffer("S ");

	/* Add the crosshair */
	addMove(x0 - r, y0);
	addLine(x0 + r, y0);
	addBuffer("S ");
	addMove(x0, y0 - r);
	addLine(x0, y0 + r);
	addBuffer("S ");
}

void addString( int n )
{
	char szWork[128];
	char *p;

	/* Font size */
	sprintf(szWork, "%6.2f ", rcText[n].size);
	p = szWork;
	while ( *p == ' ')
		p++;
	addBuffer(p);
	/* Font name */
	sprintf(szWork, "/%s ", rcText[n].szFont);
	addBuffer(szWork);
	addBuffer("F ");

	addColor(rcText[n].crColor);

	sprintf(szWork, "%7.2f ", rcText[n].x);
	p = szWork;
	while ( *p == ' ')
		p++;
	addBuffer(p);

	sprintf(szWork, "%7.2f ", rcText[n].y);
	p = szWork;
	while ( *p == ' ')
		p++;
	addBuffer(p);
	addBuffer("M ");

	addBuffer("( ");
	addBuffer(rcText[n].szString);
	addBuffer(" ) TC ");
}

void addDial( int nDialNum )
{
	int i;
	char szWork[132];
	double x, y, x0, y0;
	double theta, theta0;
	double fCurrentIndication;
	int nBig;
	char szIndicate[128], szFormat[32];
	char *p;
	double fTop, fDelta;

	/* Draw circle */

	fTop = 3.0 * 3.14159268 / 2.0;     /* Angle at top of dial */
	fDelta = 3.14159268 / 18.0;        /* Range of angles to center number */

	/* Make the circle bolder for larger dials */
	if ( rcDial[nDialNum].fRadius > 400.0 )
		addBuffer("gsave 0.677 W ");
	else
		addBuffer("gsave 0.339 W ");

	addColor(rcDial[nDialNum].crCircle);

	x0 = rcDial[nDialNum].x0;
	y0 = rcDial[nDialNum].y0;

	theta0 = 90.0 + (360.0 - rcDial[nDialNum].fSpan) / 2.0 + rcDial[nDialNum].fStartAngle; /* Starting angle */

	for ( i = 0; i < 1000; i++ )       /* Will draw circle in 1000 segments */
	{
		theta = 3.14159268 * (theta0 + ((double) i) * rcDial[nDialNum].fSpan / 999.0) / 180.0;
		x = x0 + rcDial[nDialNum].fRadius * cos( theta );
		y = y0 - rcDial[nDialNum].fRadius * sin( theta );
		if ( i )
			addLine(x, y);
		else
			addMove(x, y);
	}
	addBuffer("S grestore ");

	/* Draw ticks and annotation */

	if ( rcDial[nDialNum].fSizeTicks > 20.0 )           /* Use a heavier line if ticks are long */
		addBuffer("gsave 0.339 W ");
	else
		addBuffer("gsave 0.169 W ");
	addColor(rcDial[nDialNum].crTickMarks);             /* Color of small tick marks */
	setFormat( nDialNum, szFormat );             /* Calculate format for numbers */

	/* Set up the font of the right size */
	sprintf(szWork, "%6.2f ", rcDial[nDialNum].fSizeFont);
	p = szWork;
	while ( *p == ' ')
		p++;
	addBuffer(p);
	addBuffer("/Helvetica-Bold ");
	addBuffer("F ");

	/* Loop through all the ticks */
	nBig = 0;
	for ( i = 0; i < rcDial[nDialNum].nNumTicks; i++ )
	{
		double nn = rcDial[nDialNum].nNumTicks - 1;
		theta = 3.14159268 * (theta0 + ((double) i) * rcDial[nDialNum].fSpan / nn) / 180.0;
		x = x0 + rcDial[nDialNum].fRadius * cos( theta );
		y = y0 - rcDial[nDialNum].fRadius * sin( theta );
		addMove(x, y);

		if ( i % rcDial[nDialNum].nBigPer )             /* Is it a big one? */
		{	/* No */
			x = x0 + (rcDial[nDialNum].fRadius + rcDial[nDialNum].fSizeTicks) * cos( theta );
			y = y0 - (rcDial[nDialNum].fRadius + rcDial[nDialNum].fSizeTicks) * sin( theta );
			addLine(x, y);
			addBuffer("S ");
		}
		else
		{	/* Yes */
			addColor(rcDial[nDialNum].crBigTickMarks);
			x = x0 + (rcDial[nDialNum].fRadius + rcDial[nDialNum].fSizeBig) * cos( theta );
			y = y0 - (rcDial[nDialNum].fRadius + rcDial[nDialNum].fSizeBig) * sin( theta );
			addLine(x, y);
			addBuffer("S ");

			/* Now need to annotate the big tick mark */
			fCurrentIndication = rcDial[nDialNum].fStartingIndicator
			                     + nBig * rcDial[nDialNum].fIncrementPerBigTick;
			nBig++;
			sprintf(szIndicate, szFormat, fCurrentIndication);

			addColor(rcDial[nDialNum].crText);          /* Set text color */
			addMove(x, y);
			sprintf(szWork, "(%s) ", szIndicate);
			addBuffer(szWork);
			/* If the tick is near the top, center text over tick */
			if ( fabs(theta - fTop) < fDelta )
				addBuffer("TC ");
			/* otherwise if the tick is on the left, start left of the tick */
			else if ( theta < fTop )
				addBuffer("TR ");
			/* Otherwise start at the tick */
			else
				addBuffer("T ");
			/* Reset to small tick color */
			addColor(rcDial[nDialNum].crTickMarks);
		}
	}

	/* Finally, place a drilling crosshair */
	addColor(0);
	addMove(x0 - 4, y0);
	addLine(x0 + 4, y0);
	addBuffer("S ");
	addMove(x0, y0 - 4);
	addLine(x0, y0 + 4);
	addBuffer("S ");

	addBuffer("grestore ");
}

void addRectangle( double x0, double y0, double width, double height )
{
	addBuffer("0.1 W ");

	addMove(x0, y0);
	addLine(x0 + width, y0);
	addLine(x0 + width, y0 + height);
	addLine(x0, y0 + height);

	addBuffer("closepath S ");
}

void addRectCenter( double x, double y, double width, double height )
{
	double a, b, x0, y0;
	a = width / 2.0;
	b = height / 2.0;
	x0 = x - a;
	y0 = y - b;

	addBuffer("0.1 W ");

	addMove(x0, y0);
	addLine(x0 + width, y0);
	addLine(x0 + width, y0 + height);
	addLine(x0, y0 + height);

	addBuffer("closepath S ");
}

char szPaperSizes[9][16] =
{ "letter", "a4", "legal", "a3", "11x17", "a1", "archD", "a0", "b0" };

double fPaperWidths[9] =
{ 21.59, 20.9903, 21.59, 29.7039, 27.94, 59.4078, 60.96, 93.9611, 100.048 };

double fPaperHeights[9] =
{ 27.94, 29.7039, 35.56, 41.9806, 43.18, 83.9611, 91.44, 118.816, 141.393 };

void CalcPageSize( void )
{
	// double fPrintableWidth;
	// double fPrintableHeight;
	// char szPagesize[32];
	int nPage;
	int i;

	// Values in paper table are from gs_statd.ps and need to be converted
	// Assumption is 12.7 mm margin all around (0.5 in)
	for ( i = 0; i < 9; i++ )
	{
		fPaperWidths[i] = 10.0 * fPaperWidths[i] - 25.4;
		fPaperHeights[i] = 10.0 * fPaperHeights[i] - 25.4;
	}

	// Initially assume we will fit on letter size paper
	nPage = 0;
	fPrintableWidth = fPaperWidths[0];
	fPrintableHeight = fPaperHeights[0];

	// Now test each paper size to see what we can fit this panel on to
	for ( i = 0; i < 8; i++ )
	{
		if ( fPanelHeight > fPrintableWidth )
		{
			//	  fprintf(stderr,"Panel is too high for %s. (%f > %f)\n",szPaperSizes[nPage],fPanelHeight,fPrintableWidth);
			nPage = i + 1;
			fPrintableWidth = fPaperWidths[nPage];
			fPrintableHeight = fPaperHeights[nPage];
		}
		if ( fPanelWidth > fPrintableHeight )
		{
			//	  fprintf(stderr,"Panel is too wide for %s. (%f > %f)\n",szPaperSizes[nPage],fPanelWidth,fPrintableHeight);
			nPage = i + 1;
			fPrintableWidth = fPaperWidths[nPage];
			fPrintableHeight = fPaperHeights[nPage];
		}
	}

	// Need to have the paper size name for warning at end of program
	strcpy(szPagesize, szPaperSizes[nPage]);
}


/* Generate a panel */
int main( int argc, char *argv[] )
{
	int i;
	char szWork[132];
	FILE *f;
	double fPanelLeftCorner, fPanelBottomCorner;

	/* sign on banner */
	fprintf(stderr, "rcrpanel Revision: 3.6.1  of %s %s.\n", __DATE__, __TIME__);

	initParams();

	/* Open the file and read it, if available */
	if ( argc < 2 )
	{
		fprintf(stderr, "Usage: %s <panel description file> > <output_file.ps>\r\n", argv[0]);
		return 1;
	}
	else if ( argc > 1 )
	{	strcpy(szFilename, argv[1]);
		f = fopen(szFilename, "r");
		if ( f == NULL )
		{
			perror("Opening script file");
			return 8;
		}
		getFile(f);
		fclose(f);
	}

	CalcPageSize();
	addHeader();

	memset(szBuffer, 0, sizeof(szBuffer));

	/* Calculate location of panel corner.  */
	fPanelLeftCorner = (fPrintableHeight - fPanelWidth) / 2.0 + 12.7;
	fPanelBottomCorner = (fPrintableWidth - fPanelHeight) / 2.0 - fPrintableWidth - 12.7;
	if ( nReverse )
	{
		sprintf(szWork, "90 rotate -2.83464567 2.83464567 scale %f %f translate ", fPanelLeftCorner - fPrintableHeight, fPanelBottomCorner);
		addBuffer(szWork);
	}
	else
	{
		sprintf(szWork, "90 rotate 2.83464567 2.83464567 scale %f %f translate ", fPanelLeftCorner, fPanelBottomCorner);
		addBuffer(szWork);
	}

	/* Draw panel outline */
	addBuffer("gsave 0.2 W ");

	addColor( crBackground );
	addBuffer("newpath ");

	addBuffer("0 0 M ");
	sprintf(szWork, "%f 0 L ", fPanelWidth);
	addBuffer(szWork);
	sprintf(szWork, "%f %f L ", fPanelWidth, fPanelHeight);
	addBuffer(szWork);
	sprintf(szWork, "0 %f L ", fPanelHeight);
	addBuffer(szWork);
	addBuffer("0 0 L ");
	addBuffer("closepath fill ");

	addColor(0);
	addBuffer("0 0 M ");
	sprintf(szWork, "%f 0 L ", fPanelWidth);
	addBuffer(szWork);
	sprintf(szWork, "%f %f L ", fPanelWidth, fPanelHeight);
	addBuffer(szWork);
	sprintf(szWork, "0 %f L ", fPanelHeight);
	addBuffer(szWork);
	addBuffer("0 0 L ");
	addBuffer("S ");
	addBuffer("grestore ");



	for ( i = 0; i < nNumControls; i++ )
		addControl( rcControl[i].x, rcControl[i].y, rcControl[i].diam );

	for (int i = 0; i < nNumRectangles; i++)
		addRectangle( rcRectangle[i].x0, rcRectangle[i].y0, rcRectangle[i].fRectangleWidth, rcRectangle[i].fRectangleHeight );

	for (int i = 0; i < nNumCtrRectangles; i++)
		addRectCenter( rcRectCenter[i].x, rcRectCenter[i].y, rcRectCenter[i].fRectangleWidth, rcRectCenter[i].fRectangleHeight);

	for ( i = 0; i < nNumDials; i++ )
		addDial( i );

	for ( i = 0; i < nNumStrings; i++ )
		addString( i );

	addBuffer("showpage ");
	purgeBuffer();
	printf("%%%%Trailer\r\n");

	if ( strcmp(szPagesize, "letter") )
		fprintf(stderr, "\n\n***NOTE: When doing ps2pdf use -sPAPERSIZE=%s\n", szPagesize);

	return 0;
}
