/**
 * $FILENAME - $TITLE
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2015 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

#include <iostream>

#include "config.h"
#include "file.h"
#include "global_state.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"



/**
 * Accessor for our singleton.
 */
CGlobalState * CGlobalState::instance()
{
    static CGlobalState *instance = new CGlobalState();
    return (instance);
}


/**
 * Constructor
 */
CGlobalState::CGlobalState()
{
    m_current_message =  new CMessage("./Maildir/simple/cur/1445339679.21187_2.ssh.steve.org.uk:2,S");

    m_maildirs = NULL;
    m_messages = NULL;
}


/**
 * Destructor
 */
CGlobalState::~CGlobalState()
{
    if (m_current_message)
        delete(m_current_message);
}


/**
 * Get the currently selected message.
 */
CMessage *CGlobalState::current_message()
{
    return (m_current_message);
}

/**
 * Called when a configuration-key has changed.
 */
void CGlobalState::config_key_changed(std::string name)
{
  if ( (name == "maildir.limit") || ( name == "maildir.prefix" ) )
    {
      update_maildirs();
    }
    else if (name == "index.limit")
    {
      update_messages();
    }
    else
    {
        // NOP.
    }

    /**
     * Get access to our Lua magic.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();

    /**
     * If there is a Config:key_changed() function, then call it.
     */
    lua_getglobal(l, "Config");
    lua_getfield(l, -1, "key_changed");

    if (lua_isnil(l, -1))
        return;

    /**
     * Call the function.
     */
    lua_pushstring(l, name.c_str());
    lua_pcall(l, 1, 0, 0);
}


/**
 * Get all messages from the currently selected folders.
 */
CMessageList * CGlobalState::get_messages()
{
    return (m_messages);
}

std::vector<std::shared_ptr<CMaildir> > CGlobalState::get_maildirs()
{
    CMaildirList display;

    /**
     * If we have no folders then we must return the empty set.
     *
     * Most likely cause?  The maildir_prefix isn't set, or is set incorrectly.
     */
    if (m_maildirs == NULL)
        return (display);


    /**
     * Filter the folders to those we can display
     */
    for (std::shared_ptr<CMaildir> maildir : (*m_maildirs))
    {
        display.push_back(maildir);
    }

    return (display);
}

void CGlobalState::update_maildirs()
{
    /**
     * If we have items already then free each of them.
     */
    if ( m_maildirs != NULL )
    {
        delete( m_maildirs );
        m_maildirs = NULL;
    }
    m_maildirs = new CMaildirList;

    /**
     * Get the maildir prefix.
     */
    CConfig *config = CConfig::instance();
    std::string prefix = config->get_string( "maildir.prefix" );
    if ( prefix.empty() )
      return;

    /**
     * We'll store each maildir here.
     */
    std::vector<std::string> folders;
    folders = CFile::get_all_maildirs(prefix);

    /**
     * TODO: Look at maildir.limit
     */
    for (std::string path : folders)
    {
      m_maildirs->push_back(std::shared_ptr<CMaildir>(new CMaildir(path)));
    }

    /**
     * Setup the size.
     */
    config->set( "maildir.max", std::to_string( m_maildirs->size() ));
}

void CGlobalState::update_messages()
{
}
