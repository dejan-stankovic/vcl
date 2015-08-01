#include <vcl/math/opencl/jacobisvd33_mcadams.h>

// C++ standard library
#include <array>

// VCL
#include <vcl/compute/opencl/buffer.h>
#include <vcl/compute/opencl/kernel.h>
#include <vcl/compute/opencl/module.h>
#include <vcl/math/ceil.h>

// Kernels
extern uint32_t JacobiSVD33McAdams[];
extern size_t JacobiSVD33McAdamsSize;

namespace Vcl { namespace Mathematics { namespace OpenCL
{
	JacobiSVD33::JacobiSVD33(Core::ref_ptr<Compute::Context> ctx)
	: _ownerCtx(ctx)
	{
		Require(ctx, "Context is valid.");

		_A = ctx->createBuffer(Vcl::Compute::BufferAccess::ReadWrite, 16 * 9 * sizeof(float));
		_U = ctx->createBuffer(Vcl::Compute::BufferAccess::ReadWrite, 16 * 9 * sizeof(float));
		_V = ctx->createBuffer(Vcl::Compute::BufferAccess::ReadWrite, 16 * 9 * sizeof(float));
		_S = ctx->createBuffer(Vcl::Compute::BufferAccess::ReadWrite, 16 * 3 * sizeof(float));

		// Load the module
		_svdModule = ctx->createModuleFromSource(reinterpret_cast<const int8_t*>(JacobiSVD33McAdams), JacobiSVD33McAdamsSize * sizeof(uint32_t));

		if (_svdModule)
		{
			// Load the jacobi SVD kernel
			_svdKernel = _svdModule->kernel("JacobiSVD33McAdams");
		}

		Ensure(implies(_svdModule, _svdKernel), "SVD kernel is valid.");
	}

	void JacobiSVD33::operator()
	(
		const Vcl::Core::InterleavedArray<float, 3, 3, -1>& inA,
		const Vcl::Core::InterleavedArray<float, 3, 3, -1>& outU,
		const Vcl::Core::InterleavedArray<float, 3, 3, -1>& outV,
		const Vcl::Core::InterleavedArray<float, 3, 1, -1>& outS
	)
	{
		Require(_svdKernel, "SVD kernel is loaded.");

		size_t numEntries = inA.size();

		auto& A = Vcl::Core::dynamic_pointer_cast<Compute::OpenCL::Buffer>(_A);
		auto& U = Vcl::Core::dynamic_pointer_cast<Compute::OpenCL::Buffer>(_U);
		auto& V = Vcl::Core::dynamic_pointer_cast<Compute::OpenCL::Buffer>(_V);
		auto& S = Vcl::Core::dynamic_pointer_cast<Compute::OpenCL::Buffer>(_S);

		if (_size < numEntries)
		{
			_size = ceil<16>(numEntries);

			A->resize(_size * 9 * sizeof(float));
			U->resize(_size * 9 * sizeof(float));
			V->resize(_size * 9 * sizeof(float));
			S->resize(_size * 3 * sizeof(float));
		}

		auto& queue = *static_cast<Compute::OpenCL::CommandQueue*>(_ownerCtx->defaultQueue().get());
		queue.write(*A, inA.data());

		// Perform the SVD computation
		std::array<size_t, 3> grid = { ceil<128>(numEntries), 0, 0 };
		std::array<size_t, 3> block = { 128, 0, 0 };
		static_cast<Compute::OpenCL::Kernel*>(_svdKernel.get())->run
		(
			queue,
			1,
			grid,
			block,
			(int) numEntries,
			(int) ceil<16>(numEntries),
			(cl_mem) *A,
			(cl_mem) *U,
			(cl_mem) *V,
			(cl_mem) *S
		);

		queue.read(outU.data(), *U);
		queue.read(outV.data(), *V);
		queue.read(outS.data(), *S);
	}
}}}
