/*
 * This file is part of the Visual Computing Library (VCL) release under the
 * MIT license.
 *
 * Copyright (c) 2016 Basil Fierz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

// VCL configuration
#include <vcl/config/global.h>

// Qt
#include <QtCore/QObject>
#include <QtCore/QAbstractListModel>

// VCL

namespace Editor
{
	class ComponentAdapter
	{
	public:
		/*!
		 *	\brief Construct a new component adapter with a name
		 *	\param name Name of the component
		 */
		ComponentAdapter(const QString& name);

		QString name() const;

	private:
		//! Name of the entity
		QString _name;
	};

	template<typename T>
	class NamedComponentAdapter : public ComponentAdapter
	{
	public:
		//! Construct a new unnamed component adapter
		NamedComponentAdapter()
		: ComponentAdapter{ typeid(std::decay_t<T>).name() }
		{

		}
	};
}
