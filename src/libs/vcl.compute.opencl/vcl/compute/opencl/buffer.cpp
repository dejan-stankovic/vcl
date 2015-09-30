/*
 * This file is part of the Visual Computing Library (VCL) release under the
 * MIT license.
 *
 * Copyright (c) 2014-2015 Basil Fierz
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
#include <vcl/compute/opencl/buffer.h>

// VCL 
#include <vcl/core/contract.h>

namespace Vcl { namespace Compute { namespace OpenCL
{
	Buffer::Buffer(Context* ctx, BufferAccess hostAccess, int size)
	: Compute::Buffer(hostAccess, size)
	, _ownerCtx(ctx)
	{
		allocate();
	}

	Buffer::~Buffer()
	{
		free();
	}

	void Buffer::allocate()
	{
		// Allocate the required device memory
		cl_int err;
		_devicePtr = clCreateBuffer(*_ownerCtx, 0, size(), nullptr, &err);
		VCL_CL_SAFE_CALL(err);
	}

	void Buffer::free()
	{
		VCL_CL_SAFE_CALL(clReleaseMemObject(_devicePtr));
	}

	void Buffer::resize(size_t new_size)
	{
		if (_devicePtr)
			free();

		setSize(new_size);

		allocate();
	}
}}}
