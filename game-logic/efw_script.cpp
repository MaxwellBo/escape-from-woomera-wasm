#include "efw_script.h"

#include <ctype.h>
#include <stdio.h>
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
		p = (char *)EfwSkipWs( p );
		if( !*p )
			break;
		start = p;
		while( *p && !isspace( (unsigned char)*p ) )
		{
			if( *p == '(' )
			{
				p++;
				while( *p && *p != ')' )
					p++;
				if( *p == ')' )
					p++;
				break;
			}
			p++;
		}
		EfwCopy( reply->actions[reply->actionCount], EFW_ACTION_LEN, start, (int)( p - start ) );
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
	const char *p;
	const char *end;
	EfwQuestion *q;
	EfwReply *r;
	int inUnwanted;

	EfwScript_Clear( script );
	if( name )
		EfwCopy( script->name, EFW_TOPIC_LEN, name, -1 );

	if( !src )
		return 0;
	if( len < 0 )
		len = (int)strlen( src );
	end = src + len;
	inUnwanted = 0;
	q = NULL;

	p = src;
	while( p < end )
	{
		lineLen = 0;
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

		(void)inUnwanted;
	}

	(void)i;
	return script->questionCount;
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
