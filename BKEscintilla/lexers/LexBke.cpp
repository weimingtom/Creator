// Scintilla source code edit control
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
#define SCE_BKE_UNTYPEB         11
#define SCE_BKE_UNTYPEC         12
#define SCE_BKE_ERROR           13

static const char *BKE_PARSER_KEY = " for foreach in extends do while function propset propget int string number typeof var delete class if else continue break return this true false void global " ;
static const char *BKE_SEPARATEOR = " ~!@#$%^&*()-+*/|{}[]:;/=.,?><\\" ;

bool isSeparator(char ch)
{
    char bt[2] ;
    bt[0] = ch ;
    bt[1] = 0 ;
    if( ch < 32 ) return true ;
    return (strstr(BKE_SEPARATEOR,&bt[0]) != NULL) ;
}

// Extended to accept accented characters
static inline void AnnotateLine( StyleContext *sc )//整行注释掉
{
    sc->SetState(SCE_BKE_ANNOTATE);   //注释状态
    while( !sc->atLineEnd && sc->More() ) sc->Forward();
    sc->SetState(SCE_BKE_DEFAULT);   //注释状态
    return ;
}

static inline void SetText( StyleContext *sc )
{
    sc->SetState(SCE_BKE_TEXT);
    while( !sc->atLineEnd && sc->ch!='[' && sc->More()) sc->Forward();
    sc->SetState(SCE_BKE_DEFAULT);
}

static inline void SetString( StyleContext *sc)
{
    int k = sc->state ;
    sc->SetState(SCE_BKE_STRING);
    while(sc->More() && !sc->atLineEnd)
    {
        sc->Forward();
        if(sc->ch=='\"' && sc->chNext!='\"')
            break;
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
                if(sc->ch=='\"' && sc->chNext!='\"')
                    break;
            }
            sc->Forward();
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
        else if(!isspace(sc->ch) || bracket[bracket.size()-1]>'0')
        {
            if(isspace(sc->ch))
            {
                if(sc->More() && !sc->atLineEnd)
                {
                    sc->Forward();
                    sc->SetState(SCE_BKE_DEFAULT);
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
    long orgpos=sc->currentPos;
    bool isattr=false;
    while(sc->More())
    {
        sc->Forward();
        if(sc->ch=='=')
        {
            isattr=true;
            break;
        }
        if(!isalpha(sc->ch))
            break;
    }
    sc->currentPos=orgpos-2;
    sc->Forward();
    sc->Forward();
    if(isattr)
    {
        sc->SetState(SCE_BKE_ATTRIBUTE);
        sc->Forward();
        while(sc->More() && isalpha(sc->ch)) sc->Forward();
        sc->SetState((SCE_BKE_DEFAULT));
    }
    if(sc->ch=='=')
        sc->Forward();
    SetValue(sc, BeginWithAt);
}

static inline void HandleAtCommand( StyleContext *sc )
{
    sc->SetState(SCE_BKE_COMMAND);    //设置状态，前面的立即着色
    if(!sc->atLineEnd && sc->More())
        sc->Forward();
    else{
        sc->ChangeState(SCE_BKE_ERROR);
        sc->SetState(SCE_BKE_DEFAULT);  //改变为错误状态，着色
    }
    while( !sc->atLineEnd && !isspace(sc->ch) && sc->More() && (sc->ch != '/' || sc->chNext != '/')) sc->Forward();
    sc->SetState(SCE_BKE_DEFAULT);
    while(!sc->atLineEnd)
    {
    	while(sc->More() && !sc->atLineEnd && isspace(sc->ch) )sc->Forward();
        if(sc->ch == '/' && sc->chNext == '/') break;
        SetAttr(sc);
    }
}

static inline void HandleCommand( StyleContext *sc )
{
    sc->SetState(SCE_BKE_COMMAND);
    if(!sc->atLineEnd && sc->More()) sc->Forward();
    else{
        //sc->ChangeState();
        sc->SetState(SCE_BKE_ERROR);
    }
    //命令本身
    while( !sc->atLineEnd && !isspace(sc->ch) && sc->ch!=']' && sc->More()) sc->Forward();
    sc->SetState(SCE_BKE_DEFAULT);

    while(!sc->atLineEnd && sc->ch!=']')
    {
        while(sc->More() && !sc->atLineEnd && isspace(sc->ch))sc->Forward();
        SetAttr(sc, false);
    }

    if(sc->ch!=']')
        sc->SetState(SCE_BKE_ERROR);
    else{
        sc->SetState(SCE_BKE_COMMAND);
        sc->Forward();
        sc->SetState(SCE_BKE_DEFAULT);
    }

}

static void SetLabel( StyleContext *sc )
{
    sc->SetState(SCE_BKE_LABEL);
    while( !sc->atLineEnd && sc->More()) sc->Forward();
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
            if( strstr(BKE_PARSER_KEY,&ls[0]) != NULL && ls[1] > 32) sc->ChangeState(SCE_BKE_UNTYPEA);
            ki = 1 ;
            memset(&ls[0],0,24) ;
            ls[0] = 32 ; //开始总要有一个空格
            sc->SetState(SCE_BKE_DEFAULT);
        }
        else if( ki < 23){
            ls[ki] = sc->ch ;
            ki++ ;
        }

        //注释
        if(sc->ch == '/' && sc->chNext == '/' ) return;
        else if(sc->ch == '[') return;
        //返回
        else if( sc->ch == '#' && sc->chNext == '#'  ){
            sc->SetState(SCE_BKE_DEFAULT);
            sc->Forward(2);
            return ;
        }
        else if( sc->ch == '\"' ) SetString(sc);

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
        while(sc.More() && sc.ch < 33) sc.Forward();

        if( sc.ch == '@' )HandleAtCommand(&sc);
        else if(sc.ch == '[' ) HandleCommand( &sc );
        //else if(sc.ch == ';' ) AnnotateLine( &sc );
        else if(sc.ch == '/' && sc.chNext == '/' ) AnnotateLine( &sc );
        else if(sc.ch == '*') SetLabel(&sc);
        //else if(sc.ch == '#' && sc.chNext == '#' ) SetParser( &sc );
        else if(sc.ch == '\"') SetString(&sc);
        else SetParser( &sc );
    }
	sc.Complete();
}


LexerModule lmBke(SCLEX_BKE, ColouriseBkeDoc, "bke");
