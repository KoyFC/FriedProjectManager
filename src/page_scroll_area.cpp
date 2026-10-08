#include "page_scroll_area.h"

#include <QEvent>

#include <algorithm>

PageScrollArea::PageScrollArea(QWidget *page)
{
    setWidget(page);
    setFrameShape(QFrame::NoFrame);
    page->installEventFilter(this);
}

void PageScrollArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    fitPage();
}

// A page asks for a new layout whenever its contents change size, such as a
// description that now wraps onto another line.
bool PageScrollArea::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == widget() && event->type() == QEvent::LayoutRequest)
    {
        fitPage();
    }
    return QScrollArea::eventFilter(watched, event);
}

void PageScrollArea::fitPage()
{
    QWidget *page = widget();
    const int width = std::max(viewport()->width(), page->minimumSizeHint().width());
    const int needed = page->hasHeightForWidth() ? page->heightForWidth(width) : page->minimumSizeHint().height();
    page->resize(width, std::max(needed, viewport()->height()));
}
