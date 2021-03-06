﻿#include <weh.h>
#include "projectwindow.h"
#include "dia/newprodia.h"
#include "BKS_info.h"
#include <QSize>
#include "dia/lablesuredialog.h"
#include "mainwindow.h"
#include <QApplication>
#include <QWindow>
#ifdef WIN32
#include <Windows.h>
#endif
#include <stdint.h>
#include "dia/ParserEditor.h"

//QList<BkeProject*> projectlist ;

extern uint32_t BKE_hash(const wchar_t *str);

ProjectWindow::ProjectWindow(QWidget *parent)
	:QTreeWidget(parent)
{
	setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT + "/projectlist").toString());
	setHeaderHidden(true);
	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(ItemDoubleClick(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowRmenu(QPoint)));
	connect(this, SIGNAL(CurrentProChange(BkeProject *)), (MainWindow*)parent, SLOT(CurrentProChange(BkeProject *)));

	QStringList ls = QString("编译脚本 发布游戏 插入路径 预览 新建脚本 新建脚本 添加文件 添加目录 导出剧本 在文件夹中显示 在这里搜索 移除 关闭工程 重命名").split(" ");
	for (int i = 0; i < BTN_COUNT; i++){
		btns[i] = new QAction(ls.at(i), this);
		connect(btns[i], SIGNAL(triggered()), this, SLOT(ActionAdmin()));
	}

	workpro = NULL;
}

ProjectWindow::~ProjectWindow()
{
	delete workpro;
}

void ProjectWindow::NewProject()
{
	NewProDia use(this);
	use.WaitUser();
	if (use.type<0)
	{
		return;
	}
	if (workpro)
	{
		global_bke_info.save();
		CloseProject();
	}
	workpro = new BkeProject();
	if (use.type == 0){
		workpro->NewProject(use.okdir, use.okname);
	}
	else if (use.type == 1){

	}

	//projectlist << pro ;
	addTopLevelItem(workpro->Root);
	BkeChangeCurrentProject();
	BkeCreator::AddRecentProject(workpro->FileDir() + "/" + BKE_PROJECT_NAME);
}

void ProjectWindow::OpenProject()
{
	QString name = QFileDialog::getOpenFileName(this, "打开工程", "", "BKE Creator工程(*.bkp)");
	OpenProject(name);
}

void ProjectWindow::OpenProject(const QString &file)
{
	if (file.isEmpty()) return;
	if (workpro && file == workpro->ProjectFile())
		return;
	CloseProject();
	QString path = QFileInfo(file).path();
	//for(auto it = projectlist.begin();it != projectlist.end();++it)
	//{
	//    if((*it)->FileDir()==path)
	//    {
	//        BkeChangeCurrentProject(*it);
	//        return;
	//    }
	//}

	workpro = new BkeProject;
	if (!workpro->OpenProject(file))
	{
		QMessageBox::information(this, "错误", "文件不存在，工程打开失败", QMessageBox::Ok);
		delete workpro;
		workpro = NULL;
		return;
	}
	//projectlist << pro ;
	addTopLevelItem(workpro->Root);
	BkeChangeCurrentProject();



	//    //读取书签
	//    QString text ;
	//    LOLI::AutoRead(text,pro->FileDir()+"/BkeProject.bmk") ;
	//    emit TextToMarks(text,pro->FileDir(),0);
	BkeCreator::AddRecentProject(workpro->FileDir() + "/" + BKE_PROJECT_NAME);
	//默认展开节点
	workpro->Root->setExpanded(true);
	workpro->Script->setExpanded(true);
	workpro->Import->setExpanded(true);

	QString t = "BKE Creator - " + file;

	MainWindow::getInstance()->setWindowTitle(t);
	//SetProp(GetActiveWindow(), L"title", (HANDLE)BKE_hash(t.toStdWString().c_str()));
	emit onProjectOpen(workpro);
}

//读取工程，并发送
bool ProjectWindow::ReadItemInfo(QTreeWidgetItem *dest, ItemInfo &f)
{
	if (dest == NULL) return false;
	BKE_PROJECT_READITEM(dest, f);
	return true;
}

//工程被双击
void ProjectWindow::ItemDoubleClick(QTreeWidgetItem * item, int column)
{
	if (!ReadItemInfo(item, info)) return;

	BkeProject *p = FindPro(info.ProName);
	QString name = p->FileDir() + info.FullName;

	if (info.FullName == "/config.bkpsr"){
		ConfigProject(p->config);
		return;
	}

	if (name.endsWith(".bkscr")){
		emit OpenThisFile(name, p->FileDir());
	}

	if (name.endsWith(".bkpsr")) {
		ParserEditor *edit = new ParserEditor(name);
		edit->load();
		QString error = edit->error();
		if (!error.isEmpty())
		{
			emit OpenThisFile(name, p->FileDir());
			edit->setParent(NULL);
			delete edit;
		}
		else 
			edit->show();
	}


}

//右键菜单
void ProjectWindow::ShowRmenu(const QPoint & pos)
{
	if (!ReadItemInfo(currentItem(), info)) return;

	if (info.Layer == 1 && info.Name == "config.bkpsr")
		return;

	QPoint pt = QCursor::pos();
	QMenu mn;

	if (info.Layer < 1){
		//mn.addAction(btns[btn_active]) ;
		mn.addAction(btns[btn_compile]);
		mn.addAction(btns[btn_release]);
	}
	else if (info.Layer > 1){
		mn.addAction(btns[btn_insertdir]);
		//mn.addAction(btns[btn_preview]);
	}

	mn.addSeparator();

	if (info.Layer == 1 || (info.Layer > 1 && info.IconKey == workpro->dirsico->cacheKey()))
	{
		auto info2 = info.getLayer1ItemInfo();
		if (info2.Name == "宏")
			mn.addAction(btns[btn_newimport]);
		if (info2.Name == "脚本")
			mn.addAction(btns[btn_newscript]);
	}

	for (int i = btn_addfile; i <= btn_search; i++){
		if(i == btn_exportscenario && (info.getLayer1ItemInfo().Name != "脚本" || !info.Name.endsWith(".bkscr"))) continue;
		mn.addAction(btns[i]);
		if (i == btn_adddir) mn.addSeparator();
		else if (i == btn_newimport) mn.addSeparator();
	}

	mn.addSeparator();
	if (info.Layer < 1){
		mn.addAction(btns[btn_close]);
	}
	else if (info.Layer > 1){
		mn.addAction(btns[btn_remove]);
		mn.addAction(btns[btn_rename]);
	}

	pt.setX(pt.x() + 10);
	mn.exec(pt);
	return;

}

//设置选中项
void ProjectWindow::SetCurrentItem(const QString &file)
{
	if (!workpro)
		return;
	//BkeProject *abc;
	QTreeWidgetItem *le = workpro->FindItemAll(file);
	if (le)
	{
		setCurrentItem(le);
	}
	//for (int i = 0; i < projectlist.size(); i++){
	//       abc = projectlist.at(i) ;
	//       le = abc->FindItem(abc->Import,file,false) ;
	//       if( le == 0 ) le = abc->FindItem(abc->Script,file,false ) ;
	//       if( le == 0 ) le = abc->FindItem(abc->Source,file,false ) ;
	//       if( le != 0 ){
	//           setCurrentItem(le);
	//           return ;
	//       }
	//   }
}

void ProjectWindow::NewFile(const ItemInfo &f, int type)
{
	QString name = QInputDialog::getText(this, "新建脚本", "输入脚本名称，如: \r\n   abc \r\n   abc/sence.bkscr");
	if (name.isEmpty())
		return;
	else if (!name.endsWith(".bkscr"))
		name.append(".bkscr");
	name = name.replace("\\", "/");

	bool ismacro = false;
	if (type == 1)
	{
		//macro
		//自动添加一些内容
		ismacro = true;
	}

	BkeProject *p = workpro;
	QFileInfo sk(p->FileDir() + info.getDir() + '/' + name);
	if (sk.exists()){
		int sk = QMessageBox::information(this, "", "文件已经存在，是否直接添加文件", QMessageBox::Yes | QMessageBox::No);
		if (sk == QMessageBox::No)
			return;
	}
	else
	{
		LOLI_MAKE_NULL_FILE(sk.filePath(), (ismacro ? "macro.bkscr" : ""));
	}

	//if( type == 1){
	//    p->FindItem(p->Import,name) ;
	//}
	//else if( type == 2){
	//    p->FindItem(info.Root,name) ;
	//}
	//else{
	//    p->FindItem(p->Source,name) ;
	//}
	p->Addfiles(QStringList() << name, info, true);
	//p->FindItem(info.Root, name);

	//p->AddFileToHash(p->typeHash(type), info.getDir().right(info.getDir().length() - 1) + '/' + name);
	p->WriteBkpFile();

}


//寻找工程
BkeProject *ProjectWindow::FindPro(const QString &proname)
{
	if (workpro && workpro->ProjectName() == proname)
		return workpro;
	//BkeProject *abc ;
	//for( int i = 0 ; i < projectlist.size() ; i++){
	//    abc = projectlist.at(i) ;
	//    if( abc->ProjectName() == proname){
	//        return abc ;
	//    }
	//}
	return 0;
}

QTreeWidgetItem *ProjectWindow::findFileInProject(const QString &name)
{
	//没有工程返回0
	if (!workpro) return 0;
	return workpro->FindItemAll(name);
	//temppro = workpro ;
	//QTreeWidgetItem *le = temppro->FindItemAll(name) ;
	//if( projectlist.size() == 1) return le ;

	//for( int i = 0 ; i < projectlist.size() ; i++){
	//    temppro = projectlist.at(i) ;
	//    if( temppro != workpro) le = temppro->FindItemAll(name) ;
	//    if( le != 0) break ;
	//}
	//return le ;
}

#undef DeleteFile
void ProjectWindow::DeleteFile(const ItemInfo &f)
{
	//是文件的话将询问是否移除文件
	BkeProject *p = FindPro(f.ProName);
	QString sk = p->FileDir() + f.FullName;
	LableSureDialog msg;
	msg.SetLable("要移除文件" + sk + "吗？");
	msg.SetCheckbox(QStringList() << "彻底删除");
	if (p->IconKey(f.IconKey) == "@dir") msg.SetCheckboxAble(0, false);
	msg.SetBtn(QStringList() << "移除" << "取消");
	if (msg.WaitUser(240, 130) == 1) return;

	if (msg.IsCheckboxChoise(0)){
		QFile(sk).remove();
	}
	//移除文件
	if (!p->RemoveItem(f)){
		QMessageBox::information(this, "错误", "移除工程过程中出了一个错误", QMessageBox::Ok);
		return;
	}
	p->files.remove(f.Name);
	//写出结果
	p->WriteBkpFile();
}

void ProjectWindow::Addfiles(const ItemInfo &f)
{
	BkeProject *p = FindPro(f.ProName);
	QStringList ls;
	if (f.RootName == "初始化" || f.RootName == "脚本"){
		ls = QFileDialog::getOpenFileNames(this, "添加文件", p->FileDir() + f.getDir(), "bkscr脚本(*.bkscr)");
	}
	else ls = QFileDialog::getOpenFileNames(this, "添加文件", p->FileDir() + f.getDir(), "所有文件(*.*)");

	if (ls.isEmpty()) return;

	if (f.Layer > 1 && !p->checkIsDir(f))
	{
		ReadItemInfo(f.Root->parent(), info);
	}

	QStringList errors;
	QStringList errors2;
	auto it = ls.begin();
	while (it != ls.end())
	{
		QString rfile = BkeFullnameToName(*it, p->FileDir() + f.getDir());
		if (p->checkFileExist(chopFileName(*it)))
		{
			errors2.append(*it);
		}
		if (rfile.isEmpty())
		{
			errors.append(*it);
			it = ls.erase(it);
		}
		else
		{
			*it = rfile;
			it++;
		}
	}
	if (!errors.isEmpty())
	{
		QMessageBox::warning(this, "警告", "不允许添加选中目录之外的文件：\n" + errors.join('\n'));
	}
	int res = 0;
	if (!errors2.isEmpty())
	{
		QMessageBox::warning(this, "警告", "以下文件由于工程里有同名文件故无法添加：\n" + errors2.join('\n'));
		res = QMessageBox::warning(this, "警告", "是否自动加上文件夹作为前缀？", QMessageBox::Yes, QMessageBox::No);
	}
	p->Addfiles(ls, f, res == QMessageBox::Yes);
}

//
void ProjectWindow::AddDir(const ItemInfo &f)
{
	BkeProject *p = FindPro(f.ProName);
	QString d = QFileDialog::getExistingDirectory(this, "添加目录", p->FileDir() + f.getDir());
	if (d.isEmpty())
		return;
	QString tmp = BkeFullnameToName(d, p->FileDir());
	if (tmp.isEmpty()){
		QMessageBox::information(this, "", "工作目录之外的文件不能添加", QMessageBox::Ok);
		return;
	}
	p->AddDir(d, tmp, f);
}

//void ProjectWindow::ReName()
//{
//    QString name = QInputDialog::getText(this,"重命名","将要改变到的名称，后缀名可以省略。不能含有\/:*?\"<>|") ;
//    if( name.isEmpty() ) return ;
//    else if( !name.endsWith(".bkscr")) name.append(".bkscr") ;
//
//    QString oldname = workpro->FileDir()+"/"+info.FullName ;
//    QString newname = oldname.left(oldname.lastIndexOf("/")) + "/" + name ;
//
//    QFile temp(newname) ;
//    if( temp.exists() ){
//        QMessageBox::information(this,"重命名","文件已存在，不能再使用这个名称。",QMessageBox::Ok) ;
//        return ;
//    }
//
//    bool result ;
//    emit FileNameChange(oldname,newname,result);
//    if( !result){
//        QMessageBox::warning(this,"重命名","重命名失败") ;
//        return ;
//    }
//
//    QTreeWidgetItem *le = currentItem() ;
//    le->setText(0,name);
//    workpro->WriteBkpFile() ;
//}


void ProjectWindow::OpenFile()
{
	QString filename = QFileDialog::getOpenFileName(this, "选择要打开的文件", "", "bksfile(*.bkscr);;Allfile(*.*)");
	if (filename.isNull()) return;

	QTreeWidgetItem *llm = findFileInProject(filename);
	if (llm == 0) emit OpenThisFile(filename, "");
	else OpenThisFile(filename, workpro->FileDir());
}

void ProjectWindow::BkeChangeCurrentProject(/*BkeProject *p*/)
{
	workpro->SetTopLeveBold(true);
	emit CurrentProChange(workpro);
	//if( workpro != 0){
	//    workpro->SetTopLeveBold(false);
	//}

	//if( p != 0){
	//    workpro = p ;
	//    workpro->SetTopLeveBold(true);
	//    emit CurrentProChange(workpro);
	//}
	//else{
	//    workpro = 0 ;
	//    emit CurrentProChange(workpro);
	//}
}

BkeProject *ProjectWindow::FindFileProject(const QString &file)
{
	if (findFileInProject(file) != 0) return workpro;
	else return 0;
}

BkeProject *ProjectWindow::FindProjectFromDir(const QString &dir)
{
	QString a = LOLI_OS_QSTRING(dir);
	if (workpro && a == LOLI_OS_QSTRING(workpro->FileDir()))
		return workpro;
	//for( int i = 0 ; i< projectlist.size() ; i++){
	//    if( a == LOLI_OS_QSTRING(projectlist.at(i)->FileDir()) ) return projectlist.at(i) ;
	//}
	return 0;
}

void ProjectWindow::ConfigProject(BkeProjectConfig *config)
{
	QBkeConfigDialog kag;
	kag.StartConfig(config);
}

void ProjectWindow::ActionAdmin()
{
	QAction *p = dynamic_cast<QAction*>(sender());

	if (p == NULL) return;
	//else if( p == btns[btn_active] ) Active(info);
	else if (p == btns[btn_compile]) emit Compile();
	else if (p == btns[btn_release]);
	else if (p == btns[btn_insertdir]) emit DirWillBeInsert(info.FullName);
	else if (p == btns[btn_preview]) PreviewFile(info);
	else if (p == btns[btn_newscript]) NewFile(info, 2);
	else if (p == btns[btn_newimport]) NewFile(info, 1);
	else if (p == btns[btn_addfile]) Addfiles(info);
	else if (p == btns[btn_adddir]) AddDir(info);
	else if (p == btns[btn_exportscenario]) ExportScenario(info);
	else if (p == btns[btn_showindir]) ShowInDir(info);
	else if (p == btns[btn_search]);
	else if (p == btns[btn_remove]) DeleteFile(info);
	else if (p == btns[btn_close]) CloseProject(info);
	else if (p == btns[btn_rename]) RenameFile(info);
}

void ProjectWindow::RenameFile(const ItemInfo &f)
{
	BkeProject *p = FindPro(f.ProName);
	if (f.Layer <= 1)
		return;
	if (!p->checkIsDir(f))
	{
		//file
		bool change;
		QString name = QInputDialog::getText(this, "重命名", "输入新名称(文件路径和文件类型不可更改)", QLineEdit::Normal, chopFileNameWithoutExt(f.FullName), &change);
		if (!change)
			return;
		QString ext = chopFileExt(f.Name);
		name = chopFileNameWithoutExt(name) + '.' + ext;
		if (name == ".bkscr")
			return;
		QString rawname = p->FileDir() + f.FullName;
		QString newname = p->FileDir() + f.Dirs + '/' + name;
		QFile temp(newname);
		if (temp.exists()){
			QMessageBox::information(this, "重命名", "文件已存在，不能再使用这个名称。", QMessageBox::Ok);
			return;
		}
		auto res = QFile(rawname).rename(newname);
		if (!res)
		{
			QMessageBox::warning(this, "重命名", "重命名失败");
			return;
		}
		f.Root->setText(0, name);
		p->files.remove(f.Name);
		p->files.insert(name, 1);
		if (newname.endsWith(".bkscr"))
		{
			emit FileNameChange(rawname, newname);
		}
	}
	else
	{
		QString name = QInputDialog::getText(this, "重命名", "输入新文件夹名(不可新建子文件夹)");
		name = chopFileName(name);
		QString rawname = p->FileDir() + f.FullName;
		QString newname = p->FileDir() + f.Dirs + '/' + name;
		QDir temp(newname);
		if (temp.exists()){
			QMessageBox::information(this, "重命名", "文件已存在，不能再使用这个名称。", QMessageBox::Ok);
			return;
		}
		auto res = temp.rename(rawname, newname);
		if (!res)
		{
			QMessageBox::warning(this, "重命名", "重命名失败");
			return;
		}
		f.Root->setText(0, name);
		p->config->removeScriptDir(f.FullName);
		p->config->addScriptDir(f.Dirs + '/' + name);
	}
	p->WriteBkpFile();
}

void ProjectWindow::PreviewFile(const ItemInfo &f)
{
	BkeProject *p = FindPro(f.ProName);
	QString n = p->FileDir() + "/" + info.FullName;
	if (n.endsWith(".bkscr") || n.endsWith(".bkpsr")) emit OpenThisFile(n, p->FileDir());
	else{
		QDesktopServices::openUrl(QUrl::fromLocalFile(n));
	}
}

void ProjectWindow::CloseProject()
{
	if (!workpro)
		return;
	//workpro->deleteLater();
	takeTopLevelItem(indexOfTopLevelItem(workpro->Root));
	//先closeALl脚本窗口
	emit onProjectClose();
	delete workpro;
	workpro = NULL;
	QString t = "BKE Creator";
	MainWindow::getInstance()->setWindowTitle(t);
}

void ProjectWindow::CloseProject(const ItemInfo &f)
{
	CloseProject();
	//BkeProject *p = FindPro(f.ProName);
	//if(p)
	//{
	//    projectlist.removeOne(p);
	//    p->deleteLater();
	//    takeTopLevelItem(indexOfTopLevelItem(p->Root));
	//    if(p == workpro)
	//    {
	//        p = NULL;
	//        if(!projectlist.empty())
	//        {
	//            p = projectlist.first();
	//        }
	//        BkeChangeCurrentProject(p);
	//    }
	//}
}

//void ProjectWindow::Active(const ItemInfo &f)
//{
//    BkeProject *p = FindPro(f.ProName);
//    if(p)
//    {
//        BkeChangeCurrentProject(p);
//    }
//}

#if defined(Q_OS_WIN)
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib,"shell32.lib")
#endif

void ProjectWindow::ShowInDir(const ItemInfo &f)
{
	BkeProject *p = FindPro(f.ProName);
	QString n = p->FileDir();
	if (info.Layer > 1)
		n += info.FullName;
	else
		n += '/';
#if defined(Q_OS_WIN)
	n.replace('/', '\\');
	QByteArray a = ("/select," + n).toLocal8Bit();
	ShellExecuteA(NULL, "open", "explorer.exe", a.data(), NULL, true);
#endif
}

void ProjectWindow::ReleaseGame(const ItemInfo &f)
{
	BkeProject *p = FindPro(f.ProName);
	p->ReleaseGame();
}

void ProjectWindow::ExportScenario(const ItemInfo &f)
{
	if (workpro)
	{
		QString text;
		QString path = workpro->FileDir() + f.FullName;
		if (LOLI::AutoRead(text, path))
		{
			text = CodeWindow::getScenarioTextFromCode(text).replace("\n\n","\n");
			path.truncate(path.lastIndexOf('.'));
			if (LOLI::AutoWrite(path + ".scenario.txt", text))
			{
				if (QMessageBox::question(NULL, "导出文本", "文本导出成功！是否打开导出目录？") == QMessageBox::Yes)
				{
					ShowInDir(f.getParentInfo());
				}
				return;
			}
		}
		QMessageBox::warning(NULL, "导出文本", "导出失败！");
	}
}

void ProjectWindow::ReleaseCurrentGame()
{
	if (workpro)
	{
		workpro->ReleaseGame();
	}
}