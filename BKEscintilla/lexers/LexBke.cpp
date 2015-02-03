﻿// Scintilla source code edit control
/** @file LexKix.cxx
 ** Lexer for KIX-Scripts.
 **/
// Copyright 2004 by Manfred Becker <manfred@becker-trdf.de>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <wctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

#include <QString>

#define SCLEX_BKE 108
#define SCE_BKE_DEFAULT         0
#define SCE_BKE_COMMAND         1
#define SCE_BKE_ATTRIBUTE       2
#define SCE_BKE_STRING          3
#define SCE_BKE_NUMBER          4
#define SCE_BKE_LABEL           5
#define SCE_BKE_ANNOTATE        6
#define SCE_BKE_OPERATORS       7
#define SCE_BKE_TEXT            8
#define SCE_BKE_VARIABLE        9
#define SCE_BKE_UNTYPEA         10
#define SCE_BKE_PARSER         11
//转义字符
#define SCE_BKE_TRANS         12
#define SCE_BKE_ERROR           13

static const char *BKE_PARSER_KEY = " for foreach in extends do while function propset propget int string number typeof var delete class if else continue break return this super with true false void global" ;
static const char *BKE_SEPARATEOR = " ~!@#$%^&*()-+*/|{}[]:;/=.,?><\\";

static inline bool isSpace(int i)
{
    return i <= 32;
}

static inline bool isAlpha(int i)
{
    return (i>='a' && i<='z') || (i>='A' && i<='Z');
}

static inline bool isDigit(int i)
{
    return i>='0' && i<='9';
}

static inline bool isWord(int i)
{
    return i >= 0x80;
}

static inline bool isIdentifyBegin(int i)
{
    return i=='_' || isAlpha(i) || isWord(i);
}

static inline bool isIdentifyBody(int i)
{
    return isIdentifyBegin(i) || isDigit(i);
}

bool isSeparator(unsigned char ch)
{
    char bt[2] ;
    bt[0] = (char)ch ;
    bt[1] = 0 ;
    if( ch < 32 ) return true ;
	return false;
    return (strstr(BKE_SEPARATEOR,&bt[0]) != NULL) ;
}

// Extended to accept accented characters
static inline void AnnotateLine( StyleContext *sc )//整行注释掉
{
	int s = sc->state;
    sc->SetState(SCE_BKE_ANNOTATE);   //注释状态
    while( !sc->atLineEnd && sc->More() ) sc->Forward();
    sc->SetState(s);   //恢复原状态
    return ;
}

static inline void AnnotateBlock(StyleContext *sc)//块注释
{
	int s = sc->state;
	sc->SetState(SCE_BKE_ANNOTATE);   //注释状态
	//这里是没有嵌套注释的版本，如果时机成熟会和Compiler同步修改
	sc->Forward();	//pass * after /
	while (sc->More())
	{
		if (sc->ch == '*' && sc->chNext == '/')
			break;
		sc->Forward();
	}
	sc->Forward();
	sc->Forward();
	sc->SetState(s);   //恢复原状态
	return;
}

inline bool isLineEnd(StyleContext *sc)
{
	if (sc->atLineEnd || !sc->More())
		return true;
	if (sc->ch == '/')
	{
		if (sc->chNext == '/')
		{
			AnnotateLine(sc);
			return true;
		}
		else if (sc->chNext == '*')
		{
			AnnotateBlock(sc);
			return isLineEnd(sc);
		}
		else
			return false;
	}
	return false;
}

inline bool passCmdName(StyleContext *sc, bool startwithat = true)
{
	bool r = false;
	while (!isLineEnd(sc) && !isSpace(sc->ch) && (startwithat || sc->ch != L']'))
	{
		r = true;
		sc->Forward();
	}
	return r;
}

inline bool prePassAttrName(StyleContext *sc)
{

}

inline bool passAttrName(StyleContext *sc)
{
	bool r = false;
	while (!isLineEnd(sc))
	{
		r = true;
		sc->Forward();
	}
	return r;
}

static inline void SetText( StyleContext *sc )
{
    sc->SetState(SCE_BKE_TEXT);
	while (sc->ch != '[' && !isLineEnd(sc))
	{
		sc->Forward();
	}
    sc->SetState(SCE_BKE_DEFAULT);
}

static inline void SetString( StyleContext *sc)
{
    int k = sc->state ;
    sc->SetState(SCE_BKE_STRING);
    while(sc->More() && !sc->atLineEnd)
    {
        sc->Forward();
        if(sc->ch=='\"'  )
        {
             if(sc->chNext!='\"')
                   break;
             else
                   sc->Forward();
        }
    }
    sc->Forward();
    sc->SetState(k);
}

static inline void SetString2( StyleContext *sc)
{
    int k = sc->state ;
	int pos = sc->currentPos;
    sc->SetState(SCE_BKE_STRING);
    sc->Forward();
	bool err = true;
    while(sc->More() && !sc->atLineEnd)
    {
		if (sc->ch == '\'')
		{
			err = false;
			break;
		}
		else if (sc->ch == '\\')
		{
			int num = 0;
			sc->SetState(SCE_BKE_TRANS);
			sc->Forward();
			switch (sc->ch)
			{
			case 'n':case 'r':case 't':case 'a':case 'b':case 'f':case 'v':
				sc->Forward();
				sc->SetState(SCE_BKE_STRING);
				continue;
			case 'o'://\o777
				num = 3;
				while (num-- >= 0)
				{
					if (sc->More())
						sc->Forward();
					else
						break;	//反正最后也要被涂成error
					if (sc->ch >= '8' || sc->ch < '0')
						break;
				}
				sc->SetState(SCE_BKE_STRING);
				continue;
			case 'x'://\xFF
				num = 2;
				while (num-- >= 0)
				{
					if (sc->More())
						sc->Forward();
					else
						break;	//反正最后也要被涂成error
					if (sc->ch < '0' || (sc->ch > '9' && towupper(sc->ch) < L'A') || towupper(sc->ch) > L'F')
						break;
				}
				sc->SetState(SCE_BKE_STRING);
				continue;
			default://error
				sc->ChangeState(SCE_BKE_ERROR);
				sc->Forward();
				sc->SetState(SCE_BKE_STRING);
				continue;
			}
		}
		sc->Forward();
	}
	if (err)
	{
		sc->resetState(pos, sc->currentPos, SCE_BKE_ERROR);
	}
    sc->Forward();
    sc->SetState(k);
}

static void SetValue(StyleContext *sc, bool BeginWithAt=true)
{
    QString bracket="0";
    while(!sc->atLineEnd)
    {
        if(isdigit(sc->ch))
        {
            sc->SetState(SCE_BKE_NUMBER);
            while(sc->More() && !sc->atLineEnd)
            {
                sc->Forward();
                if(!isalnum(sc->ch))
                    break;
            }
            sc->SetState(SCE_BKE_DEFAULT);
        }
        else if(sc->ch=='\"')
        {
            sc->SetState(SCE_BKE_STRING);
            while(sc->More() && !sc->atLineEnd)
            {
                sc->Forward();
                if(sc->ch=='\"'  )
                {
                	if(sc->chNext!='\"')
                   	break;
                   else
                   	sc->Forward();
                }
            }
            if(!sc->atLineEnd) sc->Forward();
            sc->SetState(SCE_BKE_DEFAULT);
        }
        else if(sc->ch == '\'')
        {
        	sc->SetState(SCE_BKE_STRING);
            sc->Forward();
            while(sc->More() && !sc->atLineEnd)
            {
                if(sc->ch=='\'')
                    break;
                else if(sc->ch=='\\')
                	sc->Forward();
                sc->Forward();
            }
            if(!sc->atLineEnd) sc->Forward();
            sc->SetState(SCE_BKE_DEFAULT);
        }
        else if(sc->ch=='_' || isalpha(sc->ch) || sc->ch>=0x80)
        {
            sc->SetState(SCE_BKE_VARIABLE);
            while(sc->More() && !sc->atLineEnd)
            {
                sc->Forward();
                if(sc->ch!='_' && !isalnum(sc->ch) && sc->ch<0x80)
                    break;
            }
            sc->SetState(SCE_BKE_DEFAULT);
        }
        else if(!isSpace(sc->ch) || bracket[bracket.size()-1]>'0')
        {
            if(isSpace(sc->ch))
            {
                if(sc->More() && !sc->atLineEnd)
                {
                    sc->Forward();
                    sc->SetState(SCE_BKE_DEFAULT);
                    continue;
                }
            }
            sc->SetState(SCE_BKE_OPERATORS);
            if(sc->ch=='(')
                bracket.push_back('1');
            else if(sc->ch=='[')
                bracket.push_back('2');
            else if(sc->ch=='{')
                bracket.push_back('3');
            else if(sc->ch==')')
            {
                if(bracket.size()>1 && bracket[bracket.size()-1]=='1')
                    bracket.resize(bracket.size()-1);
                else
                {
                    bracket="0";
                    sc->SetState(SCE_BKE_ERROR);
                }
            }
            else if(sc->ch==']')
            {
                if(bracket.size()>1 && bracket[bracket.size()-1]=='2')
                    bracket.resize(bracket.size()-1);
                else
                {
                    if(bracket=="0" && !BeginWithAt)
                    {
                        sc->SetState(SCE_BKE_DEFAULT);
                        return;
                    }
                    bracket="0";
                    sc->SetState(SCE_BKE_ERROR);
                }
            }
            else if(sc->ch=='}')
            {
                if(bracket.size()>1 && bracket[bracket.size()-1]=='3')
                    bracket.resize(bracket.size()-1);
                else
                {
                    bracket="0";
                    sc->SetState(SCE_BKE_ERROR);
                }
            }
            else if(sc->ch == '/' && sc->chNext=='/')
                break;
            if(sc->More() && !sc->atLineEnd)
            {
                sc->Forward();
                sc->SetState(SCE_BKE_DEFAULT);
            }
        }
        else
            break;
    }
}

static void SetAttr( StyleContext *sc, bool BeginWithAt=true )
{
    int orgpos=sc->currentPos;
    bool isattr=false;
    int attrpos = -1;
    bool quote=false;
    bool quote2=false;
    int bracket=0;
    while(sc->More() && !sc->atLineEnd)
    {
        if((!BeginWithAt && !bracket && sc->ch == ']')
            ||isSpace(sc->ch))
            break;
        if(sc->ch=='"')
        {
            if(sc->chNext=='"')
            {
                sc->Forward(2);
                continue;
            }
            else
                quote=!quote;
        }
        else if(sc->ch=='\'')
        {
            quote2 = !quote2;
        }
        else if(sc->ch=='\\')
        {
            if(quote2)
            {
                if(sc->chNext=='\'')
                    sc->Forward();
            }
        }
        else if(sc->ch=='[')
        {
            bracket++;
        }
        else if(sc->ch==']')
        {
            bracket--;
        }
        else if(!quote && !quote2 && sc->ch=='=')
        {
            isattr=true;
            attrpos=sc->currentPos;
            break;
        }
        sc->Forward();
    }
    sc->currentPos=orgpos-2;
    sc->Forward();
    sc->Forward();
    if(isattr)
    {
        bool first=true;
        sc->SetState(SCE_BKE_ATTRIBUTE);
        while(sc->currentPos<attrpos)
        {
            if((first && isIdentifyBegin(sc->ch))||
               (!first && isIdentifyBody(sc->ch)))
            {
                first = false;
            }
            else
            {
                sc->SetState(SCE_BKE_ERROR);
            }
            sc->Forward();
        }
        sc->SetState(SCE_BKE_DEFAULT);
    }
    if(sc->ch=='=')
        sc->Forward();
    SetValue(sc, BeginWithAt);
}

static inline void HandleAtCommand( StyleContext *sc )
{
    sc->SetState(SCE_BKE_COMMAND);    //设置状态，前面的立即着色
	//命令名
	if (!passCmdName(sc, false))
	{
		sc->ChangeState(SCE_BKE_ERROR);
		sc->SetState(SCE_BKE_ERROR);
	};
    sc->SetState(SCE_BKE_DEFAULT);

    //if(!sc->atLineEnd && sc->More())
    //    sc->Forward();
    //else{
    //    sc->ChangeState(SCE_BKE_ERROR);
    //    sc->SetState(SCE_BKE_DEFAULT);  //改变为错误状态，着色
    //}
    //while( !sc->atLineEnd && sc->More() && !isSpace(sc->ch) && (sc->ch != '/' || sc->chNext != '/')) sc->Forward();
    while(!sc->atLineEnd)
    {
        while(sc->More() && !sc->atLineEnd && isSpace(sc->ch) )sc->Forward();
        if(sc->ch == '/' && sc->chNext == '/') break;
        SetAttr(sc);
    }
}

static inline void HandleCommand( StyleContext *sc )
{
	int pos1 = sc->currentPos;
    sc->SetState(SCE_BKE_COMMAND);
	//sc->ForwardSetState(SCE_BKE_DEFAULT);
    //命令名
	if(!passCmdName(sc, false))
	{
		sc->resetState(pos1, pos1, SCE_BKE_ERROR);
	};
	sc->SetState(SCE_BKE_DEFAULT);

    while(!isLineEnd(sc) && sc->ch!=']')
    {
        while(sc->More() && !sc->atLineEnd && isSpace(sc->ch))sc->Forward();
        SetAttr(sc, false);
    }

	if (sc->ch != ']')
	{
		sc->resetState(pos1, pos1, SCE_BKE_ERROR);
	}
    else{
        sc->SetState(SCE_BKE_COMMAND);
        sc->Forward();
        sc->SetState(SCE_BKE_DEFAULT);
    }

}

static void SetLabel( StyleContext *sc )
{
    sc->SetState(SCE_BKE_LABEL);
    while( !sc->atLineEnd && sc->More() && !isSpace(sc->ch)) sc->Forward();
    sc->SetState(SCE_BKE_DEFAULT);
}

static void SetParser( StyleContext *sc)
{
    sc->SetState(SCE_BKE_DEFAULT);

    int ki = 1 ;
    char ls[24] ;
    memset(&ls[0],0,24) ;
    ls[0] = 32 ;
    bool isk ;

    while( sc->More() && !sc->atLineEnd ){

        isk = isSeparator(sc->ch) ;
        if( isk ){
            ls[ki] = 32 ;  //总是有一个空格，保证完全匹配
            if( strstr(BKE_PARSER_KEY,&ls[0]) != NULL && ls[1] > 32){ sc->ChangeState(SCE_BKE_UNTYPEA); sc->SetState(SCE_BKE_DEFAULT);}
            ki = 1 ;
            sc->Forward();
            memset(&ls[0],0,24) ;
            ls[0] = 32 ; //开始总要有一个空格
            sc->SetState(SCE_BKE_DEFAULT);
            continue;
        }
        else if( ki < 23){
            ls[ki] = sc->ch ;
            ki++ ;
        }

        //注释
		if (sc->ch == '/' && sc->chNext == '/') return;
		else if (sc->atLineStart && sc->ch == ';')
			return;//注释
        else if(sc->ch == '[') return;
        //返回
        else if( sc->ch == '#' && sc->chNext == '#'  ){
            sc->SetState(SCE_BKE_DEFAULT);
            sc->Forward(2);
            return ;
        }
        else if( sc->ch == '\"' ) SetString(sc);
        else if( sc->ch == '\'' ) SetString2(sc);
        else if( isk && isoperator(sc->ch) ) sc->SetState(SCE_BKE_OPERATORS);

        sc->Forward();
    }
}

static void ColouriseBkeDoc(unsigned int startPos, int length, int initStyle,
                           WordList *keywordlists[], Accessor &styler) {

	styler.StartAt(startPos);

	StyleContext sc(startPos, length, initStyle, styler);

    while(sc.More())
    {
        while(sc.More() && sc.ch < 33)
			sc.Forward();

        if( sc.ch == '@' )
			HandleAtCommand(&sc);
        else if(sc.ch == '[' )
			HandleCommand( &sc );
        else if(sc.ch == '/' && sc.chNext == '/' ) 
			AnnotateLine( &sc );
		else if (sc.ch == ';' && sc.atLineStart) 
			AnnotateLine(&sc);
		else if (sc.ch == '/' && sc.chNext == '*')
		{
			AnnotateBlock(&sc);
		}
		else if (sc.ch == '*') 
			SetLabel(&sc);
		//else if (sc.ch == '#')
		//{
		//	if (sc.chNext == '#')
		//		setParser(&sc);
		//	else
		//		setParser2(&sc);
		//}
        else if(sc.ch == '\"') 
			SetString(&sc);
        else if(sc.ch == '\'') 
			SetString2(&sc);
        else
			SetText( &sc );
    }
	sc.Complete();
}


LexerModule lmBke(SCLEX_BKE, ColouriseBkeDoc, "bke");
