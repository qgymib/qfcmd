#ifndef QFCMD_CONTAINER_HPP
#define QFCMD_CONTAINER_HPP

#include <QSet>

namespace qfcmd {

/**
 * Find the index of a specified key in the given QSet.
 *
 * @param handle the QSet to search
 * @param key the key to find in the QSet
 * @return the index of the key in the QSet, or -1 if not found
 * @throws None
 */
template<typename T>
qsizetype QSet_IndexOf(const QSet<T>& handle, const T& key)
{
    typename QSet<T>::const_iterator it = handle.constBegin();

    qsizetype idx = 0;
    for (; it != handle.constEnd(); it++, idx++)
    {
        if (*it == key)
        {
            return idx;
        }
    }

    return -1;
}

/**
 * @brief Retrieves the element at the specified index in the given QSet.
 *
 * The index 0 is the first element, index 1 is the second element, and so on.
 *
 * @param handle the QSet from which to retrieve the element
 * @param idx the index of the element to retrieve
 * @return the element at the specified index
 * @throws None
 */
template<typename T>
T QSet_At(const QSet<T>& handle, qsizetype idx)
{
    qsizetype i = 0;
    typename QSet<T>::const_iterator it = handle.constBegin();
    for (; i < idx && it != handle.constEnd(); i++, it++)
    {
    }

    Q_ASSERT(i == idx);
    Q_ASSERT(it != handle.constEnd());

    return *it;
}

} /* namespace qfcmd */

#endif
