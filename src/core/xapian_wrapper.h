#ifndef XAPIAN_WRAPPER_H
#define XAPIAN_WRAPPER_H

// Xapian defines `slots` and `signals` which conflict with Qt.
// We undefine them before including xapian.h, then restore Qt's versions.
#undef slots
#undef signals
#undef emit
#undef foreach

#include <xapian.h>

#define slots Q_SLOTS
#define signals Q_SIGNALS
#define emit Q_EMIT
#define foreach Q_FOREACH

#endif // XAPIAN_WRAPPER_H
