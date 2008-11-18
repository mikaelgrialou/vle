/**
 * @file vle/eov/Window.hpp
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2008 The VLE Development Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef VLE_EOV_WINDOW_HPP
#define VLE_EOV_WINDOW_HPP

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <glibmm/timer.h>
#include <vle/eov/Plugin.hpp>

namespace vle { namespace eov {

    class Window : public Gtk::Window
    {
    public:
        Window(Glib::Mutex& mutex, int time);

        virtual ~Window() {}

        /**
         * @brief Assign a plugin to the Window.
         * @param plg the PluginPtr to assign.
         */
        void setPlugin(PluginPtr plg);

        /**
         * @brief Kiill the redraw timer.
         */
        void killClock();

    private:
        /**
         * @brief Check if a plugin was set with the setPlugin function every
         * 200ms. Delete this timer if a plugin was set.
         * @return false to kill the timer.
         */
	bool checkPluginTimer();

        /**
         * @brief Event m_timeout ms, the plugin expose event function was call
         * to redraw the Plugin drawing area widget.
         * @return false to kill the timer.
         */
        bool redrawTimer();

        Glib::Mutex&                m_mutex;
        Gtk::VBox                   m_rootBox;
        PluginPtr                   m_plugin;
	bool                        m_isinit;
        sigc::connection            m_connection;
        unsigned int                m_timeout;
        bool                        m_median;
    };

}} // namespace vle eov

#endif
