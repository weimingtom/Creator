﻿#ifndef PARSER_H
#define PARSER_H

//#define PARSER_PRO


#include <QStringList>
#include <QTextStream>
#include <QTextCodec>
#include <QStack>
#include <QFile>
#include <QObject>
#include <Qsci/qsciscintilla.h>
#include "wordsupport.h"
#include "completebase.h"
#include "bkeSci/BkeIndicatorBase.h"

#ifndef PARSER_PRO
#include <function.h>
#endif

extern QStringList SYSlist ;
extern QStringList KEYlist ;

class BkeParser :public QObject
{
    Q_OBJECT
signals:
    void IfOpenGetText(const QString &file, QString &text) ;
    void ErrorAt(int pos ,const QString &info) ;
public:
    enum{
        SHOW_NULL ,
        SHOW_USECOMMANDLIST ,
        SHOW_AUTOCOMMANDLIST ,
        SHOW_AUTOVALLIST ,
        SHOW_USEVALLIST ,
        SHOW_LABEL ,
        SHOW_SYS
    };

    BkeParser(QObject *parent = 0);
    ~BkeParser() ;

    void ParserText( const QString &t) ;
    void ParserFile(const QString &file,const QString &dirfrom) ;
    void PushParserText( const QString &t) ;
    void SetDir(const QString &d){ dir = d ;}
    const char* GetCommandList( const QString &t) ;
    const char* GetValList( const QString &t) ;
    const char* GetLabelList( const QString &t) ;
    const char* GetList(const QString &t) ;
    void SetCommandList(const QString &t) ;
    void SetVarList(const QString &t) ;
    bool IfHasThenSet(const QString &t) ;
    bool HasTheChileOf(const QString &t) ;
    void TextBeChange(BkeModifiedBase *modbase,QsciScintilla *sciedit) ;
    int  GetIndentLayer(QsciScintilla *edit,int line) ;
    int  GetLessIndent(QsciScintilla *edit,int line) ;
    QStringList GetImportFiles(){ return importfile ; }
    bool isListShow(){ return showtype != SHOW_NULL ; }
    QString GetInfo(const QString &t) ;

    int showtype ;
    CompleteBase VariablesBase ; //类、字典、变量
    CompleteBase CommandBase ;
    CompleteBase *valBase ;
    CompleteBase *apiBase ;

private:
    CompleteBase EmptyBase ;
    WordSupport *wordadmin ;
    QString dir ;
    QString errorinfo ;
    QStringList importfile ;

    QByteArray outbyte ;
    QStringList outlist ;
    int errorid ;
    bool ErrorFlag ;
    bool KeyFlag ;
    int  completeState ;
    int  IndentLayer ;

    QStack<WordSupport*> StackWordadmin ;
    void ErrorSub(int pos,const QString &info) ;
    void CommandStart() ;
    void defCommand(int pos = -1) ;
    void defFunction() ;
    void defDictionary(CompleteBase *le) ;
    void defLabel() ;
    bool CLeftExp() ;//@左表达
    bool CRightExp() ; //@右表达
    int  RightExp(bool next = true) ; //右表达式
    void SetVariable(int pos=-1,bool toend=false) ;
    void ParserModel(bool issub) ;
    void ParserModel2() ;
    void ImportFile() ;

};

#endif // PARSER_H