/*
 * This file is part of the Visual Computing Library (VCL) release under the
 * MIT license.
 *
 * Copyright (c) 2015 Basil Fierz
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
#include <vcl/graphics/runtime/opengl/resource/buffer.h>

// C++ standard library

// FMT
#include <fmt/format.h>

// VCL
#include <vcl/core/contract.h>
#include <vcl/graphics/opengl/gl.h>

#ifdef VCL_OPENGL_SUPPORT

#	if defined(VCL_GL_ARB_direct_state_access)
#		define glCreateBuffersVCL glCreateBuffers
#		define glNamedBufferStorageVCL glNamedBufferStorage
#		define glMapNamedBufferRangeVCL glMapNamedBufferRange
#		define glUnmapNamedBufferVCL glUnmapNamedBuffer
#		define glFlushMappedNamedBufferRangeVCL glFlushMappedNamedBufferRange
#		define glCopyNamedBufferSubDataVCL glCopyNamedBufferSubData
#		define glClearNamedBufferDataVCL glClearNamedBufferData
#		define glClearNamedBufferSubDataVCL glClearNamedBufferSubData
#		define glGetNamedBufferSubDataVCL glGetNamedBufferSubData
#	elif defined(VCL_GL_EXT_direct_state_access)
#		define glCreateBuffersVCL glGenBuffers
#		define glNamedBufferStorageVCL glNamedBufferStorageEXT
#		define glMapNamedBufferRangeVCL glMapNamedBufferRangeEXT
#		define glUnmapNamedBufferVCL glUnmapNamedBufferEXT
#		define glFlushMappedNamedBufferRangeVCL glFlushMappedNamedBufferRangeEXT
#		define glCopyNamedBufferSubDataVCL glNamedCopyBufferSubDataEXT
#		define glClearNamedBufferDataVCL glClearNamedBufferDataEXT
#		define glClearNamedBufferSubDataVCL glClearNamedBufferSubDataEXT
#		define glGetNamedBufferSubDataVCL glGetNamedBufferSubDataEXT
#	endif

namespace Vcl { namespace Graphics { namespace Runtime { namespace OpenGL
{
	BufferBindPoint::BufferBindPoint(GLenum target, GLuint id)
	: _target(target)
	, _id(id)
	{
		glBindBuffer(_target, _id);
	}
	BufferBindPoint::~BufferBindPoint()
	{
		glBindBuffer(_target, GL_NONE);
	}

	Buffer::Buffer(const BufferDescription& desc, bool allowPersistentMapping, bool allowCoherentMapping, const BufferInitData* init_data)
	: Runtime::Buffer(desc.SizeInBytes, desc.Usage, desc.CPUAccess)
	, Resource()
	, _allowPersistentMapping(allowPersistentMapping)
	, _allowCoherentMapping(allowCoherentMapping)
	{
		VclRequire(implies(usage() == ResourceUsage::Immutable, cpuAccess().isAnySet() == false), "No CPU access requested for immutable buffer.");
		VclRequire(implies(usage() == ResourceUsage::Dynamic, cpuAccess().isSet(ResourceAccess::Read) == false), "Dynamic buffer is not mapped for reading.");
		VclRequire(implies(init_data, init_data->SizeInBytes == desc.SizeInBytes), "Initialization data has same size as buffer.");
		VclRequire(implies(_allowCoherentMapping, _allowPersistentMapping), "A coherent buffer access is persistent.");
		
		VclRequire(glewIsSupported("GL_ARB_buffer_storage"), "GL buffer storage extension is supported.");
		VclRequire(glewIsSupported("GL_ARB_clear_buffer_object"), "GL clear buffer object extension is supported.");

		GLenum flags = GL_NONE;
		switch (usage())
		{
		case ResourceUsage::Default:
			// Allows to use glBufferSubData
			flags |= GL_DYNAMIC_STORAGE_BIT;
			break;

		case ResourceUsage::Immutable:
			flags |= GL_NONE;
			break;

		case ResourceUsage::Dynamic:
			// Dynamic buffers are always map writable
			flags |= GL_MAP_WRITE_BIT;

			if (_allowPersistentMapping)
				flags |= GL_MAP_PERSISTENT_BIT;

			if (_allowCoherentMapping)
				flags |= GL_MAP_COHERENT_BIT;

			break;

		case ResourceUsage::Staging:
			flags |= GL_DYNAMIC_STORAGE_BIT;

			if (_allowPersistentMapping)
				flags |= GL_MAP_PERSISTENT_BIT;

			if (_allowCoherentMapping)
				flags |= GL_MAP_COHERENT_BIT;

			if (cpuAccess().isSet(ResourceAccess::Write))
				flags |= GL_MAP_WRITE_BIT;

			if (cpuAccess().isSet(ResourceAccess::Read))
				flags |= GL_MAP_READ_BIT;

			break;
		}
		
		// Allocate a GL buffer ID
		glCreateBuffersVCL(1, &_glId);

		// Allocate GPU memory
		const void* init_data_ptr = init_data ? init_data->Data : nullptr;

		glNamedBufferStorageVCL(_glId, desc.SizeInBytes, init_data_ptr, flags);
		
		// Alternative formulation for to prevent performance warnings
		//if (usage() != ResourceUsage::Staging)
		//{
		//	glNamedBufferStorageVCL(_glId, desc.SizeInBytes, init_data_ptr, flags);
		//}
		//else
		//{
		//	// Use old BufferData approach to prevent performance warnings
		//	flags = GL_DYNAMIC_DRAW;
		//	if (cpuAccess().isSet(ResourceAccess::Write))
		//		flags = GL_DYNAMIC_DRAW;
		//	else if (cpuAccess().isSet(ResourceAccess::Read))
		//		flags = GL_DYNAMIC_READ;
		//
		//	glNamedBufferData(_glId, desc.SizeInBytes, init_data_ptr, flags);
		//}
		
		VclEnsure(_glId > 0, "GL buffer is created.");
	}

	Buffer::~Buffer()
	{
		VclRequire(_glId > 0, "GL buffer is created.");

		if (_mappedAccess.isAnySet())
		{
			glUnmapNamedBufferVCL(_glId);

			// Reset the mapped indicator
			_mappedAccess.clear();
			_mappedOptions.clear();
			_mappedOffset = 0;
			_mappedSize = 0;
		}

		if (_glId > 0)
		{
			glDeleteBuffers(1, &_glId);

			// Reset the buffer id
			_glId = 0;
		}

		VclEnsure(_glId == 0, "GL buffer is cleaned up.");
	}

	BufferBindPoint Buffer::bind(GLenum target)
	{
		return{ target, _glId };
	}

	void* Buffer::map(size_t offset, size_t length, Flags<ResourceAccess> access, Flags<MapOptions> options)
	{
		VclRequire(_glId > 0, "GL buffer is created.");
		VclRequire(!_mappedAccess.isAnySet(), "Buffer is not mapped");
		VclRequire(usage() == ResourceUsage::Dynamic || usage() == ResourceUsage::Staging, "GL memory is mappable");
		VclRequire(offset + length <= sizeInBytes(), "Map request lies in range");

		VclRequire(implies(options.isSet(MapOptions::CoherentWrite), _allowCoherentMapping && options.isSet(MapOptions::Persistent)), "Coherent access was configured upon creation");
		VclRequire(implies(options.isSet(MapOptions::Persistent), _allowPersistentMapping), "Persistent mapping was configured upon creation");
		VclRequire(implies(options.isSet(MapOptions::ExplicitFlush), access.isSet(ResourceAccess::Write)), "Explicit flush control only valid when mapped write");
		VclRequire(implies(options.isSet(MapOptions::InvalidateBuffer), !access.isSet(ResourceAccess::Read)), "Invalidation is valid on write");
		VclRequire(implies(options.isSet(MapOptions::InvalidateRange), !access.isSet(ResourceAccess::Read)), "Invalidation is valid on write");

		VclRequire(implies(access.isSet(ResourceAccess::Read), cpuAccess().isSet(ResourceAccess::Read)), "Read access was requested at initialization");
		VclRequire(implies(access.isSet(ResourceAccess::Write), cpuAccess().isSet(ResourceAccess::Write)), "Write access was requested at initialization");

		// Mapped pointer
		void* mappedPtr = nullptr;

		// Map flags
		GLenum map_flags = GL_NONE;
		if (options.isSet(MapOptions::InvalidateBuffer))
			map_flags |= GL_MAP_INVALIDATE_BUFFER_BIT;
		else if (options.isSet(MapOptions::InvalidateRange))
			map_flags |= GL_MAP_INVALIDATE_RANGE_BIT;

		// Persistently map the buffer to the host memory, if usage allows it
		if (usage() == ResourceUsage::Dynamic && access.isSet(ResourceAccess::Write))
		{
			if (options.isSet(MapOptions::Persistent))
				map_flags |= GL_MAP_PERSISTENT_BIT;

			if (options.isSet(MapOptions::CoherentWrite))
				map_flags |= GL_MAP_COHERENT_BIT;
			else if (options.isSet(MapOptions::ExplicitFlush) && access.isSet(ResourceAccess::Write))
				map_flags |= GL_MAP_FLUSH_EXPLICIT_BIT;

			if (access.isSet(ResourceAccess::Write))
				map_flags |= GL_MAP_WRITE_BIT;

			mappedPtr = glMapNamedBufferRangeVCL(_glId, offset, length, map_flags);

			_mappedAccess = access;
			_mappedOptions = options;
			_mappedOffset = offset;
			_mappedSize = length;
		}
		else if (usage() == ResourceUsage::Staging && cpuAccess().isAnySet())
		{
			if (options.isSet(MapOptions::Persistent))
				map_flags |= GL_MAP_PERSISTENT_BIT;

			if (options.isSet(MapOptions::CoherentWrite))
				map_flags |= GL_MAP_COHERENT_BIT;
			else if (options.isSet(MapOptions::ExplicitFlush) && access.isSet(ResourceAccess::Write))
				map_flags |= GL_MAP_FLUSH_EXPLICIT_BIT;

			if (access.isSet(ResourceAccess::Write))
				map_flags |= GL_MAP_WRITE_BIT;

			if (access.isSet(ResourceAccess::Read))
				map_flags |= GL_MAP_READ_BIT;

			mappedPtr = glMapNamedBufferRangeVCL(_glId, offset, length, map_flags);

			_mappedAccess = access;
			_mappedOptions = options;
			_mappedOffset = offset;
			_mappedSize = length;
		}
		
		VclEnsure(implies(usage() == ResourceUsage::Dynamic || usage() == ResourceUsage::Staging, _mappedAccess.isAnySet()), "Buffer is mapped.");
		VclAssertBlock
		{
			GLint64 min_align = 0;
			glGetInteger64v(GL_MIN_MAP_BUFFER_ALIGNMENT, &min_align);
			VclEnsureEx(((ptrdiff_t) mappedPtr - offset) % min_align == 0, "Mapped pointers are aligned correctly.", fmt::format("Offset: {}, Minimum aligment: {}", offset, min_align));
		}

		return mappedPtr;
	}

	void Buffer::flushRange(size_t offset, size_t length)
	{
		VclRequire(_glId > 0, "GL buffer is created.");
		VclRequire(_mappedAccess.isSet(ResourceAccess::Write) && _mappedOptions.isSet(MapOptions::ExplicitFlush), "Buffer is mapped with explicit flush");
		VclRequire(offset + length <= _mappedSize, "Flush request lies in mapped range");

		// If access is not using the coherency flag, we have to 
		// make sure that we are still providing coherent memory access
		glFlushMappedNamedBufferRangeVCL(_glId, offset, length);
	}

	void Buffer::unmap()
	{
		VclRequire(_glId > 0, "GL buffer is created.");
		VclRequire(implies(usage() == ResourceUsage::Dynamic || usage() == ResourceUsage::Staging, _mappedAccess.isAnySet()), "Buffer is mapped.");

		// Mark buffer as unmapped
		_mappedAccess.clear();
		_mappedOptions.clear();
		_mappedOffset = 0;
		_mappedSize = 0;

		// Unmap the buffer
		GLboolean success = glUnmapNamedBufferVCL(_glId);
		if (!success)
		{
			// Memory was lost
			throw gl_memory_error{ "Video memory was trashed" };
		}
		
		VclEnsure(!_mappedAccess.isAnySet(), "Buffer is not mapped.");
	}

	void Buffer::clear()
	{
		VclRequire(_glId > 0, "GL buffer is created.");

		glClearNamedBufferDataVCL(_glId, GL_R8I, GL_RED, GL_BYTE, nullptr);
	}
	
	void Buffer::clear(const Graphics::OpenGL::AnyRenderType& rt, void* data)
	{
		VclRequire(_glId > 0, "GL buffer is created.");

		glClearNamedBufferDataVCL(_glId, rt.internalFormat(), rt.format(), rt.componentType(), data);
	}
	
	void Buffer::clear(size_t offset, size_t size)
	{
		VclRequire(_glId > 0, "GL buffer is created.");
		VclRequire(offset + size <= sizeInBytes(), "Size and the offset lie within the buffer.");
		
		glClearNamedBufferSubDataVCL(_glId, GL_R8I, offset, size, GL_RED, GL_BYTE, nullptr);
	}

	void Buffer::clear(size_t offset, size_t size, const Graphics::OpenGL::AnyRenderType& rt, void* data)
	{
		VclRequire(_glId > 0, "GL buffer is created.");
		VclRequire(offset + size <= sizeInBytes(), "Size and the offset lie within the buffer.");
		
		glClearNamedBufferSubDataVCL(_glId, rt.internalFormat(), offset, size, rt.format(), rt.componentType(), data);
	}

	void Buffer::copyTo(void* dst, size_t srcOffset, size_t dstOffset, size_t size) const
	{
		VclRequire(_glId > 0, "GL buffer is created.");
		VclRequire(implies(size < std::numeric_limits<size_t>::max(), srcOffset + size <= sizeInBytes()), "Size to copy is valid");

		if (size == std::numeric_limits<size_t>::max())
			size = sizeInBytes() - srcOffset;

		glGetNamedBufferSubDataVCL(_glId, srcOffset, size, (char*) dst + dstOffset);
	}

	void Buffer::copyTo(Buffer& target, size_t srcOffset, size_t dstOffset, size_t size) const
	{
		VclRequire(_glId > 0, "GL buffer is created.");
		VclRequire(target.id() > 0, "GL buffer is created.");
		VclRequire(sizeInBytes() <= target.sizeInBytes(), "Size to copy is valid");
		VclRequire(implies(size < std::numeric_limits<size_t>::max(), srcOffset + size <= sizeInBytes()), "Size to copy is valid");
		VclRequire(implies(size < std::numeric_limits<size_t>::max(), dstOffset + size <= target.sizeInBytes()), "Size to copy is valid");

		if (size == std::numeric_limits<size_t>::max())
			size = sizeInBytes() - srcOffset;
		VclCheck(dstOffset + size <= target.sizeInBytes(), "Size to copy is valid");
		
		glCopyNamedBufferSubDataVCL(_glId, target.id(), srcOffset, dstOffset, size);
	}
}}}}
#endif // VCL_OPENGL_SUPPORT
