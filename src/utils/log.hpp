#ifndef QFCMD_LOG_HPP
#define QFCMD_LOG_HPP

#include <QString>

namespace qfcmd {

class Log
{
public:
    /**
     * @brief Initialize the logger.
     * @param[in] path - log file path.
     */
    static void init(const QString& path = QString());

    /**
     * @brief Exit the logger.
     */
    static void exit();
};

} /* namespace qfcmd */

#endif
