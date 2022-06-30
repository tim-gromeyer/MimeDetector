#include "mainwindow.h"
#include "mimetypemodel.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QTreeView>

#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMimeDatabase>
#include <QMimeType>


MainWindow::MainWindow(QString f, QWidget *parent)
    : QMainWindow(parent)
    , model(new MimetypeModel(this))
    , treeView(new QTreeView(this))
    , detailsText(new QTextEdit(this))
    , findIndex(0)
{
    setupMenuBar();

    QSplitter *centralSplitter = new QSplitter(this);
    setCentralWidget(centralSplitter);
    treeView->setUniformRowHeights(true);
    treeView->setModel(model);

    QList<QStandardItem *> items = model->findItems("application/octet-stream", Qt::MatchContains | Qt::MatchFixedString | Qt::MatchRecursive);
    if (!items.isEmpty())
        treeView->expand(model->indexFromItem(items.constFirst()));

    connect(treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::currentChanged);
    centralSplitter->addWidget(treeView);
    detailsText->setReadOnly(true);
    centralSplitter->addWidget(detailsText);

    updateFindActions();
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *detectFileAction =
        fileMenu->addAction(tr("&Detect File Type"), this, &MainWindow::detectFile);
    detectFileAction->setShortcuts(QKeySequence::Open);

    QAction *exitAction = fileMenu->addAction(tr("E&xit"), qApp, &QApplication::quit);
    exitAction->setShortcuts(QKeySequence::Quit);

    QMenu *findMenu = menuBar()->addMenu(tr("&Edit"));
    QAction *findAction =
        findMenu->addAction(tr("&Find"), this, &MainWindow::find);
    findAction->setShortcuts(QKeySequence::Find);

    findNextAction = findMenu->addAction(tr("Find &Next"), this, &MainWindow::findNext);
    findNextAction->setShortcuts(QKeySequence::FindNext);
    findPreviousAction = findMenu->addAction(tr("Find &Previous"), this, &MainWindow::findPrevious);
    findPreviousAction->setShortcuts(QKeySequence::FindPrevious);

    menuBar()->addMenu(tr("&About"))->addAction(tr("&About Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::currentChanged(const QModelIndex &index)
{
    if (index.isValid())
        detailsText->setText(MimetypeModel::formatMimeTypeInfo(model->mimeType(index)));
    else
        detailsText->clear();
}

void MainWindow::selectAndGoTo(const QModelIndex &index)
{
    treeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
    treeView->setCurrentIndex(index);
}

void MainWindow::detectFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Choose File"));
    if (fileName.isEmpty())
        return;
    QMimeDatabase mimeDatabase;
    const QFileInfo fi(fileName);
    const QMimeType mimeType = mimeDatabase.mimeTypeForFile(fi);
    const QModelIndex index = mimeType.isValid()
        ? model->indexForMimeType(mimeType.name()) : QModelIndex();
    if (index.isValid()) {
        statusBar()->showMessage(tr("\"%1\" is of type \"%2\"").arg(fi.fileName(), mimeType.name()));
        selectAndGoTo(index);
    } else {
        QMessageBox::information(this, tr("Unknown File Type"),
                                 tr("The type of %1 could not be determined.")
                                 .arg(QDir::toNativeSeparators(fileName)));
    }
}

void MainWindow::updateFindActions()
{
    const bool findNextPreviousEnabled = findMatches.size() > 1;
    findNextAction->setEnabled(findNextPreviousEnabled);
    findPreviousAction->setEnabled(findNextPreviousEnabled);
}

void MainWindow::findPrevious()
{
    if (--findIndex < 0)
        findIndex = findMatches.size() - 1;
    if (findIndex >= 0)
        selectAndGoTo(findMatches.at(findIndex));
}

void MainWindow::findNext()
{
    if (++findIndex >= findMatches.size())
        findIndex = 0;
    if (findIndex < findMatches.size())
        selectAndGoTo(findMatches.at(findIndex));
}

void MainWindow::find()
{
    QInputDialog inputDialog(this);
    inputDialog.setWindowTitle(tr("Find"));
    inputDialog.setLabelText(tr("Text:"));
    if (inputDialog.exec() != QDialog::Accepted)
        return;
    const QString value = inputDialog.textValue().trimmed();
    if (value.isEmpty())
        return;

    findMatches.clear();
    findIndex = 0;
    const QList<QStandardItem *> items =
            model->findItems(value, Qt::MatchContains | Qt::MatchFixedString | Qt::MatchRecursive);
    for (const QStandardItem *item : items)
        findMatches.append(model->indexFromItem(item));
    statusBar()->showMessage(tr("%n mime types match \"%1\".", 0, findMatches.size()).arg(value));
    updateFindActions();
    if (!findMatches.isEmpty())
        selectAndGoTo(findMatches.constFirst());
}
