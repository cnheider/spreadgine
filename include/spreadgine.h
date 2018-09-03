//Copyright <>< 2018 Charles Lohr, under the MIT-x11 or NewBSD licenses, you choose.

#ifndef _SPREADGINE_H
#define _SPREADGINE_H

#include <stdint.h>
#include <stdio.h>

#ifdef GLES2
#include <GLES2/gl2.h>
#elif defined( EGL )
#include <EGL/egl.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif


typedef struct SpreadShader SpreadShader;
typedef struct SpreadGeometry SpreadGeometry;
typedef struct SpreadTexture SpreadTexture;
typedef struct Spreadgine Spreadgine;


#define SPREADGINE_VIEWPORTS 2
#define SPREADGINE_CAMERAS   10
#define SPREADGINE_CIRCBUF   (65536*32)	//A 2MB circular buffer.

#define SPREADGINE_CACHEMAP_SIZE 1307
#define SPREADGINE_MAXKEEP 2048

#define SPREADGINE_T_FLOAT 0
#define SPREADGINE_T_UBYTE 1

#define MAX_ATTRIBUTES 8

//Convenience structs for compound literals.
struct FTriple_t { float x, y, z; };
struct FQuad_t   { float w, x, y, z; };
#define FTriple( x, y, z )  (const float*)&((struct FTriple_t){ x, y , z })
#define FQuad( w, x, y, z ) (const float*)&((struct FQuad_t){ w, x, y , z })

//Having these fixed is faster and smaller than the compound literals.
extern const float FPIdentity[16]; //Identity matrix
extern const float FPZero[16]; //All zeroes
extern const float FQZero[4];

struct SpreadHashEntry
{
	struct SpreadHashEntry * next;
	char * key;
	int payload_reserved;
	int entry_in_KEEPlist;
	uint32_t payloadlen;
	uint8_t * payload;
};

typedef struct SpreadHashEntry SpreadHashEntry;


struct Spreadgine
{
	SpreadShader ** shaders;
	int setshaders;
	SpreadGeometry ** geos;
	int setgeos;
	SpreadTexture ** textures;
	int settexs;

	int current_shader;

	//Matrix management appens in CNFG3D.h
	float (*vpperspectives)[16];  //Perspective matrix per-eye.
	float (*vpviews)[16];   //View matrix per-eye.
	char ** vpnames;
	int   vpedges[SPREADGINE_VIEWPORTS][4]; //x,y,w,h
	int   setvps;

	FILE * fReport;

	//Internal.  NO TOUCHIE!
	uint32_t		cbhead;	//Head counts up all the way to int_max.  Be sure to modulus whenever indexing into spreadthread.
	uint8_t * cbbuff;
	void * spreadthread;
	float  lastclearcolor[4];

	//These are filled with messages that will be transferred to newly connected clients.
	SpreadHashEntry * KEEPhash[SPREADGINE_CACHEMAP_SIZE];
	SpreadHashEntry * KEEPlist[SPREADGINE_MAXKEEP];
	int KEEPlistnum;
	void * KEEPmutex;
	volatile int doexit;
};


//Initializes the spreadgine with a basic shader 'default', a cube 'cube', and a texture 'gradiant'
//It also initializes `vps` views with camera at the origin. For 99.9% of applications, vps should be 2.
//Be sure to set everything up before a client connects. None of these functions will update the client.
//Also... 'fReport' can either be stderr or something you make with open_memstream
Spreadgine * SpreadInit( int w, int h, const char * title, int httpport, int vps, FILE * fReport );
void SpreadDestroy( Spreadgine * spr );


void spglEnable( Spreadgine * e, uint32_t en );
void spglDisable( Spreadgine * e, uint32_t de );
void spglLineWidth( Spreadgine * e, float wid );
void spglSwap(Spreadgine * e);
void spglClearColor( Spreadgine * e, float r, float g, float b, float a );
void spglClear( Spreadgine * e, uint32_t clearmask );
//TODO: Add more GL stand-ins here.  Note that only functions available in GLES, GL and WebGL should be made available here.


/////////////////////////////CAMERAS//////////////////////////////

void SpreadSetupCamera( Spreadgine * spr, uint8_t camid, float fov, float aspect, float near, float far, const char * camname );
void SpreadChangeCameaPerspective( Spreadgine * spr, uint8_t camid, float * newpersp );
void SpreadChangeCameaView( Spreadgine * spr, uint8_t camid, float * newview );


/////////////////////////////SHADERS///////////////////////////////

struct SpreadShader
{
	Spreadgine * parent;
	uint32_t shader_in_parent;
	char * shadername;

	char * fragment_shader_source;
	char * vertex_shader_source;
	char * geometry_shader_source;

	double fragment_shader_time;
	double vertex_shader_time;
	double geometry_shader_time;

	//Need a way to translate uniform locations to client monitors...
	//These use "index" as the slot.
	char ** uniform_slot_names;
	int  * uniform_slot_name_lens;
	int known_uniform_slots;
	float ** uniforms_in_slots;
	int * size_of_uniforms;

	//"Required" uniform matrix entries.
	int model_index;  // YOU MUST INCLUDE "mmatrix" so spreadgine knows where to shove the camera.
	int view_index;  // YOU MUST INCLUDE "vmatrix" so spreadgine knows where to shove the camera.
	int perspective_index; // YOU MUST INCLUDE "pmatrix" so spreadgine knows where to shove the camera.

	//Internal backend types.
	int vertex_shader;
	int fragment_shader;
	int geometry_shader;
	int program_shader;
	int attribute_slots[MAX_ATTRIBUTES];  //attrib0..attrib7
};

SpreadShader * SpreadLoadShader( Spreadgine * spr, const char * shadername, const char * fragmentShader, const char * vertexShader, const char * geometryShader );
int SpreadGetUniformSlot( SpreadShader * shd, const char * slotname );
void SpreadUniform4f( SpreadShader * shd, int slot, const float * uni );
void SpreadUniform16f( SpreadShader * shd, int slot, const float * uni );
void SpreadFreeShader( SpreadShader * shd );
void SpreadApplyShader( SpreadShader * shd );
void SpreadCheckShaders( Spreadgine * spr );

//////////////////////////////GEOMETRY/////////////////////////////
struct SpreadGeometry
{
	Spreadgine * parent;
	uint32_t geo_in_parent;
	char * geoname;

	//Array[0] = Indices
	//Array[1] = Position
	//Array[2] = Color	
	//Array[3] = Texutre Coordinates
	//Array[4] = Normal

	int indices;
	uint16_t * indexarray;

	uint32_t ibo;
	uint32_t * vbos;

	void ** arrays;
	uint8_t * strides;
	uint8_t * types; 		//always GL_FLOAT (0), GL_UNSIGNED_BYTE (1), must be numerically 0 or 1.
	int laststartv;

	int numarrays;

	int render_type;	//GL_TRIANGLES, GL_POINTS, GL_LINES, etc.
	int verts;

	int max_set; //Not used inside core spreadgine, more for immediate mode.

	void * user; //Used for things like immediate mode, etc.
};

SpreadGeometry * SpreadCreateGeometry( Spreadgine * spr, const char * geoname, int render_type, int indices, uint16_t * indexbuffer, int verts, int nr_arrays, const void ** arrays, int * strides, int * types );
void UpdateSpreadGeometry( SpreadGeometry * geo, int arrayno, void * arraydata ); //If arrayno == -1, update everything.  -2 = update everything AND update VBOs
void SpreadRenderGeometry( SpreadGeometry * geo, const float * modelmatrix, int startv, int numv ); 	//If nv = -1, run until end of list.
void SpreadFreeGeometry( SpreadGeometry * geo );


//////////////////////////////TEXTURES//////////////////////////////

//Not implementing textures yet.

struct SpreadTexture
{
	Spreadgine * parent;
	uint32_t texture_in_parent;
	char * texname;

	GLuint textureID;
	int pixwid;				//# of bytes per pixel.
	int channels; 			//Becomes GL_RED, GL_RG, GL_RGB, GL_RGBA
	int type;				//Supported: GL_FLOAT and GL_UNSIGNED_BYTE
	uint8_t * pixeldata;
	int w, h;
};


SpreadTexture * SpreadCreateTexture( Spreadgine * spr, const char * texname, int w, int h, int chan, int mode ); //Typically chan=4, and mode=GL_UNSIGNED_BYTE (GL_FLOAT not supported on rpi)
void SpreadUpdateSubTexture( SpreadTexture * tex, void * texdat, int x, int y, int w, int h );
void SpreadApplyTexture( SpreadTexture * tex, int slot );
void SpreadFreeTexture( SpreadTexture * tex );


#endif


