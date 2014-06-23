#include "codewindow.h"
#include "dia/lablesuredialog.h"

CodeWindow::CodeWindow(QWidget *parent)
    :QMainWindow(parent)
{
    diasearch  = new SearchBox(this) ;
    addDockWidget(Qt::BottomDockWidgetArea,diasearch);
    currentedit = 0 ;

    QSize fff = parent->size() ;
    hint.setWidth(fff.width()*0.8);
    hint.setHeight(fff.height()-50);

    ks1.setColor(QColor(0x80,0,0));
    ks1.setPaper(QColor(0xff,0xf0,0xf0));
    ks2.setColor(QColor(0x80,0,0x80));
    ks2.setPaper(QColor(0xff,0xf0,0xff));
    CreateBtn();

    filewatcher = new QFileSystemWatcher(this) ;
    watcherflag = 1 ;

    currentpos = -1 ;
    stackwidget = new QStackedWidget(this) ;
    setCentralWidget(stackwidget);
    DrawLine(false);

    this->addToolBar(Qt:: BottomToolBarArea,toolbar2);


    connect(lablelist,SIGNAL(currentIndexChanged(int)),this,SLOT(SetCurrentEdit(int))) ;
    connect(stackwidget,SIGNAL(currentChanged(int)),this,SLOT(SetCurrentEdit(int))) ;
    connect(btnnextact,SIGNAL(triggered()),this,SLOT(NextEdit())) ;
    connect(btnlastact,SIGNAL(triggered()),this,SLOT(LastEdit())) ;
    connect(btnsaveact,SIGNAL(triggered()),this,SLOT(SaveFile())) ;
    connect(btncloseact,SIGNAL(triggered()),this,SLOT(CloseFile())) ;
    connect(btnsaveasact,SIGNAL(triggered()),this,SLOT(SaveAs())) ;
    //编译
    connect(btncompileact,SIGNAL(triggered()),this,SLOT(CompileAll())) ;
    connect(btnbookmarkact,SIGNAL(triggered()),this,SLOT(AddBookMark())) ;
    //查找
    connect(btnfindact,SIGNAL(triggered()),diasearch,SLOT(SearchModel())) ;
    connect(btnreplaceact,SIGNAL(triggered()),diasearch,SLOT(ReplaceModel())) ;
    connect(&comtool,SIGNAL(CompliteFinish()),this,SLOT(CompileFinish())) ;
    connect(btnrunact,SIGNAL(triggered()),this,SLOT(RunBKE())) ;
    connect(pannote,SIGNAL(triggered()),this,SLOT(AnnotateSelect())) ;
    connect(btnclearact,SIGNAL(triggered()),this,SLOT(ClearCompile())) ;
    //编码转换
    connect(btncodeact,SIGNAL(triggered()),this,SLOT(ChangeCodec())) ;
    connect(btnselectall,SIGNAL(triggered()),this,SLOT(SelectAll())) ;
    //文件从外部改变
    connect(filewatcher,SIGNAL(fileChanged(QString)),this,SLOT(QfileChange(QString))) ;
    //转到行
    connect(btnfly,SIGNAL(triggered()),this,SLOT(GotoLine())) ;
    //编译并运行
    connect(btncompilerunact,SIGNAL(triggered()),this,SLOT(CompileAndRun())) ;
    //点击标签
    connect(slablelist,SIGNAL(currentIndexChanged(int)),this,SLOT(GotoLable(int))) ;


    btnDisable();
    ChangeProject(0);
    ignoreflag = false ;
    isRun = false ;
    isSearLable = false ; //是否在查找标签，如果是，刷新文件队列

    searchlablelater = 0 ;
    isCompileNotice = _NOTICE_ALWAYS ;

}

void CodeWindow::CreateBtn()
{
    toolbar = new QToolBar ;
    toolbar->setFixedHeight(24);
    toolbar->setStyleSheet(BKE_QCSS_OBJECT.value("toolbar2").toString());


    btnlastact = new QAction(QIcon(":/cedit/source/btnlast.png"),"上一个窗口",this) ;
    btnnextact = new QAction(QIcon(":/cedit/source/btnnext.png"),"下一个窗口",this) ;
    btnsaveact = new QAction(QIcon(":/cedit/source/save.png"),"保存",this) ;
    btnsaveasact = new QAction(QIcon(":/cedit/source/saveas.png"),"另存为...",this) ;
    btncodeact = new QAction(QIcon(":/cedit/source/code.png"),"改变源文件的编码",this) ;
    btncopyact = new QAction(QIcon(":/cedit/source/copy.png"),"复制",this) ;
    btncutact = new QAction(QIcon(":/cedit/source/edit_cut.png"),"剪切",this) ;
    btnpasteact = new QAction(QIcon(":/cedit/source/paste.png"),"粘贴",this) ;
    btncompileact = new QAction(QIcon(":/cedit/source/compile.png"),"编译成bkbin脚本",this) ;
    btncompilerunact = new QAction(QIcon(":/cedit/source/compile_run.png"),"编译并运行",this) ;
    btnrunact = new QAction(QIcon(":/cedit/source/run.png"),"运行",this) ;
    btndebugact = new QAction(QIcon(":/cedit/source/debug.png"),"调试",this) ;
    btncloseact = new QAction(QIcon(":/cedit/source/close.png"),"关闭",this) ;
    btnfindact = new QAction(QIcon(":/cedit/source/find.png"),"查找",this) ;
    btnreplaceact = new QAction(QIcon(":/cedit/source/replace(2).png"),"替换",this) ;
    btnbookmarkact = new QAction(QIcon(":/cedit/source/Bookmark.png"),"添加书签",this) ;
    btnmarkact = new QAction(QIcon(":/cedit/source/pin.png"),"添加标记",this) ;
    btnredoact =  new QAction(QIcon(":/cedit/source/redo.png"),"重做",this) ;
    btnundoact = new QAction(QIcon(":/cedit/source/undo.png"),"撤销",this) ;
    btnclearact = new QAction(QIcon(":/cedit/source/clear.png"),"清理编译项目",this) ;
    pannote = new QAction("选中部分注释/反注释",this) ;
    btnselectall = new QAction("全选",this) ;
    btnfly = new QAction(QIcon(":/cedit/source/flay.png"),"转到行...",this) ;

    btnsaveact->setShortcut( Qt::CTRL + Qt::Key_S );
    btncopyact->setShortcut( Qt::CTRL + Qt::Key_C );
    btnpasteact->setShortcut( Qt::CTRL + Qt::Key_V );
    btncutact->setShortcut( Qt::CTRL + Qt::Key_X );
    btnundoact->setShortcut( Qt::CTRL + Qt::Key_Z );
    btnredoact->setShortcut( Qt::CTRL + Qt::Key_Y );
    btnfindact->setShortcut( Qt::CTRL + Qt::Key_F );
    btncompileact->setShortcut( Qt::CTRL + Qt::Key_B );
    btnselectall->setShortcut( Qt::CTRL + Qt::Key_A );
    btnfly->setShortcut(  Qt::CTRL + Qt::Key_G  );
    btncompilerunact->setShortcut(   Qt::CTRL + Qt::Key_R  );

    toolbar->addAction(btnlastact) ;
    toolbar->addAction(btnnextact) ;

    lablelist = new QComboBox ;
    lablelist->setFixedSize(150,22);
    lablelist->setStyleSheet(BKE_QCSS_OBJECT.value("combox2").toString());
    lablelist->setView(new QListView());
    toolbar->addWidget(lablelist) ;

    toolbar->addSeparator() ;
    toolbar->addAction(btnsaveact) ;
    toolbar->addAction(btnsaveasact) ;
    toolbar->addAction(btncodeact) ;
    toolbar->addSeparator() ;
    toolbar->addAction(btnundoact) ;
    toolbar->addAction(btnredoact) ;
    toolbar->addAction(btnfindact) ;
    toolbar->addAction(btnreplaceact) ;
    toolbar->addSeparator() ;
    toolbar->addAction(btncopyact) ;
    toolbar->addAction(btncutact) ;
    toolbar->addAction(btnpasteact) ;
    toolbar->addSeparator() ;
    toolbar->addAction(btnbookmarkact) ;
    toolbar->addAction(btnmarkact) ;
    toolbar->addSeparator() ;



    slablelist = new QComboBox ;
    slablelist->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    slablelist->setStyleSheet(BKE_QCSS_OBJECT.value("combox2").toString());
    slablelist->setView(new QListView());
    toolbar->addWidget(slablelist);
    toolbar->addAction(btncloseact) ;



    //下边工具栏
    toolbar2 = new QToolBar(this) ;
    toolbar2->setFixedHeight(24);
    toolbar2->setMovable(false);
    toolbar2->addAction(btncompileact) ;
    toolbar2->addAction(btncompilerunact) ;
    toolbar2->addAction(btnrunact) ;
    toolbar2->addAction(btndebugact);
    toolbar2->addAction(btnclearact) ;

    QWidget *space = new QWidget(this) ;
    space->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    toolbar2->addWidget(space);
    kag = new QProgressBar(this) ;
    kag->setFixedSize(200,20);
    kag->setStyleSheet(BKE_QCSS_OBJECT.value("processbar").toString());
    toolbar2->addWidget(kag) ;

}

void CodeWindow::OtherWinOtherwin(OtherWindow *win)
{
    othwin = win ;
    //定位文件
    connect(win,SIGNAL(Location(BkeMarkerBase*)),this,SLOT(ToLocation(BkeMarkerBase*))) ;


}

void CodeWindow::OtherWinProject(ProjectWindow *p)
{
    prowin = p ;
    //打开文件
    connect(p,SIGNAL(OpenThisFile(QString,QString)),this,SLOT(addFile(QString,QString))) ;
    //插入路径
    connect(p,SIGNAL(DirWillBeInsert(QString)),this,SLOT(TextInsert(QString))) ;
    //编译脚本
    connect(p,SIGNAL(Compile()),this,SLOT(CompileAll())) ;
    //当前项目被改变
    connect(p,SIGNAL(CurrentProChange(BKEproject*)),this,SLOT(ChangeProject(BKEproject*))) ;
    //改变当前文件
    connect(this,SIGNAL(CurrentFileChange(QString)),p,SLOT(SetCurrentItem(QString))) ;
    //打开项目时读取书签以及标记
    connect(p,SIGNAL(TextToMarks(QString,QString,int)),this,SLOT(TextToMarks(QString,QString,int))) ;
    //新建空白文档
    //connect(p->btnnewfileact,SIGNAL(triggered()),this,SLOT(NewEmptyFile())) ;
}

void CodeWindow::OtherwinFileList(BkeLeftFileWidget *flist)
{
    filewidget = flist ;
    //改变文件
    connect(filewidget,SIGNAL(currentRowChanged(int)),this,SLOT(SetCurrentEdit(int))) ;
    //关闭文件
    connect(filewidget->btnclose,SIGNAL(triggered()),this,SLOT(CloseFile())) ;
    //关闭所有
    connect(filewidget->btnCloseAll,SIGNAL(triggered()),this,SLOT(CloseAll())) ;
}

//设置正在编辑项目
void CodeWindow::SetCurrentEdit(int pos)
{
    if( ignoreflag ) return ;
    else if( pos == stackwidget->currentIndex() && pos == lablelist->currentIndex()
            && pos == filewidget->currentRow()) return ;

    ignoreflag = true ;
    stackwidget->setCurrentIndex(pos);
    lablelist->setCurrentIndex(pos);
    filewidget->setCurrentRow(pos);
    ignoreflag = false ;

    ChangeCurrentEdit(pos);
}

void CodeWindow::SetCurrentEdit(QWidget *w)
{
    stackwidget->setCurrentWidget(w);
    SetCurrentEdit(stackwidget->currentIndex());
}

//改变正在编辑的文档，文件列表选项是同步改变的
void CodeWindow::ChangeCurrentEdit(int pos)
{
    if( !stackwidget->styleSheet().isEmpty() ) DrawLine(true);

    currentpos = pos ;  //当前位置
    if( currentedit == stackwidget->currentWidget() ) return ;

    currentpos = pos ;  //当前位置
    btncopyact->setEnabled(false);

    //释放文档改变信号
    if( currentpos > 0) CurrentConnect(false);

    currentbase = docWidgetHash.value(stackwidget->currentWidget(),0) ;
    if( currentbase == 0 ){
        QMessageBox::critical(this,"","致命错误：没有找到匹配的QWidget！",QMessageBox::Ok) ;
        return ;
    }

    //改变项目
    ChangeProject( prowin->FindProjectFromDir( currentbase->ProjectDir()) );

    currentedit = currentbase->edit ;
    //连接文档改变信号
    CurrentConnect(true);

    diasearch->SetSci(currentedit) ; //查找管理
    markadmin.SetFile(currentbase->FullName());  //标记管理器
    btnsaveact->setEnabled(currentedit->isModified());
    btnnextact->setEnabled( currentpos < stackwidget->count()-1);
    btnlastact->setEnabled(currentpos > 0);
    btnsaveasact->setEnabled(true);
    btnpasteact->setEnabled(true);
    btnfindact->setEnabled(true);
    btnreplaceact->setEnabled(true);
    btnbookmarkact->setEnabled(true);
    btnmarkact->setEnabled(true);
    btnundoact->setEnabled( currentedit->isUndoAvailable() );
    btnredoact->setEnabled( currentedit->isRedoAvailable() );
    btncodeact->setEnabled(true);
    btncloseact->setEnabled(stackwidget->count() > 0);
    btnfly->setEnabled(true);

    emit CurrentFileChange(currentbase->FullName());
    emit CurrentFileChange(currentbase->Name(),currentbase->ProjectDir());

    AddMarksToEdit();
    BackstageSearchLable(currentedit);
}

//断开、连接当前文档信号
void CodeWindow::CurrentConnect(bool c)
{
    if( c ){
        connect(currentedit,SIGNAL(copyAvailable(bool)),btncopyact,SLOT(setEnabled(bool))) ;
        connect(currentedit,SIGNAL(copyAvailable(bool)),btncutact,SLOT(setEnabled(bool))) ;
        connect(currentedit,SIGNAL(Undoready(bool)),btnundoact,SLOT(setEnabled(bool))) ;
        connect(currentedit,SIGNAL(Redoready(bool)),btnredoact,SLOT(setEnabled(bool))) ;
        connect(btncopyact,SIGNAL(triggered()),currentedit,SLOT(copy())) ;
        connect(btncutact,SIGNAL(triggered()),currentedit,SLOT(cut())) ;
        connect(btnpasteact,SIGNAL(triggered()),currentedit,SLOT(paste())) ;
        connect(btnredoact,SIGNAL(triggered()),currentedit,SLOT(redo())) ;
        connect(btnundoact,SIGNAL(triggered()),currentedit,SLOT(undo())) ;
    }
    else{
        disconnect(currentedit,SIGNAL(copyAvailable(bool)),btncopyact,SLOT(setEnabled(bool))) ;
        disconnect(currentedit,SIGNAL(copyAvailable(bool)),btncutact,SLOT(setEnabled(bool))) ;
        disconnect(currentedit,SIGNAL(Undoready(bool)),btnundoact,SLOT(setEnabled(bool))) ;
        disconnect(currentedit,SIGNAL(Redoready(bool)),btnredoact,SLOT(setEnabled(bool))) ;
        disconnect(btncopyact,SIGNAL(triggered()),currentedit,SLOT(copy())) ;
        disconnect(btncutact,SIGNAL(triggered()),currentedit,SLOT(cut())) ;
        disconnect(btnpasteact,SIGNAL(triggered()),currentedit,SLOT(paste())) ;
        disconnect(btnredoact,SIGNAL(triggered()),currentedit,SLOT(redo())) ;
        disconnect(btnundoact,SIGNAL(triggered()),currentedit,SLOT(undo())) ;
    }
}

void CodeWindow::btnDisable()
{
    btnnextact->setEnabled(false);
    btnlastact->setEnabled(false) ;
    btnsaveact->setEnabled(false) ;
    btnsaveasact->setEnabled(false) ;
    btncopyact->setEnabled(false) ;
    btnpasteact->setEnabled(false) ;
    //btndebugact->setEnabled(false) ;
    btncloseact->setEnabled(false) ;
    btnfindact->setEnabled(false);
    btnreplaceact->setEnabled(false);
    btnbookmarkact->setEnabled(false);
    btnmarkact->setEnabled(false);
    btncutact->setEnabled(false);
    btnredoact->setEnabled(false);
    btnundoact->setEnabled(false);
    btncodeact->setEnabled(false);
    btnfly->setEnabled(false);
}

//打开文件，文件列表是自动维护的
void CodeWindow::addFile(const QString &file,const QString &prodir)
{
    BkeDocBase* loli = docStrHash.value( LOLI_OS_QSTRING(file),0) ;

    //如果为空，则创建
    if( loli == 0 ){
        loli = new BkeDocBase ;
        loli->SetFileName(file);
        if( !loli->File()->exists() ){
            int i = QMessageBox::warning(this,"打开错误","文件：\r\n  "+file+" 不存在！",QMessageBox::Ok|QMessageBox::Ignore) ;
            if( i == QMessageBox::Ok) return;
        }

        QString en ;
        if( !LOLI::AutoRead(en,loli->File()) ){
            QMessageBox::critical(this,"打开错误","文件：\r\n  "+file+" 读取失败！",QMessageBox::Ok) ;
            return ;
        }

        loli->SetProjectDir(prodir);
        //新的编辑窗口
        simpleNew(loli,en);

        BKEproject *tpro = prowin->FindProjectFromDir(prodir) ;
        if( prodir != 0 ) loli->edit->setParser( tpro->lex );

        BkeCreator::AddRecentFile(loli->FullName()) ;
        //添加文件监视
        bool ks = filewatcher->addPath(loli->FullName()) ;
        ks = false ;
    }

    //从当前文档的附近路径中寻找项目，失败返回0
    ChangeProject( prowin->FindProjectFromDir(loli->ProjectDir()) );
    //改变当前显示项
    SetCurrentEdit(loli->edit);
}

void CodeWindow::simpleNew(BkeDocBase *loli,const QString &t)
{
    ignoreflag = true ; //忽略改变，在所有准备工作完成以后才改变

    loli->edit = new BkeScintilla(this) ;
    int pos = LOLI_SORT_INSERT( ItemTextList, loli->Name()) ;
    filewidget->insertItem(pos,loli->Name()) ;
    lablelist->insertItem(pos,loli->Name()) ;
    stackwidget->insertWidget(pos,loli->edit) ;
    docStrHash[ LOLI_OS_QSTRING(loli->FullName()) ] = loli ;
    docWidgetHash[ loli->edit ] = loli ;

    //文字
    loli->edit->setText(t);
    loli->edit->setModified(false);  //没有改变

    //文件被改变
    connect(loli->edit,SIGNAL(modificationChanged(bool)),this,SLOT(DocChange(bool))) ;
    //右键菜单
    connect(loli->edit,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ShowRmenu(QPoint))) ;

    ignoreflag = false ;
}

//文档被改变，标题加*
void CodeWindow::DocChange(bool m)
{
    if( currentpos > lablelist->count()-1 || currentpos < 0) return ;
    QString title = ItemTextList.at(currentpos) ;

    if(m){
        title.append('*') ;
        lablelist->setItemText(currentpos,title);
        filewidget->item(currentpos)->setText(title);
        btnsaveact->setEnabled(true);
    }
    else{
        lablelist->setItemText(currentpos,title);
        filewidget->item(currentpos)->setText(title);
        btnsaveact->setEnabled(false);
    }
}

void CodeWindow::NextEdit()
{
    if( currentpos < stackwidget->count() -1 ){
        SetCurrentEdit(currentpos+1);
    }
}

void CodeWindow::LastEdit()
{
    if( currentpos > 0 ) SetCurrentEdit(currentpos-1);
}

void CodeWindow::SaveALL()
{
    QWidget *p ;
    BkeDocBase *llm ;
    for( int i = 0 ; i < stackwidget->count() ; i++){
        llm = docWidgetHash.value(stackwidget->widget(i)) ;
        if( llm == NULL ) continue ;
        else simpleSave(llm);
    }
}

void CodeWindow::SaveFile()
{
    simpleSave(currentbase);
    return ;
}

void CodeWindow::simpleSave(BkeDocBase *loli)
{
    if( loli->FullName() == "New" ){
        QString temp = QFileDialog::getSaveFileName(this,"保存文件") ;
        if( temp.isEmpty() ) return ;
        else if( !temp.endsWith(".bkscr") && !temp.endsWith(".bkpsr"))
            temp.append(".bkscr") ;
        loli->SetFileName(temp);
    }

    watcherflag = 0 ;
    if( !LOLI::AutoWrite(loli->File(),loli->edit->text(),"UTF-8")){
        QMessageBox::critical(this,"","文件：\r\n"+currentbase->FullName()+" 写出失败！") ;
        watcherflag = 1 ;
        return ;
    }
    loli->edit->setModified(false);
}

//另存为
void CodeWindow::SaveAs()
{
    QString name = QFileDialog::getSaveFileName(this,"另存为","","bkscr脚本(*.bkscr)") ;
    if( name.isEmpty()) return ;
    else if( !name.endsWith(".bkscr")) name.append(".bkscr") ;

    //改变文件监视
    filewatcher->removePath(currentbase->FullName()) ;
    currentbase->SetFileName(name);
    filewatcher->addPath(name) ;
    simpleSave(currentbase);
}

void CodeWindow::CloseFile()
{
    if( currentedit->isModified() ){
        QMessageBox *msg = new QMessageBox(this) ;
        msg->addButton("保存",QMessageBox::AcceptRole);
        msg->addButton("关闭",QMessageBox::RejectRole);
        msg->addButton("取消",QMessageBox::DestructiveRole);

        msg->setText( currentbase->FullName()+"\r\n已经被修改，是否保存？");
        int i = msg->exec() ;
        if( i == QMessageBox::AcceptRole) SaveFile();
        else if( i == QMessageBox::DestructiveRole ) return ;
    }

    simpleClose( currentbase ) ;
}

void CodeWindow::CloseFile(int pos)
{
    if( pos < 0 || pos > ItemTextList.size() ) return ;
    BkeDocBase *llm = docStrHash.value(LOLI_OS_QSTRING(ItemTextList.at(pos))) ;
    simpleClose(llm);
}

void CodeWindow::CloseAll()
{
    QList<BkeDocBase*> ls = docWidgetHash.values() ;

    BkeDocBase *ptr ;
    for( int i = 0 ; i < ls.size() ; i++){
        ptr = ls.at(i) ;
        if( ptr->edit->isModified() ){
            SetCurrentEdit(ptr->edit);
            if( QMessageBox::information(this,"","文件:\r\n"+ptr->FullName()+"\r\n已经修改，是否保存？"
                                         ,QMessageBox::Save|QMessageBox::Close) == QMessageBox::Save) simpleSave(ptr);
            simpleClose(ptr);
        }
        else{
            simpleClose(ptr);
        }
    }
}

QSize CodeWindow::sizeHint () const
{
    return hint ;
}



void CodeWindow::TextInsert(const QString &text)
{
    if( stackwidget->count() < 1) return ;
    currentedit->insert("\""+text+"\"");
}


void CodeWindow::ImportBeChange(const QString &text,int type)
{

}

//文件将被删除
void CodeWindow::FileWillBeDel(const QString &file)
{
    BkeDocBase *akb = docStrHash.value(LOLI_OS_QSTRING(file),0) ;
    if( akb == 0 ){
        return ;
    }
    else{
        stackwidget->setCurrentWidget(akb->edit);
        SetCurrentEdit(stackwidget->currentIndex());
        QMessageBox msg ;
        msg.setText("文件：\r\n"+akb->FullName()+" 已经被打开，是否关闭？");

        int i = msg.exec() ;
        if( i == 0) CloseFile();
        else if( i == 1 ) SaveAs();
        else if( i == 2 ) return ;
    }
}

void CodeWindow::FileIOclose(const QStringList &list)
{
    QStringList ls ;
    BkeDocBase *llm ;
    QList<BkeDocBase*> doclist ;
    for( int i = 0 ; i < list.size() ; i++){
        llm = docStrHash.value(list.at(i),0) ;
        //已经被打开的文件
        if( llm != 0 ){
            ls.append( list.at(i) );
            doclist.append( llm );
        }
    }


}


void CodeWindow::Compile()
{
    LableSureDialog abc(this) ;
    abc.SetLable("标记是通过在main.bkscr中的第一个@jump插入一个选项实现的，\r\n为保证标记正确启用，建议在main.bkscr中只进行变量初始化工作。");
    abc.SetCheckbox(QStringList()<<"运行"<<"启用标记");
    if( currentedit->hasSelectedText() ) abc.SetBtn(QStringList()<<"编译全部脚本"<<"编译当前脚本"<<"编译选择部分");
    else abc.SetBtn(QStringList()<<"编译全部脚本"<<"编译当前脚本");
    int i = abc.WaitUser(330,200) ;

    if( i == 0) CompileAll();
}


void CodeWindow::CompileAll()
{
    LOLI_CLEAR_TEMP(BKE_CURRENT_DIR+"/temp");
    //当前编辑项不是项目的话，什么也不会发生
    if( currentproject == 0 ) return ;

    //设置按钮
    btncompileact->setEnabled(false);
    btncompilerunact->setEnabled(false);
    btnrunact->setEnabled(false);
    //清理上次编译的项目
    deleteCompileFile(ComList);

    ComList = currentproject->AllScriptFiles() ;

    //拷贝文件
    if( !WriteOpenFile(currentproject->FileDir()) ){
        btncompileact->setEnabled(true);
        btncompilerunact->setEnabled(true);
        btnrunact->setEnabled(true);
        return ;
    }
    QStringList ls = ListDirsCopy(ComList,currentproject->FileDir(),BKE_CURRENT_DIR+"/temp") ;

    if( ls.size() > 0){
        QMessageBox msg ;
        msg.setText("以下文件复制失败：\r\n" + ls.join("\r\n") );
        msg.addButton(QMessageBox::Ok);
        msg.exec() ;
        return ;
    }

    kag->show();
    //开始编译
    comtool.Compile(BKE_CURRENT_DIR+"/temp");
}

bool CodeWindow::WriteOpenFile(const QString &dir)
{
    //总是不保存
    if( isCompileNotice == _NOTICE_NOSAVE ) return true;

    QList<BkeDocBase*> ls = docStrHash.values() ;
    QList<BkeDocBase*> ks ;
    QStringList ns ;

    for( int i = 0 ; i < ls.size() ; i++){
        if( LOLI_OS_QSTRING(ls.at(i)->ProjectDir()) != LOLI_OS_QSTRING(dir) ) continue ;
        if( !ls.at(i)->edit->isModified() ) continue ;
        ks.append(ls.at(i));
        ns.append( ls.at(i)->Name() );
    }

    if( ns.size() < 1) return true ;

    if( isCompileNotice == _NOTICE_ALWAYS ){

        LableSureDialog msg ;
        msg.SetLable("以下文件已经修改，是否保存？");
        msg.SetBtn(QStringList()<<"取消"<<"不要保存" << "全部保存");
        msg.SetCheckbox(QStringList()<<"不再提示");
        QListWidget *tls = new QListWidget(&msg) ;
        tls->addItems(ns);
        msg.SetCenterWidget(tls);
        msg.SetDefaultBtn(2);
        int k = msg.WaitUser() ;

        if( msg.IsCheckboxChoise(0) && k == 1) isCompileNotice = _NOTICE_NOSAVE ;
        if( msg.IsCheckboxChoise(0) && k == 2) isCompileNotice = _NOTICE_ALLSAVE ;

        if( k == 0) return false ;
        else if( k == 1) return true ;
        else if( k != 2 ) return true ;

    }

    for( int i = 0 ; i < ks.size() ; i++) simpleSave(ks.at(i));

    return true ;
}

//拷贝编译过的脚本
void CodeWindow::copyCompileFile(QStringList &list)
{
    int len = BKE_PROJECT_DIR.length() ;
    QFile abc ;
    QString temp ;
    for( int i = 0 ; i < list.size() ; i++){
        temp = list.at(i) ;
        temp = temp.right(temp.length()-len-1) ;
        if( temp.endsWith(".bkscr")){
            temp = temp.left(temp.length()-6) ;
            abc.setFileName(BKE_CURRENT_DIR+"/temp/"+temp+".bkbin");
            abc.copy(BKE_PROJECT_DIR+"/"+temp+".bkbin") ;
        }
        else if( temp.endsWith(".bkpsr") ){
            abc.setFileName(BKE_CURRENT_DIR+"/temp/"+temp);
            abc.copy(BKE_PROJECT_DIR+"/"+temp) ;
        }
    }
}

//删除编译过的文件
void CodeWindow::deleteCompileFile(QStringList &list)
{
    QFileInfo info ;
    QFile llm ;
    for( int i = 0 ; i < list.size() ; i++){
        info.setFile(list.at(i));
        if( info.suffix() == "bkscr" ){
            llm.setFileName(info.path()+"/"+info.baseName()+".bkbin");
            llm.remove() ;
        }
    }
}

bool CodeWindow::ReadyCompile(const QString &file)
{
    //不复制觉得路径
    if( file.indexOf("\\") >= 0) return true;
    int s = BKE_PROJECT_DIR.length() ;

    QFileInfo info( BKE_CURRENT_DIR + "/temp/" + file.right(file.length()- s - 1) ) ;
    QDir abc(info.path()) ;
    if( !abc.exists() ) abc.mkpath( info.path() ) ;  //创建路径

    BkeDocBase *ak = docStrHash.value( LOLI_OS_QSTRING(file),0 ) ;
    if( ak == 0 ){  //文件没有被打开，直接复制
        QFile llm(file) ;
        if( !llm.copy(info.filePath())) return false ;
        else return true ;
    }
    else{  //已经打开则写到文件
        return LOLI::AutoWrite(info.filePath(),ak->edit->text()) ;
    }

}


//编译完成
void CodeWindow::CompileFinish()
{
    btncompileact->setEnabled(true);
    kag->setValue(kag->maximum());

    ComText = comtool.Result() ;
    markadmin.ProblemsFromText(BKE_PROJECT_DIR,ComText);

    BkeMarkList *problemslist ;
    if( stackwidget->count() > 0){
        //给当前文件标记错误信息
        problemslist = markadmin.GetPrombleMark(currentbase->FullName(),true) ;
        CheckProblemMarks(currentedit,problemslist);
        currentedit->update();
    }
    else{
        problemslist = markadmin.GetPrombleMark("",true) ;
    }

    othwin->compileedit->setText(ComText);
    othwin->ShowProblem(problemslist);

    copyCompileFile(ComList);

    //按钮可用
    btncompileact->setEnabled(true);
    btncompilerunact->setEnabled(true);
    btnrunact->setEnabled(true);
    QTimer::singleShot(8*1000,kag,SLOT(reset())) ; //8秒之后隐藏
    if( markadmin.errorcount < 1 ) btnrunact->setEnabled(true) ;  //编译完成并且没有问题运行按钮才可用
    if( markadmin.errorcount < 1 && isRun ) RunBKE();
    isRun = false ;
}

void CodeWindow::CompileAndRun()
{
    isRun = true ;
    CompileAll();
}

void CodeWindow::FileNameChange(const QString &oldname,const QString &newname,bool &c)
{

}

//转到文件
void CodeWindow::ToLocation(BkeMarkerBase *p)
{
    BKEproject *pro = prowin->FindFileProject(p->FullName) ;
    if( pro == 0){
        addFile(p->FullName,0);
    }
    else  addFile(p->FullName,pro->FileDir());

    if( p->Atpos > 1) currentedit->setFirstVisibleLine(p->Atpos-1);
    else currentedit->setFirstVisibleLine(p->Atpos);
}

void CodeWindow::ShowRmenu( const QPoint & pos )
{
    QMenu menu(this) ;
    menu.addAction(btncopyact) ;
    menu.addAction(btncutact) ;
    menu.addAction(btnpasteact) ;
    menu.addSeparator() ;
    menu.addAction(btnfindact) ;
    menu.addAction(btnreplaceact) ;
    menu.addAction(btnfly) ;
    menu.addSeparator() ;
    menu.addAction(pannote) ;
    menu.addAction(btnbookmarkact) ;
    menu.addAction(btnmarkact) ;

    QPoint temp = QCursor::pos() ;
    temp.setX(temp.x() + 10);
    QAction *result = menu.exec(temp) ;

}

//添加书签
void CodeWindow::AddBookMark()
{
    QString info = QInputDialog::getText(this,"新建书签","输入书签的标记信息，\r\n如果为空，书签不会创建") ;
    if( info.isEmpty() ) return ;

    int line = currentedit->GetTrueCurrentLine() ;
    if( currentbase != 0 ){
        //markadmin.AddBookMark(info, line ,BkeFullnameToName(currentbase->fullname,currentproject->FileDir()) );
        currentproject->WriteMarkFile(&markadmin) ;
    }
    else{
        markadmin.AddBookMark(info, line ,"" );
    }

    currentedit->markerAdd(line,3) ;
    othwin->RefreshBookMark(markadmin.GetBookMark(currentbase->FullName(),true));
}

//更新所有标记信息
void CodeWindow::AddMarksToEdit()
{
    CheckBookMarks(currentedit,markadmin.GetBookMark(currentbase->FullName(),false));
    CheckMarks(currentedit,markadmin.GetMarks(currentbase->FullName(),false));
    CheckProblemMarks(currentedit,markadmin.GetPrombleMark(currentbase->FullName(),false));
    currentedit->update();
}

//标记更新完后使用 updata
void CodeWindow::CheckProblemMarks(BkeScintilla *edit,BkeMarkList *list)
{
    edit->clearAnnotations();
    edit->markerDeleteAll(1);
    edit->markerDeleteAll(2);

    if( list->isEmpty() ) return ;

    BkeMarkerBase *abc ;
    QString info ;

    for( int i = 0 ; i < list->size() ; i++){
        abc = list->at(i) ;
        info = edit->annotation(abc->Atpos-1) ;

        if( info.isEmpty() ) info = abc->Information ;
        else info += "\r\n" + LOLI_AUTONEXT_QSTRING(abc->Information) ;

        if( abc->Type == 1){
             edit->annotate(abc->Atpos-1,info,ks1);
             edit->markerAdd(abc->Atpos-1,1) ;
        }
        else{
            edit->annotate(abc->Atpos-1,info,ks2);
            edit->markerAdd(abc->Atpos-1,2) ;
        }
    }
}

void CodeWindow::CheckBookMarks(BkeScintilla *edit,BkeMarkList *list)
{
    edit->markerDeleteAll(3);

    if( list->isEmpty() ) return ;

    BkeMarkerBase *abc ;
    for( int i = 0 ; i < list->size() ; i++){
        abc = list->at(i) ;
        edit->markerAdd(abc->Atpos,3) ;
    }
}

void CodeWindow::CheckMarks(BkeScintilla *edit,BkeMarkList *list)
{
    edit->markerDeleteAll(4);

    if( list->isEmpty() ) return ;

    BkeMarkerBase *abc ;
    for( int i = 0 ; i < list->size() ; i++){
        abc = list->at(i) ;
        edit->markerAdd(abc->Atpos,4) ;
    }
}


//运行BKEngine.exe
void CodeWindow::RunBKE()
{
//    QString temp = BKE_CURRENT_DIR+"/ProjectCake/"+currentproject->ProjectName() ;
//    QDir cakedir ;
//    cakedir.mkpath(temp) ;

    QString ndir = BKE_CURRENT_DIR+"/tool/BKEngine_Dev.exe" ;
    QDir::setCurrent(currentproject->FileDir()) ;
    QProcess::startDetached( ndir,QStringList() ) ;
}

void CodeWindow::AnnotateSelect()
{
    currentedit->BkeAnnotateSelect();
}

void CodeWindow::ClearCompile()
{
    deleteCompileFile(ComList) ;
    btnrunact->setEnabled(false) ; //清理后运行按钮不可用
    btndebugact->setEnabled(false); //debug按钮也不可用
}

void CodeWindow::ChangeCodec()
{
    QMessageBox msg ;
    QComboBox akb(&msg) ;
    akb.setGeometry(65,65,118,24);
    akb.addItems(QStringList()<<"UTF-8"<<"Big5"<<"GB18030-0"<<"JShift-JIS"<<"UTF-16BE"<<"UTF-16LE"<<"UTF-32BE"<<"UTF-32LE");
    msg.setWindowTitle("选择原文件编码");
    msg.resize(240,300);
    msg.setText("当文件出现乱码时，请选择原文件编码，\r\n在选择正确的编码之前不要保存，\r\n否则会导致文件不可逆损坏！\r\n\r\n\r\n");
    msg.addButton(QMessageBox::Yes);
    if( msg.exec() == QMessageBox::Yes ){
            QFile *f = currentbase->File() ;
            if( !f->isOpen() && !f->open(QFile::ReadWrite) ) return ;
            f->seek(0) ;
            QByteArray temp = f->readAll() ;
            QTextCodec *codec = QTextCodec::codecForName(akb.currentText().toLatin1()) ;
            currentedit->setText(codec->toUnicode(temp));
    }
}



void CodeWindow::FileReadyToCompile(int i)
{
    if( i >= ComList.size()) return ;
    kag->setValue(i);
}

//当前项目被改变
void CodeWindow::ChangeProject(BKEproject *p)
{
    if( p == 0){
        btncompileact->setEnabled(false);  //编译按钮只有当项目出现时才可用
        btncompilerunact->setEnabled(false);
        btnrunact->setEnabled(false) ;
        btndebugact->setEnabled(false);
        currentproject = 0 ;
        return ;
    }

    currentproject = p ;
    btncompileact->setEnabled(true);  //项目出现后编译按钮都是可用的
    btncompilerunact->setEnabled(true);
}

void CodeWindow::TextToMarks(const QString &text,const QString &dir,int type)
{
    if( type == 0){
        markadmin.BookMarksFromText(text,dir);
    }
    othwin->RefreshBookMark(markadmin.GetBookMark("",true));
}

void CodeWindow::NewEmptyFile()
{
    BkeDocBase *llm = new BkeDocBase ;
    llm->SetFileName("New");
    simpleNew(llm,"");
    SetCurrentEdit(llm->edit);
}

void CodeWindow::SelectAll()
{
    if( stackwidget->count() < 1) return ;
    currentedit->selectAll();
}

void CodeWindow::QfileChange(const QString &path)
{
    //文件监视有bug，占时不启用。
    return ;
    if( watcherflag == 0){
        watcherflag = 1 ;
        return ;
    }

    if( !docStrHash.contains(LOLI_OS_QSTRING(path))) return ;
    addFile(path,"");

    if( !currentbase->File()->exists() ){
        QMessageBox msg ;
        msg.setText("文件：\r\n"+path+"\r\n已经被删除，是否关闭文件？");
        msg.addButton("关闭",QMessageBox::AcceptRole);
        msg.addButton("另存为..",QMessageBox::RejectRole);
        msg.addButton("取消",QMessageBox::DestructiveRole);
        int k = msg.exec() ;

        if( k == QMessageBox::AcceptRole ){
            filewatcher->removePath(path) ;
            CloseFile();
        }
        else if( k == QMessageBox::RejectRole ) SaveAs();
        else return ;
    }
    else{
        QMessageBox msg ;
        msg.setText("文件：\r\n"+path+"\r\n已经被改变，是否重新载入？");
        msg.addButton(QMessageBox::Yes);
        msg.addButton(QMessageBox::No);
        if( msg.exec() == QMessageBox::No ) return ;
        else{
            QString en ;
            LOLI::AutoRead(en,path) ;
            currentedit->setText(en);
            currentedit->setModified(false);
        }
    }
}

void CodeWindow::simpleClose(BkeDocBase *loli)
{
    int tpos = stackwidget->indexOf(loli->edit) ;
    if( tpos < 0) return ;

    ignoreflag = true ;
    lablelist->removeItem(tpos);
    filewidget->takeItem(tpos) ;
    ItemTextList.removeAt(tpos);
    stackwidget->removeWidget(loli->edit);
    docStrHash.remove( LOLI_OS_QSTRING(loli->FullName()) ) ;
    docWidgetHash.remove( loli->edit ) ;
    //移除文件监视
    //filewatcher->removePath(currentbase->FullName()) ;

    loli->edit->close() ;
    loli->edit->deleteLater();
    lablelist->setCurrentIndex(-1);
    delete loli ;
    ignoreflag = false ;


    if( stackwidget->count() < 1){
        btnDisable();
        currentedit = NULL ;
        filewatcher->removePaths(filewatcher->files()) ;
        DrawLine(false);
    }
    else{
        SetCurrentEdit(stackwidget->currentIndex());
    }

}

//跳转到指定行
void CodeWindow::GotoLine()
{ 
    if( stackwidget->count() < 1) return ;

    bool ok ;
    int i = QInputDialog::getInt(this,"跳转","输入行号",0,-1,999999,1,&ok) ;

    if( !ok ) return ;
    else if( i < 1) i = 1 ;
    else if( i > currentedit->lines() ) i = currentedit->lines() ;
    currentedit->setFirstVisibleLine( i -1  );
}

//后台搜索标签
void CodeWindow::BackstageSearchLable(BkeScintilla *edit)
{
    if( isSearLable ){  //正在运行，进入队列
        searchlablelater = edit ;
        return ;
    }

    edit->findFirst1("^[*].*",false,true,false,false) ;

    slablelist->clear();
    slablels.clear();
    WordSupport wow ;
    for( int i = 0 ; i < edit->testlist.size() ; i++){
        BkeIndicatorBase llm = edit->testlist.at(i) ;
        if( llm.IsNull() ) continue ;

        int xl,xi ;
        edit->lineIndexFromPosition(llm.Start(),&xl,&xi);
        wow.setText(edit->text(xl));
        if( wow.NextWord2() == "*" ){
            slablelist->addItem("*"+wow.NextWord2(),xl);
            slablels.append( "*"+wow.NextWord2() );
        }
    }

    if(searchlablelater != 0){
        BackstageSearchLable(searchlablelater);
        searchlablelater = 0 ;
    }
}

void CodeWindow::GotoLable(int i)
{
    currentedit->setFirstVisibleLine( slablelist->itemData(i).toInt()) ;

}

void CodeWindow::DrawLine(bool isClear)
{
    if( isClear ){
        stackwidget->setStyleSheet("");
    }
    else{

        stackwidget->setStyleSheet("QWidget{border-left:1px solid #313131;border-top:1px solid #313131 }");
    }
}