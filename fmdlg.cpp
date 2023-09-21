#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QTextDocument>
#include <QDesktopServices>

#include "fmdlg.h"
#include "ui_fmdlg.h"

//======================================================================================
bool copyFile(const QString &src, const QString &dst, bool move)
{
  QString cmd = "cp";
  QStringList args;
  args << src << dst;
  if (QProcess::execute(cmd, args) != 0)
    return false;

  if (move)
    return QFile::remove(src);

  return true;
}
//======================================================================================
FMDlg::FMDlg(const QString &title, const QString &localPath, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FMDlg)
{
  ui->setupUi(this);

  ui->fm->setTitle(title);


  m_usbPath = "/media/pi/";
  m_localPath=localPath;//qApp->applicationDirPath()+"/reports/";



  //connect(ui->download, SIGNAL(clicked()), SLOT(download()));
  connect(ui->upload, SIGNAL(clicked()), SLOT(upload()));
  connect(ui->remove, SIGNAL(clicked()), SLOT(remove()));
  connect(ui->preview, SIGNAL(clicked()), SLOT(previewReport()));

  connect(ui->treeView,SIGNAL(clicked(QModelIndex)),this,SLOT(treeClicked(QModelIndex)));
  connect(ui->treeView,SIGNAL(expanded(QModelIndex)),this,SLOT(treeExpanded()));
/*
  int button_height = mcfg.value<int>("screen_kbd/button_height");
  ui->download->setFixedSize(button_height, button_height);
  ui->upload->setFixedSize(button_height, button_height);
  ui->remove->setFixedSize(button_height, button_height);
  ui->preview->setFixedSize(button_height, button_height);
  ui->close->setMinimumHeight(button_height);
*/

  //QStringList filters;
  //filters << "*.odf";
  model=new QDirModel(this);

  model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  //model->setReadOnly(true);
  //model->setNameFilters(filters);
  //model->setRootPath(m_usbPath);
  ui->treeView->setModel(model);

  ui->treeView->setRootIndex(model->index(m_usbPath));
  ui->treeView->resizeColumnToContents(0);
  ui->treeView->hideColumn(1);
  ui->treeView->hideColumn(2);
  ui->treeView->hideColumn(3);
  ui->treeView->setHeaderHidden(true);

}
//=====================================================================================
void FMDlg::treeClicked(QModelIndex modelIndex)
{
  m_currentUsbPath=model->filePath(modelIndex);
    populatePanel(ui->usbPanel, m_currentUsbPath);
}
//=====================================================================================
void FMDlg::treeExpanded()
{
  ui->treeView->resizeColumnToContents(0);
}
//=====================================================================================
FMDlg::~FMDlg()
{
  delete ui;
}
//=========================================================================================
void FMDlg::launch()
{
  populatePanel(ui->usbPanel, m_usbPath);
  populatePanel(ui->localPanel, m_localPath);

  exec();
}

//==========================================================================================
  /*
void FMDlg::download()
{

  QListWidgetItem *i = ui->usbPanel->currentItem();

  if (!i) {
    QMessageBox::critical(this, tr("Error"), tr("Please choose a file on USB drive!"));
    return;
  }

  if (!copyFile(i->data(Qt::UserRole).toString(), m_localPath)) {
    QMessageBox::critical(this, tr("Error"), tr("Error copying file!"));
    return;
  }

  /// проверяем, есть ли уже в локальной папке файл с таким именем
  QList<QListWidgetItem *> tmp = ui->localPanel->findItems(i->text(), Qt::MatchExactly);
  QListWidgetItem *j = tmp.isEmpty() ? new QListWidgetItem(i->text(), ui->localPanel)
                                     : tmp.first();
  j->setData(Qt::UserRole, m_localPath + '/' + QFileInfo(i->data(Qt::UserRole).toString()).fileName());

  qDebug().nospace() << "FM: download(" << i->data(Qt::UserRole).toString()
                     << ", " << j->data(Qt::UserRole).toString() << ")";

}
  */
//==================================================================================================
void FMDlg::upload()
{
  QListWidgetItem *i = ui->localPanel->currentItem();

  if (!i) {
    QMessageBox::critical(this, tr("Error"), "Оберіть файл для копіювання на лівій панелі!");
    return;
  }



  if (!copyFile(i->data(Qt::UserRole).toString(), m_currentUsbPath)) {
    QMessageBox::critical(this, tr("Error"), "Помилка копіювання файлу.\n\nОберіть папку на правій панелі.\nКопіювання можливе тільки на USB-накопичувачі\nвідформатовані в FAT32! ");
    return;
  }

  /// проверяем, есть ли уже на флешке папке файл с таким именем
  QList<QListWidgetItem *> tmp = ui->usbPanel->findItems(i->text(), Qt::MatchExactly);
  QListWidgetItem *j = tmp.isEmpty() ? new QListWidgetItem(i->text(), ui->usbPanel)
                                     : tmp.first();
  j->setData(Qt::UserRole, m_usbPath + '/' + QFileInfo(i->data(Qt::UserRole).toString()).fileName());

  qDebug().nospace() << "FM: upload(" << i->data(Qt::UserRole).toString()
                     << ", " << j->data(Qt::UserRole).toString() << ")";
}
//===============================================================================================
void FMDlg::remove()
{

  if (ui->localPanel->currentItem() == NULL) {
    QMessageBox::critical(this, tr("Error"), "Оберіть файл для видалення");
    return;
  }

  if (QMessageBox::question(NULL,"Видалення файлу","Видалити файл " + ui->localPanel->currentItem()->text() + "?",QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes)
  {

      if (!QFile::remove(ui->localPanel->currentItem()->data(Qt::UserRole).toString())) {
        QMessageBox::critical(this, tr("Error"), "Помилка видалення файлу!");
        return;
      }

      qDebug().nospace() << "FM: remove(" << ui->localPanel->currentItem()->data(Qt::UserRole).toString() << ")";

      populatePanel(ui->localPanel,m_localPath);
  }
}
//================================================================================================
void FMDlg::previewReport()
{


  if (ui->localPanel->currentItem() == NULL) {
    QMessageBox::critical(this, tr("Error"), "Оберіть звіт!");
    return;
  }

  QString fileName = ui->localPanel->currentItem()->data(Qt::UserRole).toString();

  //QMessageBox::information(NULL,"title",fileName);

  QDesktopServices::openUrl(QUrl(fileName));

  /*
  QFile file(QString("./reports/preview_cache/%1.html").arg(fileName));
  if (file.open(QIODevice::ReadOnly)) {
    QString html = QString::fromUtf8(file.readAll());
    QTextDocument document;
    document.setHtml(html);
    ReportPreviewDlg preview_dlg(&document, this);
    preview_dlg.exec();
  }
*/
}
//==============================================================================================
void FMDlg::populatePanel(QListWidget *w, const QString &path)
{
   w->clear();
    QDir dir(path);
  QFileInfoList files = dir.entryInfoList(m_filter, QDir::Files, QDir::Name);
  foreach (QFileInfo f, files) {
    QListWidgetItem *i = new QListWidgetItem();
    i->setText(f.fileName());
    i->setData(Qt::UserRole, f.absoluteFilePath());
    w->addItem(i);
  }
  if (w==ui->localPanel) ui->localPanel->sortItems(Qt::DescendingOrder);
}
//===============================================================================================
