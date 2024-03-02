#include <QHeaderView>

#include "fstreeview.hpp"

qfcmd::FsTreeView::FsTreeView(QWidget* parent)
	: QTreeView(parent)
{
	m_model = new QFileSystemModel;
	m_model->setRootPath(QDir::rootPath());
	m_model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

	setModel(m_model);

	/* Hide all columns except the first one. */
	for (int i = 1; i < m_model->columnCount(); i++)
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
	QModelIndex index = m_model->index(path);
	expand(index);
	scrollTo(index);
	setCurrentIndex(index);
}
