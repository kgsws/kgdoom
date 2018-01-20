#ifndef __D_THINK__
#define __D_THINK__


#ifdef __GNUG__
#pragma interface
#endif

//
// [kg] notes for Lua support
//
// Since all LightUserData share same metatable,
// every exported type has to have special header
// to identify type for Lua access.
// I used existing 'thinker' for this.
// Degen thinker only contains lua_type.
// Thinker or degen thinker has to be first in
// every exported type.
// This way it is possible to use any poninter as LightUserData.

//
// Experimental stuff.
// To compile this as "ANSI C with classes"
//  we will need to handle the various
//  action functions cleanly.
//
typedef  void (*actionf_v)();
typedef  void (*actionf_p1)( void* );
typedef  void (*actionf_p2)( void*, void* );
// [kg] item pickup
typedef  int (*actionf_p2i)( void*, void* );

typedef union
{
  actionf_p1	acp1;
  actionf_v	acv;
  actionf_p2	acp2;
  actionf_p2i   acp2i;
} actionf_t;


// [kg] types for lua
// keep 'thinker_names' up to date in kg_lua.c
typedef enum
{
	TT_INVALID, // should not be used
	TT_MOBJ,
	TT_MOBJINFO, // degen
	TT_PLAYER, // degen
	TT_SECTOR, // degen?
	TT_LINE, // degen
	TT_GENPLANE,
	TT_SECCALL,
} luathinker_t;


// Historically, "think_t" is yet another
//  function pointer to a routine to handle
//  an actor.
typedef actionf_t  think_t;


// Doubly linked list of actors.
typedef struct thinker_s
{
	luathinker_t lua_type;
	struct thinker_s*	prev;
	struct thinker_s*	next;
	think_t			function;
} thinker_t;

// [kg] simplified thinker for non-tickers
typedef struct
{
	luathinker_t lua_type;
} degenthinker_t;

#endif

