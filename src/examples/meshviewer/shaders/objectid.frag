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
#version 430 core
#extension GL_ARB_enhanced_layouts : enable

////////////////////////////////////////////////////////////////////////////////
// Shader Input
////////////////////////////////////////////////////////////////////////////////
layout(location = 0) in VertexData
{
	// ID of the primitive
	flat int PrimitiveId;

} In;

////////////////////////////////////////////////////////////////////////////////
// Shader Output
////////////////////////////////////////////////////////////////////////////////

// x/y: Object ID; Primitive ID
out ivec2 ObjectIds;

////////////////////////////////////////////////////////////////////////////////
// Shader Constants
////////////////////////////////////////////////////////////////////////////////

// Id of the object to render
uniform int ObjectIdx;

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
void main(void)
{
	// Store 0-based IDs
	ObjectIds.x = ObjectIdx;
    ObjectIds.y = In.PrimitiveId;
}
