#ifndef BKECOMPILE_H
#define BKECOMPILE_H

#include <QObject>
#include <QProcess>

class BkeCompile : public QObject
{
    Q_OBJECT
public:
    explicit BkeCompile(QObject *parent = 0);

    void Compile(const QString dir) ;
    QString Result()  ;

signals:
    void CompliteFinish() ;
    void NewFileReady(int i) ;

public slots:
    void StandardOutput() ;
    void finished(int exitCode ) ;
private:
    QByteArray result ;
    QProcess *cmd ;
    QString text ;
    QTextCodec *codec ;
    QStringList list ;

};

#endif // BKECOMPILE_H