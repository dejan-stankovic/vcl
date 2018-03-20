# Control CUDA support
SET(VCL_CUDA_SUPPORT CACHE BOOL "Turn off the VCL CUDA support")
IF(VCL_CUDA_SUPPORT)
	FIND_PACKAGE(CUDA)	
	IF(CUDA_FOUND)
		ADD_DEFINITIONS(-DVCL_CUDA_SUPPORT)
	ENDIF(CUDA_FOUND)
ENDIF(VCL_CUDA_SUPPORT)