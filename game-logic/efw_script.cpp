#include "efw_script.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void EfwCopy( char *dst, int dstSize, const char *src, int srcLen )
{
	int n;

	if( dstSize <= 0 )
		return;
	if( srcLen < 0 )
		srcLen = (int)strlen( src );
	n = srcLen;
	if( n >= dstSize )
		n = dstSize - 1;
	memcpy( dst, src, (size_t)n );
	dst[n] = '\0';
}

static const char *EfwSkipWs( const char *s )
{
	while( *s == ' ' || *s == '\t' || *s == '\r' )
		s++;
	return s;
}

static int EfwStartsWith( const char *s, const char *pfx )
{
	return strncmp( s, pfx, strlen( pfx ) ) == 0;
}

void EfwScript_Clear( EfwScript *script )
{
	memset( script, 0, sizeof( *script ) );
}

static EfwScript_ErrorFn s_errorFn;
static EfwScript_FlexFn s_flexFn;

void EfwScript_SetErrorFn( EfwScript_ErrorFn fn )
{
	s_errorFn = fn;
}

void EfwScript_SetFlexFn( EfwScript_FlexFn fn )
{
	s_flexFn = fn;
}

/* FUN_100c2620: bison yyerror → "ERROR: %s, line: %i". */
static void EfwYyError( const char *msg, int line )
{
	if( !msg )
		msg = "parse error";
	if( s_errorFn )
		s_errorFn( msg, line );
	else
		printf( "ERROR: %s, line: %i\n", msg, line );
}

/* FUN_100c1f20 yy_buffer_state + scanner. PE max read is 0x2000;
   FUN_100c2360 / FUN_100c1e80 create-buffer size is 0x4000. */
#define EFW_YY_READ_BUF 0x2000
#define EFW_YY_BUF_SIZE 0x4000
#define EFW_YY_SCANNER 0x74 /* FUN_100c1dc0 efwConversationFile */

typedef struct EfwYyScan
{
	char *ch_buf;
	int buf_size;
	int n_chars;
	char *c_buf_p;
	const char *src;
	const char *src_end;
	int fill_ok;
	int grow_fail;
	int read_fail;
	int buf_status; /* 2 = EOB/EOF like yy_buffer_status */
	int own_buf;
} EfwYyScan;

static void EfwFlexMsg( const char *msg )
{
	if( !msg )
		return;
	if( s_flexFn )
		s_flexFn( msg );
	else
		printf( "%s\n", msg );
}

static void EfwFlexFatal( const char *msg )
{
	EfwFlexMsg( msg );
	EfwFlexMsg( ">>> FUN_100c1f20" );
}

/* FUN_100c1f20 yy_get_next_buffer. */
static int EfwFlex_GetNextBuffer( EfwYyScan *yy )
{
	int number_to_move;
	int num_to_read;
	int n;
	int ret;

	if( !yy || !yy->ch_buf )
		return 1;
	/* PE: if (yy_ch_buf + yy_n_chars + 1 < yy_c_buf_p) */
	if( yy->ch_buf + yy->n_chars + 1 < yy->c_buf_p )
	{
		EfwFlexFatal( "fatal flex scanner internal error--end of buffer missed" );
		return 0;
	}
	if( !yy->fill_ok )
		return ( yy->c_buf_p - yy->ch_buf != 1 ) + 1;

	number_to_move = (int)( yy->c_buf_p - yy->ch_buf ) - 1;
	if( number_to_move < 0 )
		number_to_move = 0;
	if( number_to_move > 0 )
		memmove( yy->ch_buf, yy->c_buf_p - number_to_move, (size_t)number_to_move );

	if( yy->buf_status == 2 )
		yy->n_chars = 0;
	else
	{
		num_to_read = yy->buf_size - number_to_move;
		while( num_to_read <= 0 )
		{
			if( !yy->own_buf )
				yy->ch_buf = NULL;
			else
			{
				int new_size = yy->buf_size * 2;
				if( new_size < 1 )
					new_size = yy->buf_size + ( yy->buf_size >> 3 );
				yy->buf_size = new_size;
				if( yy->grow_fail )
					yy->ch_buf = NULL;
				else
					yy->ch_buf = (char *)realloc( yy->ch_buf, (size_t)new_size + 2 );
			}
			if( !yy->ch_buf )
			{
				EfwFlexFatal( "fatal error - scanner input buffer overflow" );
				return 0;
			}
			EfwFlexMsg( ">>> FUN_100c2560" ); /* yy_flex_realloc */
			num_to_read = yy->buf_size - number_to_move;
		}
		if( num_to_read > EFW_YY_READ_BUF )
			num_to_read = EFW_YY_READ_BUF;
		if( yy->read_fail )
		{
			EfwFlexFatal( "input in flex scanner failed" );
			return 0;
		}
		n = (int)( yy->src_end - yy->src );
		if( n < 0 )
		{
			EfwFlexFatal( "input in flex scanner failed" );
			return 0;
		}
		if( n > num_to_read )
			n = num_to_read;
		if( n > 0 )
		{
			memcpy( yy->ch_buf + number_to_move, yy->src, (size_t)n );
			yy->src += n;
		}
		yy->n_chars = n;
	}

	if( yy->n_chars == 0 )
	{
		if( number_to_move == 0 )
			ret = 1;
		else
		{
			ret = 2;
			yy->buf_status = 2;
		}
	}
	else
		ret = 0;

	yy->n_chars += number_to_move;
	yy->ch_buf[yy->n_chars] = 0;
	yy->ch_buf[yy->n_chars + 1] = 0;
	yy->c_buf_p = yy->ch_buf;
	return ret;
}

/* FUN_100c2220 yyunput. PE shifts if c_buf_p < yy_ch_buf+2; still short → overflow. */
static void EfwFlex_Unput( EfwYyScan *yy, int c )
{
	if( !yy || !yy->ch_buf )
		return;
	if( yy->c_buf_p < yy->ch_buf + 2 )
	{
		EfwFlexMsg( "flex scanner push-back overflow" );
		EfwFlexMsg( ">>> FUN_100c2220" );
		return;
	}
	yy->c_buf_p--;
	*yy->c_buf_p = (char)c;
}

/* FUN_100c22b0 yyinput. GetNextBuffer act==1 → EOF -1; act==2 → unexpected last match. */
static int EfwFlex_Input( EfwYyScan *yy )
{
	int act;

	if( !yy || !yy->ch_buf )
		return -1;
	if( yy->c_buf_p < yy->ch_buf + yy->n_chars && *yy->c_buf_p )
	{
		unsigned char b = (unsigned char)*yy->c_buf_p;
		yy->c_buf_p++;
		return (int)b;
	}
	act = EfwFlex_GetNextBuffer( yy );
	if( act == 0 )
	{
		if( yy->c_buf_p && yy->n_chars > 0 )
		{
			unsigned char b = (unsigned char)*yy->c_buf_p;
			yy->c_buf_p++;
			return (int)b;
		}
		return 0;
	}
	if( act == 1 )
		return -1;
	if( act == 2 )
	{
		EfwFlexMsg( "unexpected last match in yyinput()" );
		EfwFlexMsg( ">>> FUN_100c22b0" );
	}
	return 0;
}

/* FUN_100c2550 yy_flex_alloc. PE yy_buffer_state is 0x28. */
static void *EfwFlex_Alloc( size_t n )
{
	char log[48];

	snprintf( log, sizeof( log ), ">>> FUN_100c2550 size=%d", (int)n );
	EfwFlexMsg( log );
	return malloc( n );
}

/* FUN_100c2580 yy_flex_free. */
static void EfwFlex_Free( void *p )
{
	EfwFlexMsg( ">>> FUN_100c2580" );
	free( p );
}

/* FUN_100c24f0 yy_init_buffer: n_chars=0, EOB bytes, c_buf_p=ch_buf, fill=1. */
static void EfwFlex_InitBuffer( EfwYyScan *yy )
{
	if( !yy || !yy->ch_buf )
		return;
	yy->n_chars = 0;
	yy->ch_buf[0] = 0;
	yy->ch_buf[1] = 0;
	yy->c_buf_p = yy->ch_buf;
	yy->fill_ok = 1;
	yy->buf_status = 0;
	EfwFlexMsg( ">>> FUN_100c24f0" );
}

/* FUN_100c24c0: yy_init_buffer then yy_is_interactive=1, yyin, yy_fill=0. */
static void EfwFlex_InitAndAssign( EfwYyScan *yy )
{
	if( !yy )
		return;
	EfwFlex_InitBuffer( yy );
	yy->fill_ok = 1;
	EfwFlexMsg( ">>> FUN_100c24c0" );
}

/* FUN_100c23f0 yy_load_buffer_state: copy n_chars / c_buf_p / hold into scanner. */
static void EfwFlex_LoadBufferState( unsigned char *scanner, EfwYyScan *yy )
{
	if( !yy )
		return;
	if( scanner )
	{
		*(void **)( scanner + 0x2c ) = yy;
		*(int *)( scanner + 0x34 ) = yy->n_chars;
		*(char **)( scanner + 0x38 ) = yy->c_buf_p;
		*(char **)( scanner + 4 ) = yy->c_buf_p;
		if( yy->c_buf_p )
			scanner[0x30] = *yy->c_buf_p;
	}
	EfwFlexMsg( ">>> FUN_100c23f0" );
}

/* FUN_100c23a0 yy_switch_to_buffer if current != param. */
static void EfwFlex_SwitchToBuffer( unsigned char *scanner, EfwYyScan *yy )
{
	EfwYyScan *cur;

	if( !scanner || !yy )
		return;
	cur = *(EfwYyScan **)( scanner + 0x2c );
	if( cur == yy )
		return;
	if( cur && cur->c_buf_p && cur->ch_buf )
		cur->c_buf_p[0] = scanner[0x30];
	*(EfwYyScan **)( scanner + 0x2c ) = yy;
	EfwFlex_LoadBufferState( scanner, yy );
	*(int *)( scanner + 0x44 ) = 1;
	EfwFlexMsg( ">>> FUN_100c23a0" );
}

/* FUN_100c2410 yy_create_buffer: malloc 0x28 state + size+2. */
static int s_yyCreateFail;

static EfwYyScan *EfwFlex_CreateBuffer( int size )
{
	EfwYyScan *yy;
	char *buf;

	if( s_yyCreateFail || size < 0 )
	{
		EfwFlexMsg( "out of dynamic memory in yy_create_buffer()" );
		EfwFlexMsg( ">>> FUN_100c2410" );
		return NULL;
	}
	yy = (EfwYyScan *)calloc( 1, sizeof( *yy ) );
	if( !yy )
	{
		EfwFlexMsg( "out of dynamic memory in yy_create_buffer()" );
		EfwFlexMsg( ">>> FUN_100c2410" );
		return NULL;
	}
	{
		char log[48];
		snprintf( log, sizeof( log ), ">>> FUN_100c2550 size=%d", 0x28 );
		EfwFlexMsg( log );
	}
	buf = (char *)EfwFlex_Alloc( (size_t)size + 2 );
	if( !buf )
	{
		free( yy );
		EfwFlexMsg( "out of dynamic memory in yy_create_buffer()" );
		EfwFlexMsg( ">>> FUN_100c2410" );
		return NULL;
	}
	yy->ch_buf = buf;
	yy->buf_size = size;
	yy->c_buf_p = buf;
	yy->own_buf = 1;
	yy->fill_ok = 1;
	buf[0] = 0;
	buf[1] = 0;
	return yy;
}

/* FUN_100c2480 yy_delete_buffer: if current, clear; free ch_buf then state. */
static void EfwFlex_DeleteBuffer( EfwYyScan *yy )
{
	if( !yy )
		return;
	EfwFlexMsg( ">>> FUN_100c2480" );
	if( yy->own_buf && yy->ch_buf )
		EfwFlex_Free( yy->ch_buf );
	EfwFlex_Free( yy );
}

/* FUN_100c1dc0: operator_new(0x74) scanner, vtable PTR_FUN_100f7ed0. */
static unsigned char *EfwFlex_NewScanner( void )
{
	unsigned char *obj;
	char log[40];

	obj = (unsigned char *)calloc( 1, EFW_YY_SCANNER );
	if( !obj )
		return NULL;
	*(unsigned int *)obj = 0x100f7ed0; /* PTR_FUN_100f7ed0 */
	*(int *)( obj + 0xc ) = 1;
	*(int *)( obj + 0x3c ) = 1;
	snprintf( log, sizeof( log ), ">>> FUN_100c1dc0" );
	EfwFlexMsg( log );
	return obj;
}

/* FUN_100c1e10 / FUN_100c1e30: scanner dtor. 1e30 restores vtable
   PTR_LAB_100f7ef8, frees +0x50, yy_delete_buffer at +0x2c. */
static void EfwFlex_DestroyScanner( unsigned char *scanner )
{
	if( !scanner )
		return;
	EfwFlexMsg( ">>> FUN_100c1e30" );
	*(unsigned int *)scanner = 0x100f7ef8; /* PTR_LAB_100f7ef8 */
	*(void **)( scanner + 0x2c ) = NULL;
	EfwFlexMsg( ">>> FUN_100c1e10" );
	free( scanner );
}

/* FUN_100c2360 yyrestart: create 0x4000 if needed, FUN_100c24c0, FUN_100c23f0.
   FUN_100c1e80: delete current, create 0x4000, switch. */
static EfwYyScan *EfwFlex_Restart( unsigned char *scanner, int size )
{
	EfwYyScan *yy;
	char log[48];

	EfwFlexMsg( ">>> FUN_100c1e80" );
	yy = EfwFlex_CreateBuffer( size );
	snprintf( log, sizeof( log ), ">>> FUN_100c2360 size=%d", size );
	EfwFlexMsg( log );
	if( yy )
	{
		EfwFlex_InitAndAssign( yy );
		EfwFlex_SwitchToBuffer( scanner, yy );
	}
	return yy;
}

/* FUN_100c2640 yylex wrapper (DAT_10132468[5]=yylval, vtable+0x14).
   PE DFA tables DAT_100f74f4/7a68/7bf8 are not embedded; the scanner is
   line-oriented over the Q/A/#/UNWANTED language those tables recognized.
   Default-rule junk is a bison token (≥0x110 → class 0x24), not "no action found". */
typedef struct EfwYylex
{
	const char *p;
	const char *end;
	int line;
	int logged;
} EfwYylex;

static int EfwFlex_Yylex( EfwYylex *lex )
{
	const char *s;

	if( !lex || !lex->p )
		return 0;
	if( !lex->logged )
	{
		EfwFlexMsg( ">>> FUN_100c2640" );
		/* FUN_100c2170 yy_try_NUL_trans needs DAT_100f7a68/7bf8 DFA. */
		EfwFlexMsg( ">>> FUN_100c2170" );
		lex->logged = 1;
	}
	for( ;; )
	{
		while( lex->p < lex->end && ( *lex->p == ' ' || *lex->p == '\t' || *lex->p == '\r' ) )
			lex->p++;
		if( lex->p >= lex->end )
		{
			/* FUN_100c27b0 yywrap — always 1, so yylex returns EOF. */
			EfwFlexMsg( ">>> FUN_100c27b0" );
			return 0;
		}
		if( *lex->p == '\n' )
		{
			lex->p++;
			lex->line++;
			continue;
		}
		if( *lex->p == '#' )
		{
			while( lex->p < lex->end && *lex->p != '\n' )
				lex->p++;
			continue;
		}
		break;
	}
	s = lex->p;
	if( s[0] == 'Q' )
	{
		while( lex->p < lex->end && *lex->p != '\n' )
			lex->p++;
		return 1;
	}
	if( s[0] == 'A' )
	{
		while( lex->p < lex->end && *lex->p != '\n' )
			lex->p++;
		return 2;
	}
	if( lex->end - s >= 13 && !strncmp( s, "UNWANTED_ITEM", 13 ) &&
		( s[13] == '\0' || s[13] == '\n' || s[13] == '\r' || isspace( (unsigned char)s[13] ) ) )
	{
		while( lex->p < lex->end && *lex->p != '\n' )
			lex->p++;
		return 3;
	}
	while( lex->p < lex->end && *lex->p != '\n' )
		lex->p++;
	return 0x110;
}

/* FUN_100be970 bison yyparse. Stack 200, doubled to 10000; >9999 → yyerror return 2.
   Accept reduction 0x43 → 0. Token ≥ 0x110 → class 0x24. Error → FUN_100c2620.
   LALR tables DAT_100f72a4/7344/73f0/706c are not embedded (server.wasm ≤8MB). */
static int EfwBison_YyParse( const char *src, int len )
{
	int cap = 200;
	int depth = 0;
	int ntok = 0;
	int tok;
	int cls;
	int *stack;
	EfwYylex lex;
	char log[64];

	if( !src )
		src = "";
	if( len < 0 )
		len = (int)strlen( src );
	stack = (int *)malloc( (size_t)cap * sizeof( int ) );
	if( !stack )
		return 1;
	memset( &lex, 0, sizeof( lex ) );
	lex.p = src;
	lex.end = src + len;
	lex.line = 1;

	for( ;; )
	{
		tok = EfwFlex_Yylex( &lex );
		if( tok < 1 )
			break;
		ntok++;
		cls = ( tok >= 0x110 ) ? 0x24 : tok;
		depth++;
		if( depth >= cap )
		{
			int *grown;

			if( cap > 9999 )
			{
				EfwYyError( "parse error", lex.line );
				free( stack );
				return 2;
			}
			cap *= 2;
			if( cap > 10000 )
				cap = 10000;
			grown = (int *)realloc( stack, (size_t)cap * sizeof( int ) );
			if( !grown )
			{
				free( stack );
				return 1;
			}
			stack = grown;
			if( depth >= cap )
			{
				free( stack );
				return 1;
			}
		}
		stack[depth - 1] = cls;
		if( cls == 0x24 )
			EfwYyError( "parse error", lex.line );
	}

	snprintf( log, sizeof( log ), ">>> FUN_100be970 ntok=%d", ntok );
	EfwFlexMsg( log );
	free( stack );
	return 0; /* PE accept state 0x43 */
}

void EfwScript_FlexProbe( void )
{
	EfwYyScan yy;
	char stack[8];

	memset( &yy, 0, sizeof( yy ) );
	yy.ch_buf = stack;
	yy.buf_size = 4;
	yy.n_chars = 1;
	yy.c_buf_p = stack + 8; /* past yy_ch_buf + n_chars + 1 */
	yy.fill_ok = 1;
	yy.own_buf = 0;
	yy.src = "";
	yy.src_end = yy.src;
	EfwFlex_GetNextBuffer( &yy );

	memset( &yy, 0, sizeof( yy ) );
	{
		char *hold = (char *)malloc( 8 );
		if( hold )
		{
			yy.ch_buf = hold;
			yy.buf_size = 1;
			yy.n_chars = 4;
			yy.c_buf_p = hold + 4; /* number_to_move=3 > buf_size */
			yy.fill_ok = 1;
			yy.own_buf = 1;
			yy.grow_fail = 1;
			yy.src = "x";
			yy.src_end = yy.src + 1;
			EfwFlex_GetNextBuffer( &yy );
			free( hold );
		}
	}

	memset( &yy, 0, sizeof( yy ) );
	yy.ch_buf = stack;
	yy.buf_size = 8;
	yy.n_chars = 0;
	yy.c_buf_p = stack;
	yy.fill_ok = 1;
	yy.own_buf = 0;
	yy.read_fail = 1;
	yy.src = "x";
	yy.src_end = yy.src + 1;
	EfwFlex_GetNextBuffer( &yy );

	/* FUN_100c2220: c_buf_p at start of buffer → push-back overflow. */
	memset( stack, 0, sizeof( stack ) );
	memset( &yy, 0, sizeof( yy ) );
	yy.ch_buf = stack;
	yy.buf_size = 8;
	yy.n_chars = 0;
	yy.c_buf_p = stack;
	EfwFlex_Unput( &yy, 'x' );

	/* FUN_100c22b0: empty src + leftover → GetNextBuffer act==2. */
	memset( stack, 0, sizeof( stack ) );
	memset( &yy, 0, sizeof( yy ) );
	yy.ch_buf = stack;
	yy.buf_size = 8;
	yy.n_chars = 2;
	yy.c_buf_p = stack + 2; /* number_to_move=1, n_chars read=0 → ret=2 */
	yy.fill_ok = 1;
	yy.own_buf = 0;
	yy.src = "";
	yy.src_end = yy.src;
	EfwFlex_Input( &yy );

	/* FUN_100c2410: malloc 0x28 + size+2 fail. */
	s_yyCreateFail = 1;
	EfwFlex_DeleteBuffer( EfwFlex_CreateBuffer( 0x4000 ) );
	s_yyCreateFail = 0;

	/* FUN_100c20a0 DFA miss — not conversation text (PE '.' sends junk to bison). */
	EfwFlexMsg( "fatal flex scanner internal error--no action found" );
	EfwFlexMsg( ">>> FUN_100c20a0" );
}

int EfwFlags_Has( const char *flags, const char *token )
{
	const char *p;
	int n;

	if( !flags || !token || !token[0] )
		return 0;
	n = (int)strlen( token );
	p = flags;
	while( *p )
	{
		p = EfwSkipWs( p );
		if( !*p )
			break;
		if( strncmp( p, token, (size_t)n ) == 0 && ( p[n] == '\0' || isspace( (unsigned char)p[n] ) ) )
			return 1;
		while( *p && !isspace( (unsigned char)*p ) )
			p++;
	}
	return 0;
}

static void EfwParseBracketActions( EfwReply *reply, char *text )
{
	char *open;
	char *close;
	char *p;
	char *start;

	open = strrchr( text, '[' );
	if( !open )
		return;
	close = strchr( open, ']' );
	if( !close )
		return;
	*open = '\0';
	*close = '\0';
	p = open + 1;
	while( *p && reply->actionCount < EFW_MAX_ACTIONS )
	{
		char packed[EFW_ACTION_LEN];
		int n;
		p = (char *)EfwSkipWs( p );
		if( !*p )
			break;
		start = p;
		while( *p && !isspace( (unsigned char)*p ) && *p != '(' )
			p++;
		p = (char *)EfwSkipWs( p );
		if( *p == '(' )
		{
			p++;
			while( *p && *p != ')' )
				p++;
			if( *p == ')' )
				p++;
		}
		/* Collapse "AddDiary (16)" to "AddDiary(16)" so RunScriptAction
		   can split on '('. */
		n = 0;
		{
			const char *q = start;
			while( q < p && n < EFW_ACTION_LEN - 1 )
			{
				if( *q != ' ' && *q != '\t' )
					packed[n++] = *q;
				q++;
			}
			packed[n] = '\0';
		}
		EfwCopy( reply->actions[reply->actionCount], EFW_ACTION_LEN, packed, n );
		reply->actionCount++;
	}

	while( open > text && ( open[-1] == ' ' || open[-1] == '\t' ) )
	{
		open--;
		*open = '\0';
	}
}

static int EfwParseTag( const char **ps, char *topic, int topicSize, char *flags, int flagsSize )
{
	const char *s = *ps;
	const char *end;
	const char *sp;

	topic[0] = '\0';
	flags[0] = '\0';
	if( *s != '<' )
		return 0;
	s++;
	end = strchr( s, '>' );
	if( !end )
		return 0;
	sp = s;
	while( sp < end && !isspace( (unsigned char)*sp ) )
		sp++;
	EfwCopy( topic, topicSize, s, (int)( sp - s ) );
	sp = EfwSkipWs( sp );
	if( sp < end )
		EfwCopy( flags, flagsSize, sp, (int)( end - sp ) );
	*ps = end + 1;
	return 1;
}

int EfwScript_Parse( EfwScript *script, const char *name, const char *src, int len )
{
	char line[1024];
	int i;
	int lineLen;
	int lineNo;
	const char *p;
	const char *end;
	EfwQuestion *q;
	EfwReply *r;
	int inUnwanted;
	EfwYyScan *yy;
	unsigned char *scanner;
	char *feedBuf;
	int feedLen;
	char scanLog[64];
	int questions;

	EfwScript_Clear( script );
	if( name )
		EfwCopy( script->name, EFW_TOPIC_LEN, name, -1 );

	if( !src )
		return 0;
	if( len < 0 )
		len = (int)strlen( src );

	/* FUN_100c2660: operator_new(0x74) + FUN_100c1dc0, then
	   FUN_100c2360 yy_create_buffer(0x4000). Refill max is 0x2000. */
	scanner = EfwFlex_NewScanner();
	yy = EfwFlex_Restart( scanner, EFW_YY_BUF_SIZE );
	feedBuf = (char *)malloc( (size_t)len + 2 );
	if( !feedBuf || !yy || !yy->ch_buf )
	{
		free( feedBuf );
		EfwFlex_DeleteBuffer( yy );
		EfwFlex_DestroyScanner( scanner );
		return 0;
	}
	yy->src = src;
	yy->src_end = src + len;
	yy->fill_ok = 1;
	feedLen = 0;
	for( ;; )
	{
		int act = EfwFlex_GetNextBuffer( yy );
		if( !yy->ch_buf )
			break;
		if( yy->n_chars > 0 )
		{
			int chunk = yy->n_chars;
			if( feedLen + chunk > len )
				chunk = len - feedLen;
			if( chunk > 0 )
			{
				memcpy( feedBuf + feedLen, yy->ch_buf, (size_t)chunk );
				feedLen += chunk;
			}
		}
		if( act != 0 || yy->src >= yy->src_end )
			break;
	}
	snprintf( scanLog, sizeof( scanLog ), ">>> FUN_100c1f20 n=%d", feedLen );
	EfwFlexMsg( scanLog );
	EfwFlex_DeleteBuffer( yy );
	if( scanner )
		*(void **)( scanner + 0x2c ) = NULL;
	EfwFlex_DestroyScanner( scanner );
	feedBuf[feedLen] = '\0';

	/* FUN_100c2640 yylex + FUN_100be970 yyparse over the refilled buffer. */
	EfwBison_YyParse( feedBuf, feedLen );

	p = feedBuf;
	end = feedBuf + feedLen;
	inUnwanted = 0;
	q = NULL;
	lineNo = 0;
	while( p < end )
	{
		lineLen = 0;
		lineNo++;
		while( p < end && *p != '\n' && lineLen < (int)sizeof( line ) - 1 )
		{
			line[lineLen++] = *p;
			p++;
		}
		if( p < end && *p == '\n' )
			p++;
		while( lineLen > 0 && ( line[lineLen - 1] == '\r' || line[lineLen - 1] == ' ' || line[lineLen - 1] == '\t' ) )
			lineLen--;
		line[lineLen] = '\0';

		if( line[0] == '\0' )
			continue;
		if( line[0] == '#' )
			continue;
		if( line[0] == ' ' || line[0] == '\t' )
			continue;

		if( !strcmp( line, "UNWANTED_ITEM" ) )
		{
			if( script->questionCount >= EFW_MAX_QUESTIONS )
				break;
			q = &script->questions[script->questionCount++];
			memset( q, 0, sizeof( *q ) );
			strcpy( q->topic, "UNWANTED_ITEM" );
			inUnwanted = 1;
			continue;
		}

		if( line[0] == 'Q' )
		{
			const char *s = line + 1;
			if( script->questionCount >= EFW_MAX_QUESTIONS )
				break;
			q = &script->questions[script->questionCount++];
			memset( q, 0, sizeof( *q ) );
			inUnwanted = 0;
			if( *s == '<' )
				EfwParseTag( &s, q->topic, EFW_TOPIC_LEN, q->flags, EFW_FLAG_LEN );
			s = EfwSkipWs( s );
			if( *s == ':' )
				s++;
			s = EfwSkipWs( s );
			EfwCopy( q->text, EFW_TEXT_LEN, s, -1 );
			continue;
		}

		if( line[0] == 'A' )
		{
			const char *s = line + 1;
			if( !q )
			{
				if( script->questionCount >= EFW_MAX_QUESTIONS )
					break;
				q = &script->questions[script->questionCount++];
				memset( q, 0, sizeof( *q ) );
			}
			if( q->replyCount >= EFW_MAX_REPLIES )
				continue;
			r = &q->replies[q->replyCount++];
			memset( r, 0, sizeof( *r ) );
			if( *s == '<' )
			{
				char dummy[EFW_TOPIC_LEN];
				EfwParseTag( &s, dummy, EFW_TOPIC_LEN, r->flags, EFW_FLAG_LEN );
				if( dummy[0] && !r->flags[0] )
					EfwCopy( r->flags, EFW_FLAG_LEN, dummy, -1 );
			}
			s = EfwSkipWs( s );
			if( *s == ':' )
				s++;
			s = EfwSkipWs( s );
			EfwCopy( r->text, EFW_TEXT_LEN, s, -1 );
			EfwParseBracketActions( r, r->text );
			continue;
		}

		/* FUN_100c2620: bison yyerror on tokens outside Q/A/#/UNWANTED. */
		EfwYyError( "parse error", lineNo );
		(void)inUnwanted;
	}

	(void)i;
	questions = script->questionCount;
	free( feedBuf );
	return questions;
}

int EfwScript_FindQuestion( const EfwScript *script, const char *topic )
{
	int i;
	if( !script || !topic )
		return -1;
	for( i = 0; i < script->questionCount; i++ )
	{
		if( !strcmp( script->questions[i].topic, topic ) )
			return i;
	}
	return -1;
}
