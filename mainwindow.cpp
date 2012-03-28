/*
    mainwindow.cpp
    Copyright (C) 2012 Mikhalevich Yurij <count@ypsilon.me>
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <QShortcut>
#include <QSpacerItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDate>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->do_this, SIGNAL(clicked()), this, SLOT(drawCouples()));
    connect(ui->saveCouplesToFileButton, SIGNAL(clicked()),
            this, SLOT(saveCouplesToFile()));
    //there is a dirty hack below
    QGridLayout *layout = static_cast <QGridLayout *> (ui->list->layout());
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Maximum,
                                    QSizePolicy::MinimumExpanding),
                    999, 0, 1, 3);
    //end of dirty hack
    //never do like above
    lli = 0;
    qsrand(QTime::currentTime().msec());
    addTeam();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addTeam()
{
    int tc = teams.count();
    if (tc) {
        teams.last()->button->disconnect();
        teams.last()->button->setIcon(QIcon(":/images/images/remove.png"));
        connect(teams.last()->button, SIGNAL(clicked()),
                this, SLOT(removeTeam()));
    }
    team *n = new team;
    n->number = new QLabel;
    n->number->setText(QString::number(teams.count() + 1) + ")");
    n->name = new QLineEdit;
    n->button = new QToolButton;
    teams.append(n);
    QGridLayout *layout = static_cast <QGridLayout *> (ui->list->layout());
    layout->addWidget(n->number, lli, 0);
    connect(n->name, SIGNAL(returnPressed()), this, SLOT(drawCouples()));
    QShortcut *sh = new QShortcut(QKeySequence("Ctrl+Return"), n->name);
    sh->setContext(Qt::WidgetShortcut);
    connect(sh, SIGNAL(activated()), this, SLOT(ctrlEnterPressed()));
    QShortcut *sh1 = new QShortcut(QKeySequence("Tab"), n->name);
    sh1->setContext(Qt::WidgetShortcut);
    connect(sh1, SIGNAL(activated()), this, SLOT(tabPressed()));
    QShortcut *sh2 = new QShortcut(QKeySequence("Shift+Tab"), n->name);
    sh2->setContext(Qt::WidgetShortcut);
    connect(sh2, SIGNAL(activated()), this, SLOT(shiftTabPressed()));
    layout->addWidget(n->name, lli, 1);
    n->button->setIcon(QIcon(":/images/images/add.png"));
    connect(n->button, SIGNAL(clicked()), this, SLOT(addTeam()));
    layout->addWidget(n->button, lli, 2);
    ++lli;
    n->name->setFocus();
}

void MainWindow::tabPressed()
{
    QShortcut *sh = static_cast <QShortcut *> (sender());
    QLineEdit *le = static_cast <QLineEdit *> (sh->parent());
    int tc = teams.count();
    for (int i = 0; i < (tc - 1); ++i) {
        if (teams.at(i)->name == le) {
            teams.at(i + 1)->name->setFocus();
            return;
        }
    }
    teams.last()->button->setFocus();
}

void MainWindow::shiftTabPressed()
{
    QShortcut *sh = static_cast <QShortcut *> (sender());
    QLineEdit *le = static_cast <QLineEdit *> (sh->parent());
    int tc = teams.count();
    for (int i = 0; i < tc; ++i) {
        if (teams.at(i)->name == le) {
            if (i)
                teams.at(i - 1)->name->setFocus();
            return;
        }
    }
}

void MainWindow::ctrlEnterPressed()
{
    QShortcut *sh = static_cast <QShortcut *> (sender());
    QLineEdit *le = static_cast <QLineEdit *> (sh->parent());
    int tc = teams.count();
    for (int i = 0; i < (tc - 1); ++i) {
        if (teams.at(i)->name == le) {
            teams.at(i + 1)->name->setFocus();
            return;
        }
    }
    addTeam();
}

void MainWindow::removeTeam()
{
    QToolButton *bt = static_cast <QToolButton *> (sender());
    int tc = teams.count();
    for (int i = 0; i < tc; ++i) {
        if (teams.at(i)->button == bt) {
            team *t = teams.at(i);
            t->button->deleteLater();
            t->name->deleteLater();
            t->number->deleteLater();
            delete t;
            teams.removeAt(i);
            if (i == (tc - 1)) {
                teams.last()->button->disconnect();
                teams.last()->button->setIcon(QIcon(":/images/images/add.png"));
                connect(teams.last()->button, SIGNAL(clicked()),
                        this, SLOT(addTeam()));
            } else {
                for (int j = i; j < (tc - 1); ++j) {
                    teams.at(j)->number->setText(QString::number(j + 1) + ")");
                }
            }
            return;
        }
    }
}

void MainWindow::drawCouples()
{
    ui->couples->clear();
    int tc = teams.count();
    QList <int> numbers;
    for (int i = 0; i < tc; ++i)
        numbers.append(i);
    bool left = true;
    while (numbers.count() != 0) {
        int curi = qrand() % numbers.count();
        int cur = numbers.at(curi);
        numbers.removeAt(curi);
        if (left) {
            ui->couples->appendPlainText(
                        teams.at(cur)->number->text()
                        + " " + teams.at(cur)->name->text().trimmed());
            left = false;
        } else {
            ui->couples->insertPlainText(
                        " <-> " + teams.at(cur)->number->text()
                        + " " + teams.at(cur)->name->text().trimmed());
            left = true;
        }
    }
}

void MainWindow::on_action_triggered()
{
    QMessageBox::about(this, tr("О программе"),
                             tr("<p>Жеребьёвка команд для "
                                "игры в «Ворошиловский стрелок» "
                                "разработана Михалевич Юрием "
                                "(<a href=\"mailto:count@ypsilon.me\">count@yps"
                                "ilon.me</a>)"
                                " в рамках проекта по созданию прикладного "
                                "ПО для игроков, авторов вопросов, "
                                "организаторов ЧГК-турниров. За тестирование и "
                                "содействие разработке благодарю Иваницкого Дми"
                                "трия.</p>"
                                "<p>Распространяется на условиях GNU General Pu"
                                "blic License третьей редакции.</p>"
                                "<p>Copyright (C) 2012 Mikhalevich Yurij &#060;"
                                "count@ypsilon.me&#062;<br/>"
                                "This program is free software: you can redistr"
                                "ibute it and/or modify it under the terms of t"
                                "he GNU General Public License as published by "
                                "the Free Software Foundation, either version 3"
                                " of the License, or (at your option) any later"
                                " version.<br/>"
                                "This program is distributed in the hope that i"
                                "t will be useful, but WITHOUT ANY WARRANTY; wi"
                                "thout even the implied warranty of MERCHANTABI"
                                "LITY or FITNESS FOR A PARTICULAR PURPOSE. See "
                                "the GNU General Public License for more detail"
                                "s.<br/>"
                                "You should have received a copy of the GNU Gen"
                                "eral Public License along with this program. I"
                                "f not, see &#060;http://www.gnu.org/licenses/"
                                "&#062;.</p>"
                                "<p>При поддержке сообщества «Ворошиловский стр"
                                "елок – Краснодар» — <a href=\"http://vk.com/vs"
                                "_krasnodar\">http://vk.com/vs_krasnodar</a>.</"
                                "p>"
                                "<p><img src=\":/images/images/shooter.jpeg\" h"
                                "eight=200/></p>"));
}

void MainWindow::saveCouplesToFile()
{
    QDate date = QDate::currentDate();
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Введите имя файла для "
                                                   "сохранения"),
                                                QDir::homePath()
                                                + QDir::separator()
                                                + "draw_"
                                                + QString::number(date.year())
                                                + "."
                                                + QString::number(date.month())
                                                + "."
                                                + QString::number(date.day())
                                                + "_.txt",
                                                tr("Текстовые файлы (*.txt)"));
    if (path.length()) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::warning(this, tr("Ошибка"),
                                 tr("Невозможно открыть файл для записи."));
        }
        QTextStream stream(&file);
#ifdef Q_OS_UNIX
        stream << ui->couples->toPlainText();
#endif
#ifdef Q_OS_WIN
        stream << ui->couples->toPlainText().replace("\n", "\r\n");
#endif
        file.close();
    }
}

void MainWindow::on_hotkeys_triggered()
{
    QMessageBox::information(this, tr("Горячие клавиши"),
                             tr("Для быстрого ввода списка команд вы можете исп"
                                "ользовать горячие клавиши:\nCtrl+Enter – перей"
                                "ти к вводу следующей команды, если необходимо,"
                                " создаст новую.\nShift+Tab и Tab – позволяют"
                                " быстро перемещаться вверх и вниз по списку "
                                "команд.\nEnter (при вводе команд) – инициирует"
                                " жеребьёвку."));
}
