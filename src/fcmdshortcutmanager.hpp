#ifndef FCMDSHORTCUTMANAGER_HPP
#define FCMDSHORTCUTMANAGER_HPP

#include <QObject>
#include <QAction>
#include <QMap>
#include <QErrorMessage>

class FcmdShortCutManager : public QObject
{
    Q_OBJECT
public:
    explicit FcmdShortCutManager(QWidget *parent);
    virtual ~FcmdShortCutManager();

public:
    /**
     * Creates a new QAction with the given text and connects it to the specified slot function.
     *
     * @param text the text for the QAction
     * @param context the context for the slot function
     * @param slot the slot function to connect to the QAction
     * @param parent the parent object (default is nullptr)
     *
     * @return a pointer to the newly created QAction
     */
    template<typename Func>
    QAction* newAction(const QString& text,
        const typename QtPrivate::ContextTypeForFunctor<Func>::ContextType *context,
        Func &&slot,
        QObject *parent = nullptr)
    {
        QAction* action = new QAction(text, parent);
        return regAction(action, context, slot);
    }

    /**
     * Register an action and connect it to a slot.
     *
     * @param action the QAction to register
     * @param context the context for the slot
     * @param slot the slot to connect to the action's triggered signal
     *
     * @return the registered action
     *
     * @throws N/A
     */
    template<typename Func>
    QAction* regAction(QAction* action,
                        const typename QtPrivate::ContextTypeForFunctor<Func>::ContextType *context,
                        Func &&slot)
    {
        QString text = action->text();
        if (m_actions.contains(text))
        {
            QErrorMessage dialog(m_parent);
            dialog.showMessage("Duplicate shortcuts");
            dialog.exec();
            exit(EXIT_FAILURE);
        }

        m_actions.insert(text, action);

        connect(action, &QAction::triggered, context, slot);
        return action;
    }

    /**
     * Sets a shortcut for the given action.
     *
     * @param text the action text to set the shortcut for, which must registered
     * @param shortcut the shortcut to set
     *
     * @return void
     *
     * @throws None
     */
    void setShortcut(const QString& text, const QKeySequence& shortcut);

signals:

private:
    typedef QMap<QString, QAction*> FcmdActionMap;

private:
    QWidget*                m_parent;   /**< Parent widget. */
    FcmdActionMap           m_actions;  /**< Registered actions. */
};

#endif // FCMDSHORTCUTMANAGER_HPP
