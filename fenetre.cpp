#include "fenetre.h"
#include "ui_fenetre.h"


fenetre::fenetre(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::fenetre)
{
    ui->setupUi(this);
    this->setWindowTitle("Correspondance STELA / CIVITAS");
    this->setWindowIcon(QIcon("icone.png"));
    connect(ui->btn_parcourir,SIGNAL(clicked()),this,SLOT(parcourir()));
    connect(ui->btn_envoyer,SIGNAL(clicked()),this,SLOT(envoyer()));

    pdftk = "bin\\pdftk.exe";
    pdftotext = "bin\\pdftotext.exe";
    sed = "bin\\sed.exe";
    zip = "bin\\zip.exe";
    grep = "bin\\grep.exe";

    logiciel = "civitas";
    QString paramXML = recupElementConfig("logiciel");
    if( paramXML != "")
        logiciel = paramXML;

    // verification du parametre : choix possibles -> CIVITAS / MAGNUS

    if( paramXML == "magnus" || paramXML == "civitas")
        this->setWindowTitle("Correspondance STELA / "+logiciel.toUpper());
    else if( paramXML != "" )
    {
        QMessageBox::information(this,"ERREUR","Il faut mettre en paramètre dans le fichier param.xml, soit la valeur MAGNUS, soit la valeur CIVITAS => le paramètre appliqué sera donc celui par défaut : CIVITAS");
        logiciel = "civitas";
    }


    this->setWindowTitle("Correspondance STELA / "+logiciel);
}

void fenetre::parcourir()
{
    fichier = QFileDialog::getOpenFileName(this,tr("Ouvrir le fichier PDF"), "C:\\Civitemp", tr("fichiers PDF (*.pdf)"));
    ui->input_fichier->setText(fichier);
}

void fenetre::envoyer()
{
    QString nomScript = "test.bat";

    // extraction des matricules dans les fichiers PDF
    QString cmd = "";
    system(QString(pdftk+" "+fichier+" burst").toLatin1().constData());     // -> lancer la commande PDFTK (decouper le pdf par pages)
    QStringList list(QDir(".").entryList());
    for (int i = 0; i < list.size(); ++i) {
        QString item = list.at(i);
        QString itemA="";
        if(item.contains("pdf"))
        {
            // -> lancer la commande PDFTOTEXT (mettre le contenu du PDF dans un TXT)
            if( logiciel == "civitas")
                cmd += pdftotext+" "+item+"\n";
            else if( logiciel == "magnus")
                cmd += pdftotext+" -raw "+item+"\n";

            item.replace(".pdf",".txt");
            itemA = item;
            itemA.replace(".txt","_a.txt");

            // -> lancer la commande SED ou GREP (récupérer le matricule dans le fichier TXT du bulletin)
            if( logiciel == "civitas")
                cmd += sed+" -n 20p "+item+" > "+itemA+"\n";
            else if( logiciel == "magnus")
                cmd += grep+" Matricule "+item+" > "+itemA+"\n";

            cmd += "del "+item+"\n";
            cmd += "rename "+itemA+" "+item+"\n";
        }
    }

    QFile *f = new QFile(nomScript,this);
    f->open(QIODevice::WriteOnly | QIODevice::Text);
    f->write(cmd.toLatin1().constData());
    f->close();

    system(QString(nomScript).toLatin1().constData());    // execution du script test.bat

    //QMessageBox::information(this,"test","test");

    // faire l'association "nom du fichier" => "matricule"
    QMap<QString, int> map;
    QStringList listTXT(QDir(".").entryList());
    for (int i = 0; i < listTXT.size(); ++i) {
        QString item = listTXT.at(i);
        if(item.contains(".txt"))
        {
            QFile file(item);
            file.open(QIODevice::ReadOnly);
            QString txt = file.readAll();
            if( logiciel == "magnus")
                txt = txt.replace("Matricule ","");
            //QMessageBox::information(this,"test - lecture fichier",txt);
            if(txt.toInt() > 0)
                map.insert(item,txt.toInt());
        }
    }

    // chercher le matricule maximum dans la liste
    int val=0, max=0;
    foreach(val, map.values())
        if(val>max)
            max=val;

    // chercher les fichiers dans la liste en fonction des numéros de matricule
    cmd = "";
    for(int i=0; i<=max; i++)
    {
        QList<QString>keys = map.keys(i);
        QString keysCh = "";
        if(keys.count() > 0) {          // si on trouve au moins un fichier, on le traite
            foreach(QString key,keys)
            {
                key.replace(".txt",".pdf");
                keysCh += key + " ";
            }
            cmd += pdftk + " " + keysCh + " output " + QString::number(i) + ".pdf\n";
        }
    }

    f = new QFile(nomScript,this);
    f->open(QIODevice::WriteOnly | QIODevice::Text);
    f->write(cmd.toLatin1().constData());
    f->close();

    system(QString(nomScript).toLatin1().constData());    // execution du script test.bat

    system(QString("del *.txt pg_*.pdf test.bat").toLatin1().constData());
    system(QString(zip+" 1.zip *.pdf").toLatin1().constData());
    system(QString("del *.pdf").toLatin1().constData());
    system(QString("move 1.zip "+QFileInfo(fichier).absolutePath()).toLatin1().constData());

    system(QString("explorer.exe "+QFileInfo(fichier).absolutePath().replace("/","\\")).toLatin1().constData());
}

int fenetre::TestIfDllAndExeExists()
{
    QStringList exe;
    exe << "bin\\pdftk.exe" << "bin\\pdftotext.exe" << "bin\\sed.exe" << "bin\\zip.exe" << "bin\\grep.exe" << "bin\\libiconv2.dll" << "bin\\zip32z64.dll" << "bin\\bzip2.dll";

    for (int i = 0; i < exe.size(); ++i) {
        QString item = exe.at(i).toLocal8Bit().constData();
        if(!QFile::exists(item)) {
            QMessageBox::critical(this,"Erreur","il manque l'executable/la dll "+item+" !");
            return false;
        }
    }

    return true;
}

QString fenetre::recupElementConfig(QString attributFichier) {
    QString valeur = "";
    QDomDocument doc("fenetre");
    QFile file("config.xml");
    if (file.open(QIODevice::ReadOnly)) {
        if (doc.setContent(&file)) {
            QDomElement root = doc.documentElement();
            QDomNodeList nl = root.elementsByTagName( attributFichier );
            if(nl.length()==1) {
                QDomNode noeud = nl.item(0);
                QDomElement element = noeud.toElement();
                valeur = element.attribute("valeur");
            }
        }
    }
    return valeur;
}

fenetre::~fenetre()
{
    delete ui;
}
