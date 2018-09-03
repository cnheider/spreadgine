#include <spreadgine.h>
#include <spreadgine_util.h>
#include <CNFG3D.h>
#include <unistd.h>
#include <os_generic.h>
#include <linmath.h>

void HandleKey( int keycode, int bDown )
{
}

void HandleButton( int x, int y, int button, int bDown )
{
}

void HandleMotion( int x, int y, int mask )
{
}

int main()
{
#if defined( MALI ) || defined( RASPI_GPU )
	Spreadgine * e = SpreadInit( 1920, 1080, "Spread Test", 8888, 2, stderr );
#else
	Spreadgine * e = SpreadInit( 800, 600, "Spread Test", 8888, 2, stderr );
#endif

	//First: Add a defualt shader
	SpreadShader * shd1 = SpreadLoadShader( e, "shd1", "assets/autobatch.frag", "assets/autobatch.vert", 0 );
	if( !shd1 )
	{
		fprintf( stderr, "Error making shader.\n" );
	}

	SpreadGeometry * sixsquare = MakeSquareMesh( e, 3, 3 );
	BatchedSet * batched   = CreateBatchedSet( e, "batchedTri", 1024, 65536, GL_TRIANGLES, 2048, 2048 );

	float eye[3] = { .014, 5, 5 };
	float at[3] =  { 0, 0, 0 };
	float up[3] =  { 0, 0, 1 };
	tdLookAt( e->vpviews[0], eye, at,up );
	tdTranslate( e->vpviews[0], -.4, 0, 0 ); //Shift vanishing point
	eye[0] = -.014;
	tdLookAt( e->vpviews[1], eye, at,up );
	tdTranslate( e->vpviews[1], .4, 0, 0 ); //Shift vanishing point

	SpreadChangeCameaView(e, 0, e->vpviews[0] );
	SpreadChangeCameaView(e, 1, e->vpviews[1] );

	tdMode( tdMODELVIEW );
	tdIdentity( gSMatrix );
	tdTranslate( gSMatrix, 0., 0., 0. );
	tdScale( gSMatrix, .1, .1, .1 );		//Operates ON f
	tdTranslate( gSMatrix, 00., 0., 0. );

	int frames = 0, tframes = 0;
	double lastframetime = OGGetAbsoluteTime();

#define NUMBATCHO 500

	BatchedObject * objs[NUMBATCHO];
	int i;
	for( i = 0; i < NUMBATCHO; i++ )
	{
		char stname[1024];
		sprintf( stname, "obj%03d",i);
		objs[i] = AllocateBatchedObject( batched, sixsquare, stname );
		UpdateBatchedObjectTransformData( objs[i], FTriple(  (i % 10) * 1.5, (i / 10) * 1.5, 0 ), FQZero, FPZero, 1.0 );
	}



	while(1)
	{
		double Now = OGGetAbsoluteTime();
		spglClearColor( e, .0, 0.0, 0.0, 1.0 );
		spglClear( e, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		spglEnable( e, GL_DEPTH_TEST );

		spglLineWidth( e, 4 );

/*		int slot = SpreadGetUniformSlot( shd1, "texSize0");
		if( slot >= 0 )
		{
			float ssf[4] = { 2048, 2048, 0, 0 };
			SpreadUniform4f( shd1, slot, ssf );
		}
		else
		{
			fprintf( stderr, "Error: Can't find parameter in shader\n" );
		}
*/

//		SpreadApplyTexture( tex, 0 );
//		SpreadApplyShader( shd1 );

/*
		tdPush();
		//tdScale( gSMatrix, .1, .1, .1 );
		SpreadRenderGeometry( plat2, gSMatrix, 0, -1 ); 
		tdPop();
*/

		//tdTranslate( modelmatrix, 0, 0, .1 );


		for( i = 0; i < NUMBATCHO; i++ )
		{
			char stname[1024];
			sprintf( stname, "obj%03d",i);

			double euler[3] = { 0, 0, tframes*.1 };
			LinmathQuat q;
			quatfromeuler( q, euler );

			float quat[4] = { q[0], q[1], q[2], q[3] }; 
			UpdateBatchedObjectTransformData( objs[i], FTriple(  (i % 10) * 1.5, ((i / 10)%10) * 1.5, sin(i*.2+tframes*.1) + (i/100) ), quat, FPZero, 1.0 );
		}

		tdPush();
		tdIdentity( gSMatrix );
		tdRotateEA( gSMatrix, 0,.2125,1 );		//Operates ON f
		tdScale( gSMatrix, .3, .3, .3 );
		RenderBatchedSet( batched, shd1, gSMatrix );


//		SpreadApplyShader( shd1 );
//		SpreadRenderGeometry( sixsquare, gSMatrix, 0, -1 );
		tdPop();

	
/*
		StartImmediateMode( batchedTri );
		tdPush();
		tdIdentity( gSMatrix );
		tdScale( gSMatrix, 2, 2, 2 );
		tdTranslate( gSMatrix, -4., -4., 0. );
		for( y = 0; y < 15; y++ )
		{
			tdTranslate( gSMatrix, 0.0, 1.1, 0 );
			tdPush();
			for( x = 0; x < 15; x++ )
			{
				tdTranslate( gSMatrix, 1.1, 0, 0 );
				tdPush();
				tdTranslate( gSMatrix, 0, 0, sin(x*.3+y*.2+tframes*.1)*5 );
				//int rstart = ((tframes)*6)%36;
				//				SpreadRenderGeometry( e->geos[0], gSMatrix, 0, -1 ); 
				float tcoff[4] = { x/10., y/10., 0, 0 };
				float tcscale[4] = { .1, .1, 0, 0 };
				ImmediateModeMesh( plat2, gSMatrix, 0, 0, tcoff, tcscale );
				//SpreadRenderGeometry( batchedTri, gSMatrix, 0, -1 ); 
				//SpreadRenderGeometry( plat2, gSMatrix, 0, -1 ); 
				tdPop();
			}
			tdPop();
		}
		tdPop();

		printf( "%d %d\n", batchedTri->indices, batchedTri->verts );
		static int set;
		if( set == 0 )
		{
			set = 1;
			UpdateMeshToGen( batchedTri );
		}
		tdPush();
		tdScale( gSMatrix, 1., 1., 1. );

		SpreadRenderGeometry( batchedTri, gSMatrix, 0, -1 ); 
		//SpreadRenderGeometry( plat2, gSMatrix, 0, -1 ); 
		//SpreadRenderGeometry( &e->geos[0], gSMatrix, 0, -1 ); 
		tdPop();

		//SpreadRenderGeometry( e->geos[0], gSMatrix, 0, -1 ); 
*/
//
//		usleep(20000);
		spglSwap( e );
		SpreadCheckShaders( e );
		frames++;
		tframes++;
		if( Now - lastframetime > 1 )
		{
			printf( "FPS: %d\n", frames );
			frames = 0;
			lastframetime++;
		}
	}
}
