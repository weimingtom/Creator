﻿#ifndef CODEWINDOW_H
#define CODEWINDOW_H
#include "weh.h"
#include <QDockWidget>
#include "topbarwindow.h"
#include "bkeSci/bkescintilla.h"
#include "projectwindow.h"
#include "bkeSci/bkecompile.h"
#include "bkeSci/bkemarks.h"
#include "otherwindow.h"
#include <Qsci/qscistyle.h>
#include <QProgressBar>
#include <QFileSystemWatcher>
#include "dia/searchbox.h"
#include "dia/bkeleftfilewidget.h"

class BkeDocBase
{
public:
	BkeScintilla *edit;

	BkeDocBase(){ edit = 0; }
	~BkeDocBase(){
		//delete edit;
		if (fileIO.isOpen()) fileIO.close();
	}

	QString FullName(){ return fullname; }
	QString Name(){ return name; }
	QString ProjectDir(){ return prodir; }
	QFile* File(){ return &fileIO; }

	bool isNull(){  //不是绝对的
		return edit == 0;
	}

	bool isProjectFile(){
		return !prodir.isEmpty();
	}

	void SetProjectDir(const QString &n){
		prodir = n;
	}

	void SetFileName(const QString &e){
		if (fileIO.isOpen()) fileIO.close();
		fullname = e;
		if (e.lastIndexOf('/') > 0){
			name = e.right(e.length() - e.lastIndexOf('/') - 1);
		}
		else name = e;
		info.setFile(fullname);
		fileIO.setFileName(fullname);
		upFileTime();
	}

	void upFileTime(){
		info.refresh();
	}

	bool isFileChange(){  //文件是否从外部被改变
		QDateTime t1 = info.lastModified();
		info.refresh();
		return t1 != info.lastModified();
	}

	QString dir(){
		QString temp;
		temp = fullname.left(fullname.length() - name.length());
		if (temp.endsWith("/")) return temp.left(temp.length() - 1);
		return temp;
	}

	QWidget *widget(){ return edit; }

private:
	QFile fileIO;
	QString fullname;
	QString name;
	QString prodir;
	QFileInfo info;
};


class CodeWindow : public QMainWindow
{
	Q_OBJECT

public:

	friend class SearchThread;

	~CodeWindow();
	CodeWindow(QWidget *parent = 0);
	QSize sizeHint() const;
	void OtherWinStartX(ProjectWindow *p, OtherWindow *win, BkeLeftFileWidget *flist);
	void OtherWinOtherwin(OtherWindow *win);
	void OtherWinProject(ProjectWindow *p);
	void OtherwinFileList(BkeLeftFileWidget *flist);
	enum{
		ERROR_NO,
		ERROR_SAVE
	};

	BkeLeftFileWidget *filewidget;


	QAction *jumpToDef;
	QAction *jumpToCode;
	QAction *gotoLabel;

	QAction *btnnextact;
	QAction *btnlastact;
	QAction *btnsaveact;
	QAction *btnsaveasact;
	QAction *btncopyact;
	QAction *btncutact;
	QAction *btnpasteact;
	QAction *btncompileact;
	QAction *btncompilelang;
	QAction *btncompilerunact;
	QAction *btnrunact;
	QAction *btndebugact;
	QAction *btncloseact;
	QAction *btnfindact;
	QAction *btnreplaceact;
	QAction *btnbookmarkact;
	QAction *btnmarkact;
	QAction *btnredoact;
	QAction *btnundoact;
	QAction *btnclearact;
	QAction *btncodeact;
	QAction *btnselectall;
	QAction *btnfly;
	QToolBar *toolbar;
	QComboBox *slablelist;
	QStringList slablels;
	bool ignoreActive;

signals:
	void CurrentFileChange(const QString &file);
	void CurrentFileChange(const QString &name, const QString &prodir);
	void FileAddProject(const QString &file);

	void CompileFinish(int errors);
signals:
	void searchOne(const QString &file, const QString &fullfile, int line, int start, int end);

public slots:
	void onTimer();
	bool CloseAll();
	void ChangeCurrentEdit(int pos);
	void SetCurrentEdit(int pos);
	void addFile(const QString &file, const QString &prodir);
	void TextInsert(const QString &text);
	void DocChange(bool m);
	void NextEdit();
	void LastEdit();
	void SaveFile();
	void SaveAs();
	void SaveALL();
	void CloseFile();
	void CloseFile(int pos);
	void ImportBeChange(const QString &text, int type);
	void FileWillBeDel(const QString &file);
	void Compile();
	void CompileAll(bool release = false);
	void CompileLang(bool release = false);
	void CompileFinish();
	void CompileError(QString s);
	void CompileAndRun();
	void FileNameChange(const QString &oldname, const QString &newname, bool &c);
	void ToLocation(BkeMarkerBase *p, const QString &prodir);
	void ShowRmenu(const QPoint & pos);
	void AddBookMark();
	void RunBKE();
	void AnnotateSelect();
	void ClearCompile();
	void ChangeCodec();
	void NewEmptyFile();
	void FileReadyToCompile(int i);
	void ChangeProject(BkeProject *p);
	void TextToMarks(const QString &text, const QString &dir, int type);
	void deleteCompileFile();
	void SelectAll();
	void QfileChange(const QString &path);
	void GotoLine();
	void GotoLabel(int i);
	void ActUndo();
	void ActRedo();
	void ActCurrentChange();
	void ActCut();
	void ActPaste();
	void ActCopy();
	void Rename(const QString &old, const QString &now);
	void searchOneFile(const QString &file, const QString &searchstr, bool iscase, bool isregular, bool isword);
	void searchAllFile(const QString &searchstr, bool iscase, bool isregular, bool isword);
	void replaceOneFile(const QString &file, const QString &searchstr, const QString &replacestr, bool iscase, bool isregular, bool isword, bool stayopen);
	void replaceAllFile(const QString &searchstr, const QString &replacestr, bool iscase, bool isregular, bool isword, bool stayopen);
	void resetLexer();
	void refreshLabel(BkeScintilla *sci);
	void refreshLabel(set<QString> &l);
	void jumpToDefFunc();
	void jumpToCodeFunc();
	void jumpToLabelFunc();

public:
	void backupAll();
	static QString getScenarioTextFromCode(QString text);

private:
	OtherWindow *othwin;
	ProjectWindow *prowin;
	BkeProject *currentproject;
	QStackedWidget *stackwidget;

	QStringList ItemTextList;
	QComboBox *lablelist;

	BkeScintilla *currentedit;
	BkeDocBase* currentbase;

	QHash<QString, BkeDocBase*> docStrHash;
	QHash<QWidget*, BkeDocBase*> docWidgetHash;
	QFileSystemWatcher *filewatcher ;


	QToolBar *toolbar2;
	BkeCompile comtool;
	BkeMarkSupport markadmin;
	QString ComText;
	QAction *pdeletebookmark;
	QAction *pdeletemark;
	QAction *pannote;
	SearchBox *diasearch;
	QProgressBar *kag;
	BkeScintilla *searchlablelater;

	int currentpos;
	int errorinfo;
	bool ignoreflag;
	bool isRun;
	bool isSearLable;
	int watcherflag ;
	int isCompileNotice;
	bool labelbanned;

	QSize hint;
	QsciStyle ks1;
	QsciStyle ks2;

	enum{
		_NOTICE_ALWAYS,
		_NOTICE_NOSAVE,
		_NOTICE_ALLSAVE
	};

	QTimer tm;
	void simpleNew(BkeDocBase *loli, const QString &t);
	void simpleSave(BkeDocBase *loli);
	bool simpleBackup(BkeDocBase *loli);
	void simpleClose(BkeDocBase *loli);
	void SetCurrentEdit(QWidget *w);
	void CreateBtn();
	void btnDisable();
	//void btnAble() ;
	void CurrentConnect(bool c);
	//bool ReadyCompile(const QString &file) ;
	void CheckProblemMarks(BkeScintilla *edit, BkeMarkList *list);
	void CheckBookMarks(BkeScintilla *edit, BkeMarkList *list);
	void CheckMarks(BkeScintilla *edit, BkeMarkList *list);
	void AddMarksToEdit();
	void FileIOclose(const QStringList &list);
	void BackstageSearchLable(BkeScintilla *edit);
	void DrawLine(bool isClear);
	bool WriteOpenFile(const QString &dir);
};

#endif // CODEWINDOW_H
