#ifndef OTHERWINDOW_H
#define OTHERWINDOW_H
#include "weh.h"
#include "topbarwindow.h"
#include "bkeSci/bkemarks.h"

//打开文件视窗

class OtherWindow : public TopBarWindow
{
    Q_OBJECT
public:
    enum{
        WIN_PROBLEM ,
        WIN_SEARCH ,
        WIN_COMPILE ,
        WIN_BOOKMARK ,
        WIN_MARK
    };

    QPushButton *btnproblem ;
    QPushButton *btnsearch ;
    QPushButton *btncompiletext ;
    QPushButton *btnbookmark ;
    QPushButton *btnmark ;

    QStackedWidget *stackw ;
    QListWidget *ProblemList ;
    QListWidget *searchlist ;
    QsciScintilla *compileedit ;
    QListWidget *bookmarklist ;
    QListWidget *marklist ;

    OtherWindow(QWidget *parent = 0);

    int ErrorCount(){ return errorcount ; }
    void ShowProblem(BkeMarkList *list) ;
    void IfShow(QPushButton *btn,bool must = false) ;
    void RefreshBookMark(BkeMarkList *b) ;

signals:
    void Location(BkeMarkerBase *mark) ;

public slots:
    void ProblemDoubleClicked(QListWidgetItem * item) ;
    void BookMarkDoubleClicked(QListWidgetItem * item) ;
    void MarkDoubleClicked(QListWidgetItem * item) ;
    void WINproblem() ;
    void WINsearch() ;
    void WINcomile() ;
    void WINbookmark() ;
    void WINmark() ;

private:
    QPushButton *lastbtn ;
    int errorcount ;

    QIcon *icoerror ;
    QIcon *icowarning ;
    QIcon *icobookmark ;

    QColor bc1 ;
    QColor bc2 ;

    BkeMarkList membookmark ;
    BkeMarkList memmark ;
    BkeMarkList memproblem ;

    QHash<QPushButton*,QWidget*> emap ;
    void BuildBookMarker() ;
    void BuildProblem() ;
    void BuildMarker() ;
    void AddItem(BkeMarkerBase *marker, QIcon *ico,QListWidget* w) ;

};

#endif // OTHERWINDOW_H