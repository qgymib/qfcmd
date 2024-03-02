#include <QHeaderView>
#include <QFileSystemModel>

#include "fstreeview.hpp"

namespace qfcmd {
struct FsTreeViewInner
{
    QFileSystemModel* model;
};
} /* namespace qfcmd */

qfcmd::FsTreeView::FsTreeView(QWidget* parent)
    : QTreeView(parent)
    , m_inner(new qfcmd::FsTreeViewInner)
{
    m_inner->model = new QFileSystemModel;
    m_inner->model->setRootPath(QDir::rootPath());
    m_inner->model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    setModel(m_inner->model);

	/* Hide all columns except the first one. */
    for (int i = 1; i < m_inner->model->columnCount(); i++)
	{
		setColumnHidden(i, true);
	}

	/* Hide the tree view header. */
	header()->hide();
}

qfcmd::FsTreeView::~FsTreeView()
{
}

void qfcmd::FsTreeView::slotChangeDirectory(const QString& path)
{
    QModelIndex index = m_inner->model->index(path);
	expand(index);
	scrollTo(index);
	setCurrentIndex(index);
}
