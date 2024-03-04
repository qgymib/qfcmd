#ifndef QFCMD_FSTABBAR_HPP
#define QFCMD_FSTABBAR_HPP

#include <QTabBar>

namespace qfcmd {

struct FsTabBarInner;

class FsTabBar : public QTabBar
{
    Q_OBJECT
public:
    FsTabBar(QWidget* parent = nullptr);
    virtual ~FsTabBar();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    //void slotButtonClicked();

private:
    FsTabBarInner* m_inner;
};

} /* namespace qfcmd */

#endif // FSTABBAR_HPP
