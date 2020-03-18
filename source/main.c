/* 
   TINY3D sample / (c) 2010 Hermes  <www.elotrolado.net>

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>

#include <io/pad.h>

#include <tiny3d.h>
#include <libfont.h>

//#include"sqlite3.h"

//#include "db_support.h"
#include "ps3sqlite.h"

// font 1: 224 chr from 32 to 255, 16 x 32 pix 2 bit depth
#include "font_b.h"

char textToDraw[3][128] = {"","",""};
int row =3;

// draw one background color in virtual 2D coordinates

void DrawBackground2D(u32 rgba)
{
    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(0  , 0  , 65535);
    tiny3d_VertexColor(rgba);

    tiny3d_VertexPos(847, 0  , 65535);

    tiny3d_VertexPos(847, 511, 65535);

    tiny3d_VertexPos(0  , 511, 65535);
    tiny3d_End();
}

void drawScene()
{
	float x, y;
    tiny3d_Project2D(); // change to 2D context (remember you it works with 848 x 512 as virtual coordinates)
    DrawBackground2D(0x0040ffff) ; // light blue 


    SetFontSize(8, 16);
    
    x= 0.0; y = 0.0;

    SetCurrentFont(0);
    SetFontColor(0xffffffff, 0x0);
    x = DrawString(x,y, "Hello World!. My nick is ");
    SetFontColor(0x00ff00ff, 0x0);
    SetCurrentFont(0);
    x = DrawString(x,y, "Crystal ");
    SetCurrentFont(0);
    SetFontColor(0xffffffff, 0x0);
    x = DrawString(x,y, "and this is one sample working with SQLite on PS3 (was fonts :-P).");
    
    x= 0; y += 64;

    DrawString(x, y, "QUERY: SELECT name, developer, releasedate, genre, players FROM game");
   
    y += 18;
    //SetFontColor(0xffffffff, 0x00a000ff);
	int i=0;
	while (i< row) {
	DrawString(x, y, textToDraw[i]);
	y += 18;
	i++;
	}
    
}


void LoadTexture()
{

    u32 * texture_mem = tiny3d_AllocTexture(64*1024*1024); // alloc 64MB of space for textures (this pointer can be global)    

    u32 * texture_pointer; // use to asign texture space without changes texture_mem

    if(!texture_mem) return; // fail!

    texture_pointer = texture_mem;

    ResetFont();
    texture_pointer = (u32 *) AddFontFromBitmapArray((u8 *) font_b, (u8 *) texture_pointer, 32, 255, 16, 32, 2, BIT0_FIRST_PIXEL);
    
    // here you can add more textures using 'texture_pointer'. It is returned aligned to 16 bytes
}



s32 main(s32 argc, const char* argv[])
{


//printf("Version: %d\n", vfs.iVersion);
int rc = 1;

sqlite3 *mdb;
char buf[256];
char cache_path[100];
char sql[256];
snprintf(cache_path, sizeof(cache_path),"/dev_hdd0/game/PS3SQLITE");

snprintf(buf, sizeof(buf), "/dev_hdd0/game/PS3SQLITE/USRDIR/test.db");

rc = db_init(cache_path);


if( rc != SQLITE_OK )
		printf("Iniziali fail\n");
	

mdb = db_open(buf, DB_OPEN_CASE_SENSITIVE_LIKE);
if(mdb == NULL) {
		printf("Open Fail\n");
		return 1;
	}

sqlite3_stmt *stmt;

snprintf(sql, sizeof(sql), "SELECT name, developer, releasedate, genre, players FROM game");
rc = sqlite3_prepare_v2(mdb, sql, -1, &stmt, 0);
	
	
if(rc != SQLITE_OK) {
    printf("Query Error\n");
}

	
rc = sqlite3_step(stmt);
int x=0;
while (rc == SQLITE_ROW && x<3) { //max 3 row for this example
	snprintf(textToDraw[x], sizeof(textToDraw[x]),"Game: %s - Developer: %s - Release Year: %s - Genre: %s - Players: %s",
			(const char *)sqlite3_column_text(stmt, 0),(const char *)sqlite3_column_text(stmt, 1),(const char *)sqlite3_column_text(stmt, 2),
			(const char *)sqlite3_column_text(stmt, 3),(const char *)sqlite3_column_text(stmt, 4));
			x++;

rc = sqlite3_step(stmt);
}
sqlite3_finalize(stmt);
sqlite3_close(mdb);


	padInfo padinfo;
	padData paddata;
	int i;
	
	tiny3d_Init(1024*1024);

	ioPadInit(7);

	// Load texture

    LoadTexture();

	
	// Ok, everything is setup. Now for the main loop.
	while(1) {

        /* DRAWING STARTS HERE */

        // clear the screen, buffer Z and initializes environment to 2D

        tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

        // Enable alpha Test
        tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

        // Enable alpha blending.
        tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
            TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
            TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
      

		// Check the pads.
		ioPadGetInfo(&padinfo);

		for(i = 0; i < MAX_PADS; i++){

			if(padinfo.status[i]){
				ioPadGetData(i, &paddata);
				
				if(paddata.BTN_CROSS){
					return 0;
				}
			}
			
		}

        drawScene(); // Draw

        /* DRAWING FINISH HERE */

        tiny3d_Flip();
		
	
	}

	return 0;
}

