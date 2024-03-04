#include <QPushButton>

#include "fstabbar.hpp"

namespace qfcmd {
struct FsTabBarInner
{
    FsTabBarInner(FsTabBar* parent);
    ~FsTabBarInner();

    FsTabBar*       parent;
    QPushButton*    addBtn;
};
}

qfcmd::FsTabBarInner::FsTabBarInner(FsTabBar *parent)
{
    this->parent = parent;
    addBtn = new QPushButton("+", parent);
}

qfcmd::FsTabBarInner::~FsTabBarInner()
{
}

qfcmd::FsTabBar::FsTabBar(QWidget* parent)
    : QTabBar(parent)
    , m_inner(new FsTabBarInner(this))
{
    m_inner->addBtn->move(3, 2);
}

qfcmd::FsTabBar::~FsTabBar()
{
}

void qfcmd::FsTabBar::resizeEvent(QResizeEvent *event)
{
    QTabBar::resizeEvent(event);

    int x = width() - m_inner->addBtn->width() - 2;
    int y = (height() - m_inner->addBtn->height()) / 2;
    m_inner->addBtn->move(x, y);
}
