/* $Header: /home/jjmcd/rcs/Dial/Dial.c,v 2.0 2003-04-26 10:59:00-04 jjmcd Rel jjmcd $ */
/* Draw a dial - platform independent version
 *
 *
 * Application creates PostScript to draw a dial according to specifications
 * in a script file.
 *
 * Usage:
 *
 *    Dial [filename] > [psfile]
 *
 *    where [filename] is the name of a text file containing the description
 *    of the dial to be drawn and [psfile] is the PostScript output.
 *
 *    If the argument is omitted, a default dial is drawn.
 *
 * The input file is a text file containing lines each describing a
 * particular aspect of the dial.  If a particular aspect is not provided,
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
 *     StartAngle = 90.0
 *
 *  The measurements are in units of 1/300th of an inch (approx 0.085mm).
 *  This makes the units approximately the same as for the Windows version
 *  for most users.
 *
 *  Colors are hexadecimal, with 2 nybbles each for red, green and blue
 *  respectively.  For example, a medium purple would be coded 0x808000.
 *
 *  Any unrecognized lines in the file are ignored.  As lines are understood,
 *  the result is displayed on stderr.
 *
 *
 *      $Author: jjmcd $
 *      $Date: 2003-04-26 10:59:00-04 $
 *      $Revision: 2.0 $
 *      $State: Rel $
 *
 *      $Log: Dial.c,v $
 *      Revision 2.0  2003-04-26 10:59:00-04  jjmcd
 *      Add ability to rotate dial starting point
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

#define COLORREF long
double fRadius;
double fSpan;
int nNumTicks;
int nBigPer;
int nReverse;
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

char szBuffer[2048];

/* Read the input file */
void getFile( FILE *f )
{
  while ( !feof(f) )
    {
      fgets(szBuffer,sizeof(szBuffer),f);
      if ( !feof(f) )
	{
	  if ( !strncmp("Radius",szBuffer,6) )
	    {
	      sscanf(&szBuffer[8],"%lf",&fRadius);
	      fprintf(stderr,"                     Radius: %lf\n",fRadius);
	    }
	  else if ( !strncmp("Span",szBuffer,4) )
	    {
	      sscanf(&szBuffer[6],"%lf",&fSpan);
	      fprintf(stderr,"                       Span: %lf\n",fSpan);
	    }
	  else if ( !strncmp("NumTicks",szBuffer,8) )
	    {
	      sscanf(&szBuffer[10],"%d",&nNumTicks);
	      fprintf(stderr,"       Number of Tick marks: %d\n",nNumTicks);
	    }
	  else if ( !strncmp("BigPer",szBuffer,6) )
	    {
	      sscanf(&szBuffer[8],"%d",&nBigPer);
	      fprintf(stderr,"        Small ticks per big: %d\n",nBigPer);
	    }
	  else if ( !strncmp("SizeTicks",szBuffer,9) )
	    {
	      sscanf(&szBuffer[11],"%lf",&fSizeTicks);
	      fprintf(stderr,"   Size of small tick marks: %lf\n",fSizeTicks);
	    }
	  else if ( !strncmp("SizeBig",szBuffer,7) )
	    {
	      sscanf(&szBuffer[9],"%lf",&fSizeBig);
	      fprintf(stderr,"     Size of big tick marks: %lf\n",fSizeBig);
	    }
	  else if ( !strncmp("StartingIndicator",szBuffer,17) )
	    {
	      sscanf(&szBuffer[19],"%lf",&fStartingIndicator);
	      fprintf(stderr,"         Starting indicator: %lf\n",fStartingIndicator);
	    }
	  else if ( !strncmp("IncrementPerBigTick",szBuffer,19) )
	    {
	      sscanf(&szBuffer[21],"%lf",&fIncrementPerBigTick);
	      fprintf(stderr,"Increment per big tick mark: %lf\n",fIncrementPerBigTick);
	    }
	  else if ( !strncmp("SizeFont",szBuffer,8) )
	    {
	      sscanf(&szBuffer[11],"%lf",&fSizeFont);
	      fprintf(stderr,"            Size of numbers: %lf\n",fSizeFont);
	    }
	  else if ( !strncmp("ColorCircle",szBuffer,11) )
	    {
	      sscanf(&szBuffer[13],"%lx",&crCircle);
	      fprintf(stderr,"            Color of circle: 0x%06lx\n",crCircle);
	    }
	  else if ( !strncmp("ColorTickMarks",szBuffer,14) )
	    {
	      sscanf(&szBuffer[16],"%lx",&crTickMarks);
	      fprintf(stderr,"  Color of small tick marks: 0x%06lx\n",crTickMarks);
	    }
	  else if ( !strncmp("ColorBigTickMarks",szBuffer,17) )
	    {
	      sscanf(&szBuffer[19],"%lx",&crBigTickMarks);
	      fprintf(stderr,"    Color of big tick marks: 0x%06lx\n",crBigTickMarks);
	    }
	  else if ( !strncmp("ColorText",szBuffer,9) )
	    {
	      sscanf(&szBuffer[11],"%lx",&crText);
	      fprintf(stderr,"           Color of numbers: 0x%06lx\n",crText);
	    }
	  else if ( !strncmp("StartAngle",szBuffer,4) )
	    {
	      sscanf(&szBuffer[12],"%lf",&fStartAngle);
	      fprintf(stderr,"                Start Angle: %lf\n",fStartAngle);
	    }
	  else if ( !strncmp("Reverse",szBuffer,7) )
	    {
	      nReverse = 1;
	      fprintf(stderr,"                    Reverse: TRUE\n");
	    }
	  else if (strlen(szBuffer)> 1 )
	    {
	      if ( szBuffer[strlen(szBuffer)-1] < '!' )
		szBuffer[strlen(szBuffer)-1] = '\0';
	      fprintf(stderr,"***[%s] NOT UNDERSTOOD***\n",szBuffer);
	    }
	}
    }
}

/* Set default parameters in case no file or missing from file */
void initParams( void )
{
  fRadius = 300.0;
  fSpan = 270.0;
  nNumTicks = 81;
  nBigPer = 10;
  nReverse = 0;
  fSizeTicks = 20.0;
  fSizeBig = 50.0;
  crCircle = 0x000000;
  crTickMarks = 0x000000;
  crBigTickMarks = 0x000000;
  crText = 0x000000;
  fSizeFont = 60.0;
  fStartingIndicator = 14.0;
  fIncrementPerBigTick = 0.01;
  fStartAngle = 0.0;
}

/* Dump the PostScript from the buffer */
void purgeBuffer( void )
{
  if ( szBuffer[strlen(szBuffer)-1] == ' ' )
    szBuffer[strlen(szBuffer)-1] = '\0';
  printf("%s\r\n",szBuffer);
  memset(szBuffer,0,sizeof(szBuffer));
}

/* Add a string to the buffer */
void addBuffer( char *p )
{
  if ( strlen(szBuffer) > 72 )
    purgeBuffer();
  strcat(szBuffer,p);
}

/* Add a setrgbcolor command to the buffer */
void addColor( long rgb )
{
  double x;
  char szWork[32];

  x = ( (double) ( (rgb&0xff0000) >> 16 ) ) / 256.0;
  sprintf(szWork,"%4.2f ",x);
  addBuffer(szWork);
  x = ( (double) ( (rgb&0xff00) >> 8 ) ) / 256.0;
  sprintf(szWork,"%4.2f ",x);
  addBuffer(szWork);
  x = ( (double) ( (rgb&0xff) ) ) / 256.0;
  sprintf(szWork,"%4.2f ",x);
  addBuffer(szWork);
  addBuffer("C ");
}

/* Add a lineto command to the buffer */
void addLine( int x, int y )
{
  char szWork[32];
  sprintf(szWork,"%d ",x);
  addBuffer(szWork);
  sprintf(szWork,"%d ",y);
  addBuffer(szWork);
  addBuffer("L ");
}

/* Add a moveto command to the buffer */
void addMove( int x, int y )
{
  char szWork[32];
  sprintf(szWork,"%d ",x);
  addBuffer(szWork);
  sprintf(szWork,"%d ",y);
  addBuffer(szWork);
  addBuffer("M ");
}

/* Calculate the format for display of values */
void setFormat( char *szFormat )
{
  int left,right;

  strcpy(szFormat,"%lf");
  if ( fStartingIndicator < 9 )
    left = 1;
  else if ( fStartingIndicator < 90 )
    left = 2;
  else if ( fStartingIndicator < 900 )
    left = 3;
  else if ( fStartingIndicator < 9000 )
    left = 4;
    else
      left = 5;
  if ( fIncrementPerBigTick > 0.9 )
    right = 0;
  else if ( fIncrementPerBigTick > 0.09 )
    right = 1;
  else if ( fIncrementPerBigTick > 0.009 )
    right = 2;
  else if ( fIncrementPerBigTick > 0.0009 )
    right = 3;
  else if ( fIncrementPerBigTick > 0.00009 )
    right = 4;
  else if ( fIncrementPerBigTick > 0.000009 )
    right = 5;
  else if ( fIncrementPerBigTick > 0.0000009 )
    right = 6;
    else
      right = 7;
  sprintf(szFormat,"%%%d.%dlf",left,right);
}

/* Add the PostScript header to the output stream */
void addHeader( void )
{
  printf("%%!PS-Adobe-2.0\r\n");
  printf("%%%%Title: Sample\r\n");
  printf("%%%%Creator: Dial $Revision: 2.0 $\r\n");
  printf("%%%%Pages: 1\r\n");
  printf("%%%%BeginProcSet: TextProcs 1.0 0\r\n");
  printf("/F { findfont exch scalefont setfont } bind def /L { lineto } bind def /C\r\n");
  printf("{ setrgbcolor } bind def /M { moveto } bind def /S { stroke } bind def /W\r\n");
  printf("{ setlinewidth } bind def /T { show } bind def /TR { dup stringwidth exch\r\n");
  printf("neg exch rmoveto show } bind def /TC { dup stringwidth exch 2 div neg\r\n");
  printf("exch rmoveto show } bind def\r\n");
  printf("%%%%EndProcSet\r\n");
  printf("%%%%EndProlog\r\n");
}

/* Generate a dial */
int main( int argc, char *argv[] )
{
  int i;
  char szWork[132];
  double x,y,x0,y0;
  double theta,theta0;
  int ix,iy;
  double fCurrentIndication;
  int nBig;
  char szIndicate[128],szFormat[32];
  char *p;
  FILE *f;
  double fTop,fDelta;

  /* sign on banner */
  fprintf(stderr,"Dial $Revision: 2.0 $ of %s.\n",__DATE__);

  initParams();

  /* Open the file and read it, if available */
  if ( argc > 1 )
    {
      f = fopen(argv[1],"r");
      if ( f==NULL )
	{
	  perror("Opening script file");
	  return 8;
	}
      getFile(f);
      fclose(f);
    }

  addHeader();

  memset(szBuffer,0,sizeof(szBuffer));
  if ( nReverse )
    addBuffer("-0.24 0.24 scale ");
  else
    addBuffer("0.24 0.24 scale ");

  /* Draw circle */

  fTop = 3.0 * 3.14159268 / 2.0;     /* Angle at top of dial */
  fDelta = 3.14159268 / 18.0;        /* Range of angles to center number */

  /* Make the circle bolder for larger dials */
  if ( fRadius > 400.0 )
    addBuffer("gsave 8 W ");
  else
    addBuffer("gsave 4 W ");

  addColor(crCircle);

  x0 = 288.0 / 0.24;                 /* Change scaling from default points */
  if ( nReverse )
    x0 = -x0;
  y0 = 432.0 / 0.24;                 /* to 1/300's inch for better resolution */

  theta0 = 90.0+(360.0-fSpan) / 2.0 + fStartAngle; /* Starting angle */

  for ( i=0; i<1000; i++ )           /* Will draw circle in 1000 segments */
    {
      theta = 3.14159268 * (theta0 + ((double) i) * fSpan / 999.0) / 180.0;
      x = fRadius * cos( theta );
      y = fRadius * sin( theta );
      ix = (int) (x + x0 + 0.5);
      iy = (int) (y0 - y + 0.5);
      if ( i )
	  addLine(ix,iy);
      else
	  addMove(ix,iy);
    }
  addBuffer("S grestore ");

  /* Draw ticks and annotation */

  if ( fSizeTicks > 20.0 )           /* Use a heavier line if ticks are long */
    addBuffer("gsave 4 W ");
  else
    addBuffer("gsave 2 W ");
  addColor(crTickMarks);             /* Color of small tick marks */
  setFormat( szFormat );             /* Calculate format for numbers */

  /* Set up the font of the right size */
  sprintf(szWork,"%4.0f ",fSizeFont);
  p = szWork;
  while ( *p == ' ')
    p++;
  addBuffer(p);
  addBuffer("/Helvetica-Bold ");
  addBuffer("F ");

  /* Loop through all the ticks */
  nBig = 0;
  for ( i=0; i<nNumTicks; i++ )
    {
      double nn = nNumTicks-1;
      theta = 3.14159268 * (theta0 + ((double) i) * fSpan / nn) / 180.0;
      x = fRadius * cos( theta );
      y = fRadius * sin( theta );
      ix = (int) (x + x0 + 0.5);
      iy = (int) (y0 - y + 0.5);
      addMove(ix,iy);

      if ( i % nBigPer )             /* Is it a big one? */
	{                            /* No */
	  x = (fRadius+fSizeTicks) * cos( theta );
	  y = (fRadius+fSizeTicks) * sin( theta );
	  ix = (int) (x + x0 + 0.5);
	  iy = (int) (y0 - y + 0.5);
	  addLine(ix,iy);
	  addBuffer("S ");
	}
      else
	{                            /* Yes */
	  addColor(crBigTickMarks);
	  x = (fRadius+fSizeBig) * cos( theta );
	  y = (fRadius+fSizeBig) * sin( theta );
	  ix = (int) (x + x0 + 0.5);
	  iy = (int) (y0 - y + 0.5);
	  addLine(ix,iy);
	  addBuffer("S ");

	  /* Now need to annotate the big tick mark */
	  fCurrentIndication = fStartingIndicator + nBig * fIncrementPerBigTick;
	  nBig++;
	  sprintf(szIndicate,szFormat,fCurrentIndication);

	  addColor(crText);          /* Set text color */
	  addMove(ix,iy);
	  sprintf(szWork,"(%s) ",szIndicate);
	  addBuffer(szWork);
	  /* If the tick is near the top, center text over tick */
	  if ( fabs(theta-fTop) < fDelta )
	    addBuffer("TC ");
	  /* otherwise if the tick is on the left, start left of the tick */
	  else if ( theta < fTop )
	    addBuffer("TR ");
	  /* Otherwise start at the tick */
	  else
	    addBuffer("T ");
	  /* Reset to small tick color */
	  addColor(crTickMarks);
	}
    }

  /* Finally, place a drilling crosshair */
  addColor(0);
  addMove(x0-50,y0);
  addLine(x0+50,y0);
  addBuffer("S ");
  addMove(x0,y0-50);
  addLine(x0,y0+50);
  addBuffer("S ");

  addBuffer("grestore ");

  addBuffer("showpage ");
  purgeBuffer();
  printf("%%%%Trailer\r\n");

  return 0;
}
