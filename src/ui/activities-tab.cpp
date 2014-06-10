#include <cstdio>
#include <QtGui>
#include <QIcon>
#include <QNetworkRequest>
#include <QStackedWidget>
#include <QWebFrame>
#include <QSslError>

#include "seafile-applet.h"
#include "account-mgr.h"
#include "events-list-view.h"
#include "utils/widget-utils.h"
#include "events-service.h"

#include "activities-tab.h"

namespace {

const int kRefreshInterval = 1000 * 60 * 5; // 5 min
const char *kLoadingFaieldLabelName = "loadingFailedText";
const char *kEmptyViewLabelName = "emptyText";
const char *kAuthHeader = "Authorization";
const char *kActivitiesUrl = "/api2/html/events/";

enum {
    INDEX_LOADING_VIEW = 0,
    INDEX_LOADING_FAILED_VIEW,
    INDEX_EVENTS_VIEW,
};


}


ActivitiesTab::ActivitiesTab(QWidget *parent)
    : TabView(parent)
{
    createEventsView();
    createLoadingView();
    createLoadingFailedView();

    mStack->insertWidget(INDEX_LOADING_VIEW, loading_view_);
    mStack->insertWidget(INDEX_LOADING_FAILED_VIEW, loading_failed_view_);
    mStack->insertWidget(INDEX_EVENTS_VIEW, events_container_view_);

    connect(EventsService::instance(), SIGNAL(refreshSuccess(const std::vector<SeafEvent>&, bool, bool)),
            this, SLOT(refreshEvents(const std::vector<SeafEvent>&, bool, bool)));

    refresh();
}

void ActivitiesTab::loadMoreEvents()
{
    EventsService::instance()->loadMore();
    load_more_btn_->setVisible(false);
    events_loading_view_->setVisible(true);
}

void ActivitiesTab::refreshEvents(const std::vector<SeafEvent>& events,
                                  bool is_loading_more,
                                  bool has_more)
{
    mStack->setCurrentIndex(INDEX_EVENTS_VIEW);

    events_loading_view_->setVisible(false);
    load_more_btn_->setVisible(has_more);
        
    events_list_view_->updateEvents(events, is_loading_more);
}

void ActivitiesTab::refresh()
{
    showLoadingView();

    EventsService::instance()->refresh(true);
}

void ActivitiesTab::createEventsView()
{
    events_container_view_ = new QWidget;
    events_container_view_->setObjectName("EventsContainerView");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    events_container_view_->setLayout(layout);

    events_list_view_ = new EventsListView;
    layout->addWidget(events_list_view_);

    load_more_btn_ = new QToolButton;
    load_more_btn_->setText(tr("More"));
    load_more_btn_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    connect(load_more_btn_, SIGNAL(clicked()),
            this, SLOT(loadMoreEvents()));
    load_more_btn_->setVisible(false);
    layout->addWidget(load_more_btn_);
    
    events_loading_view_ = newLoadingView();
    events_loading_view_->setVisible(false);
    layout->addWidget(events_loading_view_);
}

void ActivitiesTab::createLoadingView()
{
    loading_view_ = ::newLoadingView();
}

void ActivitiesTab::createLoadingFailedView()
{
    loading_failed_view_ = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout;
    loading_failed_view_->setLayout(layout);

    QLabel *label = new QLabel;
    label->setObjectName(kLoadingFaieldLabelName);
    QString link = QString("<a style=\"color:#777\" href=\"#\">%1</a>").arg(tr("retry"));
    QString label_text = tr("Failed to get actvities information<br/>"
                            "Please %1").arg(link);
    label->setText(label_text);
    label->setAlignment(Qt::AlignCenter);

    connect(label, SIGNAL(linkActivated(const QString&)),
            this, SLOT(refresh()));

    layout->addWidget(label);
}

void ActivitiesTab::showLoadingView()
{
    mStack->setCurrentIndex(INDEX_LOADING_VIEW);
}
