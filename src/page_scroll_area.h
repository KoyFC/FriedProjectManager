#pragma once

#include <QScrollArea>

// Scrolls a page whose wrapped text needs more height the narrower it gets. A
// resizable QScrollArea sizes its page by its minimum size hint, which ignores
// that, so a long page would be squeezed until its widgets overlap.
class PageScrollArea : public QScrollArea
{
public:
    explicit PageScrollArea(QWidget *page);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void fitPage();
};
