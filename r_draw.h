#ifndef __R_DRAW__
#define __R_DRAW__


#ifdef __GNUG__
#pragma interface
#endif


extern int		dc_x;
extern int		dc_yl;
extern int		dc_yh;
extern fixed_t		dc_iscale;
extern fixed_t		dc_texturemid;

// first pixel in a column
extern byte*		dc_source;		


// The span blitting interface.
// Hook in assembler or system specific BLT
//  here.
void 	R_DrawColumn (void);
void 	R_DrawColumnLow (void);

void R_DrawColumnHoley();

// The Spectre/Invisibility effect.
void 	R_DrawFuzzColumn (void);
void 	R_DrawFuzzColumnLow (void);

// Draw with color translation tables,
//  for player sprite rendering,
//  Green/Red/Blue/Indigo shirts.
void	R_DrawTranslatedColumn (void);
void	R_DrawTranslatedColumnLow (void);

void
R_VideoErase
( unsigned	ofs,
  int		count );

extern int		ds_y;
extern int		ds_x1;
extern int		ds_x2;

extern fixed_t		ds_xfrac;
extern fixed_t		ds_yfrac;
extern fixed_t		ds_xstep;
extern fixed_t		ds_ystep;

// start of a 64*64 tile image
extern byte*		ds_source;
extern int		dc_src_height;
extern int		dc_src_width;

extern uint8_t *dc_colormap;
extern uint8_t *dc_translation;
extern uint8_t *dc_lightcolor;

extern int dc_holestep;

// Span blitting for rows, floor/ceiling.
// No Sepctre effect needed.
void 	R_DrawSpan (void);
void R_DrawSpanShadow (void);

// Low resolution mode, 160x200?
void 	R_DrawSpanLow (void);


void
R_InitBuffer
( int		width,
  int		height );


// Initialize color translation tables,
//  for player rendering etc.
void	R_InitTranslationTables (void);

// [kg] pick renderer
void R_SetupRenderFunc(int style, void *table, void *translation);
void R_SetupRenderFuncWall(int style, void *table, void *translation);
void R_SetupRenderFuncSpan(int style, void *table, void *translation, boolean masked);

// Rendering function.
void R_FillBackScreen (void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder (void);



#endif

