#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndexList>

QT_BEGIN_NAMESPACE
class QAction;
class QTextEdit;
class QTreeView;
QT_END_NAMESPACE


class MimetypeModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QString f = QLatin1String(), QWidget *parent = nullptr);

private slots:
    void currentChanged(const QModelIndex &);
    void detectFile();
    void find();
    void findNext();
    void findPrevious();

    void setupMenuBar();

private:
    void selectAndGoTo(const QModelIndex &index);
    void updateFindActions();

    MimetypeModel *model;
    QTreeView *treeView;
    QTextEdit *detailsText;
    QAction *findNextAction;
    QAction *findPreviousAction;
    QModelIndexList findMatches;
    int findIndex;
};

#endif // MAINWINDOW_H
