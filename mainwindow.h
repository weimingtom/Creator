#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <qtnetwork/QNetworkReply>
#include "weh.h"
#include "codewindow.h"
#include "projectwindow.h"
#include "otherwindow.h"
#include "otherbasicwin.h"
#include "dia/bkeleftfilewidget.h"
#include "dia/qsearchlineedit.h"
#include "dia/lablesuredialog.h"

extern QSplitter *ras[3] ;
extern CodeWindow *codeedit ;
extern OtherWindow *otheredit ;
extern ProjectWindow *projectedit ;
extern BkeLeftFileWidget *fileListWidget ;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

signals:

public slots:
    void HiddenLeft() ;
    void AboutBkeCreator() ;
    void ClearMenu() ;
    void ReflashMenu() ;
    void Recentfile() ;
    void NormalAction() ;
    void Bedestroy() ;
    void CurrentFileChange(const QString &name,const QString &prodir) ;
    void HelpCreator() ;
    void CheckUpdate() ;
    void CheckConnect() ;
    void upfileFinish(QNetworkReply *netf) ;

private:
//    CodeWindow *codeedit ;
//    FileWindow *fileedit ;
//    OtherWindow *otheredit ;
//    ProjectWindow *projectedit ;
//    BkeLeftFileWidget *fileListWidget ;

    //QSplitter *ras[3] ;
    QToolBar *downbar ;

    QMenu *wmenu ;
    QMenu *rpmenu ;
    QMenu *rfmenu ;
    QAction *btnhiddenleftact ;
    QAction *btnnewprojectact ;
    QAction *btnopenprojectact ;
    QAction *btnopenfileact ;
    QAction *btnnewfileact ;
    QToolBar *btnbar ;

    QSearchLineEdit *editsearch ;
    QNetworkAccessManager *netAdimin ;

    QVector<QFile*> FilePtrList ;
    QStringList OpenNameList ;

    //各类视图
    QTreeWidget *ProjectBasic ;
    QListWidget *OpenFileBasic ;
    QTreeWidget *BookMarkBasic ;
    QTreeWidget *MarkBasic ;

    bool isConnetct ;

    void CreateMenu() ;
    void CreateDownBar() ;
    void BtnChangeStyle( QWidget *btn) ;
    QList<QAction*> SetMenuList(QMenu *mn,const QStringList &list) ;
    void isUpdate(QJsonObject &newJSON) ;

    void test() ;
protected:
    void closeEvent(QCloseEvent *e);

};

#endif // MAINWINDOW_H