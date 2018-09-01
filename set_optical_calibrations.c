#include <spreadgine.h>
#include <CNFG3D.h>
#include <unistd.h>
#include <os_generic.h>
#include <libsurvive/survive.h>
#include <string.h>
#include <math.h>
#include <spreadgine_vr.h>
#include <spreadgine_util.h>

void HandleKey( int keycode, int bDown )
{
}

void HandleButton( int x, int y, int button, int bDown )
{
}

void HandleMotion( int x, int y, int mask )
{
}

void HandleControllerInput()
{
	if( WM[0] )
	{
		static int vsmode;
		static double lasta1, lasta2;
		static int last_bm;
		int bm = WM[0]->buttonmask;
		if( !(last_bm & 1 ) && (bm &1) )
		{
			vsmode = (vsmode+1)%4;
		}
		if( vsmode == 0 ) 
			spglClearColor( gspe, .1, 0.2, 0.1, 1.0 );
		else if( vsmode == 1)
			spglClearColor( gspe, .2, 0.1, 0.1, 1.0 );
		else if( vsmode == 2 )
			spglClearColor( gspe, .1, 0.1, 0.2, 1.0 );
		else
			spglClearColor( gspe, .2, 0.2, 0.2, 1.0 );
			


		if( !(last_bm & 2 ) && (bm &2) )
		{
			lasta1 = WM[0]->axis1;
			lasta2 = WM[0]->axis2;
		}
		double x = WM[0]->axis2/32767.;
		double y = WM[0]->axis3/32767.;
		float last_ang = atan2( lasta1, lasta2 );
		float cur_ang = atan2( x, y );
		float delta = cur_ang - last_ang;
		if( delta > 3.14159*2 ) delta -= 3.14159*2;
		if( delta <-3.14159*2 ) delta += 3.14159*2;
		float rang = sqrt(WM[0]->axis1*WM[0]->axis1 + WM[0]->axis2*WM[0]->axis2);

		if( rang > 10000 && (bm &2) && (last_bm & 2 ) )
		{
			if( vsmode == 0 )
			{
				diopter += delta/100;
			}
			else if( vsmode == 1 )
			{
				disappearing+= delta/100;
			}
			else if( vsmode == 2 )
			{
				fovie += delta;
				SpreadSetupCamera( gspe, 0, fovie, (float)1080/1200, .01, 1000, "CAM0" );
				SpreadSetupCamera( gspe, 1, fovie, (float)1080/1200, .01, 1000, "CAM1" );
			}
			else
			{
				eyez += delta/10.0;
			}
			printf( "WM0: %6.3f %6.3f %6.3f %d %d %f %f %d %f %f %f %f\n", last_ang, cur_ang, delta, WM[0]->buttonmask, WM[0]->axis1, x, y, WM[0]->charge, disappearing, diopter, fovie, eyez );
		}
		last_bm = bm;
		lasta1 = x;
		lasta2 = y;
	}
}

int main( int argc, char ** argv )
{
	Spreadgine * e = gspe = SpreadInit( 2160, 1200, "Spread Game Survive Test", 8888, 2, stderr );

	gargc = argc;
	gargv = argv;


	tdMode( tdMODELVIEW );

	SpreadGeometry * gun = LoadOBJ( gspe, "assets/simple_gun.obj", 1, 1 );
	SpreadGeometry * platform = LoadOBJ( gspe, "assets/platform.obj", 1, 1 );


	//e->geos[0]->render_type = GL_LINES;
	UpdateSpreadGeometry( e->geos[0], -1, 0 );

	SpreadSetupVR();


	int x, y, z;

	int frames = 0, tframes = 0;
	double lastframetime = OGGetAbsoluteTime();
	double TimeSinceStart = 0;
	double Last = OGGetAbsoluteTime();
	double TimeOfLastSwap = OGGetAbsoluteTime();
	while(1)
	{
		double Now = OGGetAbsoluteTime();
		spglClearColor( e, .1, 0.1, 0.1, 1.0 );

/*
	int32_t buttonmask;
	int16_t axis1;
	int16_t axis2;
	int16_t axis3;
	int8_t charge;
	int8_t charging : 1;
	int8_t ison : 1;
	int8_t additional_flags : 6;
*/
		HandleControllerInput();


		spglClear( e, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		spglEnable( e, GL_DEPTH_TEST );

		spglEnable( e, GL_CULL_FACE );

		SpreadSetupEyes();

		int x, y;

        tdMode( tdMODELVIEW );
        tdIdentity( gSMatrix );

		spglLineWidth( e, 4 );

		SpreadApplyShader( e->shaders[0] );


		tdPush();

		//Draw watchmen
		tdPush();
		tdTranslate( gSMatrix, wmp[0].Pos[0], wmp[0].Pos[1], wmp[0].Pos[2] );
		tdRotateQuat( gSMatrix, wmp[0].Rot[0], wmp[0].Rot[1], wmp[0].Rot[2], wmp[0].Rot[3] );
		SpreadRenderGeometry( gun, gSMatrix, 0, -1 ); 
		tdPop();

		tdPush();
		tdTranslate( gSMatrix, wmp[1].Pos[0], wmp[1].Pos[1], wmp[1].Pos[2] );
		tdRotateQuat( gSMatrix, wmp[1].Rot[0], wmp[1].Rot[1], wmp[1].Rot[2], wmp[1].Rot[3] );
		SpreadRenderGeometry( gun, gSMatrix, 0, -1 ); 
		tdPop();

		SpreadRenderGeometry( platform, gSMatrix, 0, -1 ); 

//		printf( "%f %f %f / %f %f %f / %f %f %f\n", wmp[0].Pos[0], wmp[0].Pos[1], wmp[0].Pos[2], wmp[1].Pos[0], wmp[1].Pos[1], wmp[1].Pos[2], phmd.Pos[0], phmd.Pos[1], phmd.Pos[2] );

		tdPush();
		//tdScale( gSMatrix, .2, .2, .2 );		//Operates ON f

		float ssf[4] = { TimeSinceStart, 0, 0, 0 };
		int slot = SpreadGetUniformSlot( e->shaders[0], "timevec");
		if( slot >= 0 )
		{
			//printf( "%f\n", ssf[0] );
			SpreadUniform4f( e->shaders[0], slot, ssf );
		}


		for( z = -2; z < 3; z++ )
		{
			for( y = -2; y < 3; y++ )
			{
				for( x = -2; x < 3; x++ )
				{
					tdPush();
					tdTranslate( gSMatrix, x, y, z );
					//int rstart = ((tframes)*6)%36;
					tdPush();
					//float sm = sin(x*1.2+y*.3+z*.8+tframes*.05);
					//tdScale( gSMatrix, sm, sm, sm );
					//tdRotateEA( gSMatrix, tframes+z*10, tframes*3.+y*10, tframes*2+x*1 );
					tdScale( gSMatrix, .1, .1, .1 );
					SpreadRenderGeometry( e->geos[0], gSMatrix, 0, -1 ); 
					tdPop();
				}
			}
		}

		tdPop();

#ifndef RASPI_GPU
		glFlush();
		double TWS = OGGetAbsoluteTime();
		TWS = TimeOfLastSwap-TWS + 1.f/90.f - .0001;
		if( TWS > 0 )	usleep(TWS*1000000);
		TimeOfLastSwap = OGGetAbsoluteTime();
#endif
		spglSwap( e );

		SpreadCheckShaders( e );
		frames++;
		tframes++;
		TimeSinceStart += (Now-Last);
		if( Now - lastframetime > 1 )
		{
			printf( "FPS: %d\n", frames );
			frames = 0;
			lastframetime++;
		}
		Last = Now;
	}
}
