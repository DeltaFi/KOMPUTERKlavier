#include <stdio.h>
#include <math.h>

float lowesterror = 10;
int hitwithlowest = 0;

void MakeChart( unsigned short cdiv, int silent )
{
	float fs = 28636363. / (cdiv+1.);
	float fstart = 440. / 16.;

	float totalsqerror = 0.0;
	int octave, note;
	if( !silent ) printf( "#define CDIV %d\n", cdiv );
	if( !silent ) printf( "unsigned short notes[] PROGMEM = {" );
	for( octave = 0; octave < 8; octave++ )
	{
		if( !silent ) printf( "\n\t" );
		for( note = 0; note < 12; note++ )
		{
			float frequency = fstart * pow( 2, ((float)note)/12. + octave );
			unsigned short nout = (65536. * frequency / fs) + 0.5;
			float actualfreq = ((float)nout) / 65536. * fs;
			float variance = (actualfreq-frequency) / frequency;

			if( !silent ) printf( "%5d, ", nout );

			if( !silent ) fprintf( stderr, "%f / %f  -- %f\n", frequency, actualfreq, variance * 1200);

			totalsqerror += variance * variance * variance * variance * 100000000.;
		}
	}

	if( !silent ) printf( "};\n" );

	if( totalsqerror < lowesterror )
	{
		lowesterror = totalsqerror;
		hitwithlowest = cdiv;
	}
	fprintf( stderr, "%d, %f\n", cdiv, totalsqerror );
}

int main()
{
	unsigned short cdiv = 806;

//	MakeChart( cdiv );
	for( cdiv = 500; cdiv < 900; cdiv++ )
	{
		MakeChart( cdiv, 1 );
	}

	MakeChart( hitwithlowest, 0 );
	return 0;
}
