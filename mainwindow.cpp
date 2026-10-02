#include "mainwindow.h"

#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkProxyFactory>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

#define LOG_DIRECTORY "Logs"

#define greenSS                                                                \
  "background-color: rgba(0, 255, 0, 255); color: rgba(0,0,0,255);"
#define blueSS "background-color: rgba(0, 0, 255, 255);"
#define redSS "background-color: rgba(255, 0, 0, 255);"

namespace {
constexpr auto RESERVATION_URL =
    "https://www.korail.com/ticket/login";
    
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  initUi();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::initUi() {
  auto *profile = QWebEngineProfile::defaultProfile();
#if 0
    profile->setHttpUserAgent(
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/139.0.0.0 Safari/537.36"
        );
#endif
  m_webPage = new Page(profile, this);
  m_webView = new QWebEngineView(this);
  m_webView->setPage(m_webPage);

  ui->webLayout->addWidget(m_webView);
  auto s = m_webView->settings();
  const QWebEngineSettings::WebAttribute attrs_to_enable[] = {
      QWebEngineSettings::AutoLoadImages,
      QWebEngineSettings::JavascriptEnabled,
      QWebEngineSettings::JavascriptCanOpenWindows,
      QWebEngineSettings::JavascriptCanAccessClipboard,
      QWebEngineSettings::LinksIncludedInFocusChain,
      QWebEngineSettings::LocalStorageEnabled,
      QWebEngineSettings::LocalContentCanAccessRemoteUrls,
      QWebEngineSettings::XSSAuditingEnabled,
      QWebEngineSettings::SpatialNavigationEnabled,
      QWebEngineSettings::LocalContentCanAccessFileUrls,
      QWebEngineSettings::HyperlinkAuditingEnabled,
      QWebEngineSettings::ScrollAnimatorEnabled,
      QWebEngineSettings::ErrorPageEnabled,
      QWebEngineSettings::PluginsEnabled,
      QWebEngineSettings::FullScreenSupportEnabled,
      QWebEngineSettings::ScreenCaptureEnabled,
      QWebEngineSettings::WebGLEnabled,
      QWebEngineSettings::Accelerated2dCanvasEnabled,
      QWebEngineSettings::AutoLoadIconsForPage,
      QWebEngineSettings::TouchIconsEnabled,
      QWebEngineSettings::FocusOnNavigationEnabled,
      QWebEngineSettings::PrintElementBackgrounds,
      QWebEngineSettings::AllowRunningInsecureContent,
      QWebEngineSettings::AllowGeolocationOnInsecureOrigins,
      QWebEngineSettings::AllowWindowActivationFromJavaScript,
      QWebEngineSettings::ShowScrollBars,
      QWebEngineSettings::PlaybackRequiresUserGesture,
      QWebEngineSettings::WebRTCPublicInterfacesOnly,
      QWebEngineSettings::JavascriptCanPaste,
      QWebEngineSettings::DnsPrefetchEnabled,
      QWebEngineSettings::PdfViewerEnabled,
      QWebEngineSettings::NavigateOnDropEnabled,
      QWebEngineSettings::ReadingFromCanvasEnabled,
      QWebEngineSettings::ForceDarkMode,
      QWebEngineSettings::PrintHeaderAndFooter,
      QWebEngineSettings::PreferCSSMarginsForPrinting,
      QWebEngineSettings::TouchEventsApiEnabled,
      QWebEngineSettings::BackForwardCacheEnabled,
  };
  for (auto a : attrs_to_enable)
    s->setAttribute(a, true);

  profile->setHttpCacheType(QWebEngineProfile::NoCache);

  connect(m_webPage, &QWebEnginePage::loadStarted, this,
          &MainWindow::onLoadStarted);
  connect(m_webPage, &QWebEnginePage::loadFinished, this,
          &MainWindow::onLoadFinished);
  connect(m_webPage, &Page::popupCreated, this, &MainWindow::onPopupCreated);
  connect(this, &MainWindow::loadHtml, this, &MainWindow::onLoadHtml);

  ui->edUrl->setText(QString::fromLatin1(RESERVATION_URL));
  on_btnGo_clicked();
}

void MainWindow::onLoadStarted() {
  m_isLoading = true;
  qDebug()
      << "#################################################################";
  qDebug() << __FUNCTION__ << m_webPage->url().toString();
  ui->edUrl->setText(m_webPage->url().toString());
  ui->btnGo->setStyleSheet(redSS);
}

void MainWindow::onLoadFinished(bool ok) {
  m_isLoading = false;
  m_autoLoadInProgress = false;
  ui->btnGo->setStyleSheet(greenSS);
  if (!ok) {
    return;
  } else {
    ui->edUrl->setText(m_webPage->url().toString());
  }
  const QString path = m_webPage->url().path();
  m_webPage->toHtml([this](const QString &result) { emit this->loadHtml(result); });

  qDebug() << __FUNCTION__ << ok << m_webPage->url().toString();
}

void MainWindow::on_btnGo_clicked() {
  if (m_isLoading || m_autoLoadInProgress)
    return;

  m_autoLoadInProgress = true;
  const QUrl targetUrl = QUrl::fromUserInput(ui->edUrl->text());
  if (m_webPage->url() == targetUrl)
    m_webPage->triggerAction(QWebEnginePage::ReloadAndBypassCache);
  else
    m_webPage->setUrl(targetUrl);
}

void MainWindow::onLoadHtml(QString result) {

  if (!result.isEmpty()) {
    if (!QDir(LOG_DIRECTORY).exists())
      QDir().mkdir(LOG_DIRECTORY);

    QString path = LOG_DIRECTORY + QString("/") +
                   QDateTime::currentDateTime().toString("MMdd_hhmmss") +
                   "_.html";

    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Append)) {
      QTextStream out(&file);
      out << result;
      file.close();
    }
  }
}


void MainWindow::onPopupCreated(Page *popupPage) {
  connect(popupPage, &QWebEnginePage::loadFinished, this,
          [this, popupPage](bool ok) {
            if (!ok)
              return;
            ui->edUrl->setText(popupPage->url().toString());
          });
}

