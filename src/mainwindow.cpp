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
#include <QScreen>


MainWindow::MainWindow(const QString &f, QWidget *parent)
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

    QList<QStandardItem *> items = model->findItems(QStringLiteral("application/octet-stream"), Qt::MatchContains | Qt::MatchFixedString | Qt::MatchRecursive);
    if (!items.isEmpty())
        treeView->expand(model->indexFromItem(items.constFirst()));

    connect(treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::currentChanged);
    centralSplitter->addWidget(treeView);
    detailsText->setReadOnly(true);
    centralSplitter->addWidget(detailsText);

    updateFindActions();

    // Resize
    const QRect availableGeometry = QGuiApplication::screenAt(pos())->availableGeometry();
    resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
    move((availableGeometry.width() - width()) / 2,
         (availableGeometry.height() - height()) / 2);
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Detect File Type"), this, &MainWindow::onDetectFile, QKeySequence::Open);

    fileMenu->addAction(tr("E&xit"), qApp, &QApplication::quit, QKeySequence::Quit);

    QMenu *findMenu = menuBar()->addMenu(tr("&Edit"));
    findMenu->addAction(tr("&Find"), this, &MainWindow::find, QKeySequence::Find);

    findNextAction = findMenu->addAction(tr("Find &Next"), this, &MainWindow::findNext);
    findNextAction->setShortcuts(QKeySequence::FindNext);
    findPreviousAction = findMenu->addAction(tr("Find &Previous"), this, &MainWindow::findPrevious);
    findPreviousAction->setShortcuts(QKeySequence::FindPrevious);

    QMenu *menuHelp = menuBar()->addMenu(tr("&Help"));
    menuHelp->addAction(QIcon::fromTheme(QStringLiteral("help-about")), tr("&About"), this, &MainWindow::helpAbout);
    menuHelp->addAction(tr("&About Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::currentChanged(const QModelIndex &index)
{
    if (index.isValid())
        detailsText->setHtml(MimetypeModel::formatMimeTypeInfo(model->mimeType(index)));
    else
        detailsText->clear();
}

void MainWindow::selectAndGoTo(const QModelIndex &index)
{
    treeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
    treeView->setCurrentIndex(index);
}

void MainWindow::onDetectFile()
{
#ifdef Q_OS_WASM
    auto fileContentReady = [this](const QString &newFile, const QByteArray &fileContent) {
        if (!newFile.isEmpty()) {
            detectFile(newFile, fileContent);
        }
    };
    QFileDialog::getOpenFileContent(tr("Files (*.*)"), fileContentReady);
#else
    QFileDialog dialog(this, tr("Open MarkDown File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted) {
        const QString fileName = dialog.selectedFiles().at(0);
        if (fileName.isEmpty())
            return;

        QFile f(fileName);
        f.open(QIODevice::ReadOnly);

        detectFile(fileName, f.readAll());
    }
#endif
}

void MainWindow::detectFile(const QString &fileName, const QByteArray &fileContents)
{
    if (fileName.isEmpty())
        return onDetectFile();

    QMimeDatabase mimeDatabase;
    const QFileInfo fi(fileName);
    const QMimeType mimeType = mimeDatabase.mimeTypeForFileNameAndData(fileName, fileContents);
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

void MainWindow::helpAbout()
{
    QMessageBox::about(this, tr("About"), tr("<h2>MimeDetector</h2>\n"
                                             "<p>As the name suggests, MimeDetector is a simple program for recognizing the MIME type of files.</p>\n"
                                             "<h2>About</h2>\n"
                                             "<table class=\"table\" style=\"border-style: none;\">\n"
                                             "<tbody>\n"
                                             "<tr>\n"
                                             "<td>Version:</td>\n"
                                             "<td>%1</td>\n"
                                             "</tr>\n"
                                             "<tr>\n"
                                             "<td>Qt Version:</td>\n"
                                             "<td>%2</td>\n"
                                             "</tr>\n"
                                             "<tr>\n"
                                             "<td>Homepage:</td>\n"
                                             "<td><a href=\"https://software-made-easy.github.io/MimeDetector/\">https://software-made-easy.github.io/MimeDetector/</a></td>\n"
                                             "</tr>\n"
                                             "</tbody>\n"
                                             "</table>"
                                             ).arg(APP_VERSION, qVersion()));
}
