/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012-2015 Icinga Development Team (http://www.icinga.org)    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#include "base/dynamicobject.hpp"
#include "base/dictionary.hpp"
#include "base/function.hpp"
#include "base/functionwrapper.hpp"
#include "base/scriptframe.hpp"

using namespace icinga;

static void DynamicObjectModifyAttribute(const String& attr, const Value& value)
{
	ScriptFrame *vframe = ScriptFrame::GetCurrentFrame();
	DynamicObject::Ptr self = vframe->Self;
	return self->ModifyAttribute(attr, value);
}

static void DynamicObjectRestoreAttribute(const String& attr)
{
	ScriptFrame *vframe = ScriptFrame::GetCurrentFrame();
	DynamicObject::Ptr self = vframe->Self;
	return self->RestoreAttribute(attr);
}

Object::Ptr DynamicObject::GetPrototype(void)
{
	static Dictionary::Ptr prototype;

	if (!prototype) {
		prototype = new Dictionary();
		prototype->Set("modify_attribute", new Function(WrapFunction(DynamicObjectModifyAttribute), false));
		prototype->Set("restore_attribute", new Function(WrapFunction(DynamicObjectRestoreAttribute), false));
	}

	return prototype;
}
