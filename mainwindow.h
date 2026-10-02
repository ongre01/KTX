#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include <QWebEngineCertificateError>
#include <QWebEnginePage>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineView>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Page : public QWebEnginePage {
  Q_OBJECT

signals:
  void popupCreated(Page *popupPage);

public:
  using QWebEnginePage::QWebEnginePage;

  // 팝업/SSO 로그인 창 처리(중요)
  QWebEnginePage *createWindow(WebWindowType) override {
    qDebug() << __FUNCTION__;
    auto *v = new QWebEngineView;
    auto *p = new Page(profile());
    v->setPage(p);
    v->resize(1000, 800); // 기본 크기
    v->show();
    emit popupCreated(p);
    return p;
  }
};

typedef enum {
  NONE_TYPE,
  ON_TYPE,
  OFF_TYPE,
} MacroTypeDef;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

signals:
  void loadHtml(QString result);

private slots:
  void onLoadStarted();
  void onLoadFinished(bool ok);
  void onLoadHtml(QString result);

  void on_btnGo_clicked();
  void onPopupCreated(Page *popupPage);


private:
  void initUi();

  Ui::MainWindow *ui;

  Page *m_webPage = nullptr;
  QWebEngineView *m_webView = nullptr;
  QWebEngineDownloadRequest *pDownloadReq = nullptr;

  bool m_isLoading = false;
  bool m_autoLoadInProgress = false;

};
#endif // MAINWINDOW_H
