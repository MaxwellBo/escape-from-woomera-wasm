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

/* FUN_100c1f20 yy_buffer_state + scanner. PE max read is 0x2000. */
#define EFW_YY_READ_BUF 0x2000

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
	EfwYyScan yy;
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

	/* FUN_100c1f20: copy the conversation through a flex input buffer
	   (max 0x2000 per refill) before the Q/A line parser. */
	memset( &yy, 0, sizeof( yy ) );
	feedBuf = (char *)malloc( (size_t)len + 2 );
	yy.ch_buf = (char *)malloc( EFW_YY_READ_BUF + 2 );
	if( !feedBuf || !yy.ch_buf )
	{
		free( feedBuf );
		free( yy.ch_buf );
		return 0;
	}
	yy.buf_size = EFW_YY_READ_BUF;
	yy.n_chars = 0;
	yy.c_buf_p = yy.ch_buf;
	yy.src = src;
	yy.src_end = src + len;
	yy.fill_ok = 1;
	yy.own_buf = 1;
	feedLen = 0;
	for( ;; )
	{
		int act = EfwFlex_GetNextBuffer( &yy );
		if( !yy.ch_buf )
			break;
		if( yy.n_chars > 0 )
		{
			int chunk = yy.n_chars;
			if( feedLen + chunk > len )
				chunk = len - feedLen;
			if( chunk > 0 )
			{
				memcpy( feedBuf + feedLen, yy.ch_buf, (size_t)chunk );
				feedLen += chunk;
			}
		}
		if( act != 0 || yy.src >= yy.src_end )
			break;
	}
	snprintf( scanLog, sizeof( scanLog ), ">>> FUN_100c1f20 n=%d", feedLen );
	EfwFlexMsg( scanLog );
	free( yy.ch_buf );
	feedBuf[feedLen] = '\0';

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
