#ifndef FENETRE_H
#define FENETRE_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QtXml/qdom.h>

namespace Ui {
class fenetre;
}

class fenetre : public QMainWindow
{
    Q_OBJECT
    
private slots:
    void parcourir();
    void envoyer();
public:
    explicit fenetre(QWidget *parent = 0);
    int TestIfDllAndExeExists();
    QString recupElementConfig(QString attributFichier);
    ~fenetre();
    
private:
    Ui::fenetre *ui;
    QString logiciel;
    QString fichier, pdftk, sed, pdftotext, zip, grep;
};

#endif // FENETRE_H
