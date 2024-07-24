/*++
Copyright (c) 2011 Microsoft Corporation

Module Name:

    scoped_ctrl_c.cpp

Abstract:

    Scoped control-c handler.

Author:

    Leonardo de Moura (leonardo) 2011-04-27.

Revision History:

--*/
#include<signal.h>
#include "util/scoped_ctrl_c.h"

static scoped_ctrl_c * g_obj = nullptr;

static void (*signal_sigint(void (*eh)(int)))(int) {
    void (*old_eh)(int);
    struct sigaction action;
    sigaction(SIGINT, NULL, &action);
    old_eh = action.sa_handler;
    action.sa_handler = eh;
    action.sa_flags |= SA_ONSTACK;
    sigaction(SIGINT, &action, NULL);
    return old_eh;
}

static void on_ctrl_c(int) {
    if (g_obj->m_first) {
        g_obj->m_cancel_eh(CTRL_C_EH_CALLER);
        if (g_obj->m_once) {
            g_obj->m_first = false;
            signal_sigint(on_ctrl_c); // re-install the handler
        }
    }
    else {
        signal_sigint(g_obj->m_old_handler);
        raise(SIGINT);
    }
}

scoped_ctrl_c::scoped_ctrl_c(event_handler & eh, bool once, bool enabled):
    m_cancel_eh(eh),
    m_first(true),
    m_once(once),
    m_enabled(enabled),
    m_old_scoped_ctrl_c(g_obj) {
    if (m_enabled) {
        g_obj = this;
        m_old_handler = signal_sigint(on_ctrl_c);
    }
}

scoped_ctrl_c::~scoped_ctrl_c() {
    if (m_enabled) {
        g_obj = m_old_scoped_ctrl_c;
        if (m_old_handler != SIG_ERR) {
            signal_sigint(m_old_handler);
        }
    }
}
