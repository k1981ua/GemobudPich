#ifndef FMDLG_H
#define FMDLG_H

#include <QDialog>
#include <QDirModel>
#include <QFileSystemModel>

class QListWidget;

namespace Ui {
class FMDlg;
}

bool copyFile(const QString &src, const QString &dst, bool move = false);

class FMDlg : public QDialog
{
    Q_OBJECT

  public:
    explicit FMDlg(const QString &title, const QString &localPath, QWidget *parent = 0);

    virtual ~FMDlg();

    void launch();

    void setLocalPath(const QString &localPath) { m_localPath = localPath; }

    void setFilter(const QStringList &filter) { m_filter = filter; }

  public slots:
    //void download();

    void upload();

    void remove();

    void previewReport();

    void treeExpanded();

    void treeClicked(QModelIndex modelIndex);

  private:
    void populatePanel(QListWidget *w, const QString &path);

    Ui::FMDlg *ui;
    QString m_usbPath, m_currentUsbPath, m_localPath;
    QStringList m_filter;

    QDirModel *model;
};

#endif // FMDLG_H
