/** 
 * @file ParameterTrame.hpp
 * @brief 
 * @author The vle Development Team
 * @date 2007-07-15
 */

/*
 * Copyright (C) 2007 - The vle Development Team
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <vle/oov/Trame.hpp>
#include <vle/value/Value.hpp>



namespace vle { namespace oov {

    class ParameterTrame : public Trame
    {
    public:
        ParameterTrame(const std::string& time,
                       const value::Value& data) :
            m_time(time),
            m_data(data)
        { }

        virtual ~ParameterTrame()
        { }

        virtual void print(std::ostream& out) const;
        
        //
        ///
        /// Get/Set functions.
        ///
        //

        inline const std::string& time() const
        { return m_time; }

        inline const value::Value data() const
        { return m_data; }

    private:
        std::string     m_time;
        value::Value    m_data;
    };

}} // namespace vle oov
