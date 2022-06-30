#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndexList>

QT_BEGIN_NAMESPACE
class QAction;
class QTextEdit;
class QTreeView;
class QSettings;
QT_END_NAMESPACE


class MimetypeModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &f = QLatin1String(), QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void currentChanged(const QModelIndex &);
    void onDetectFile();

    void find();
    void findNext();
    void findPrevious();

    void setupMenuBar();

    void helpAbout();

private:
    void selectAndGoTo(const QModelIndex &index);
    void updateFindActions();

    void detectFile(const QString &f = QLatin1String());

    void loadSettings();
    void saveSettings();

    MimetypeModel *model;
    QTreeView *treeView;
    QTextEdit *detailsText;
    QAction *findNextAction;
    QAction *findPreviousAction;
    QModelIndexList findMatches;
    int findIndex;

    QSettings *settings;
};

#endif // MAINWINDOW_H
