#include "WraPP.h"



MTypeId WraPPDeformerNode::id(0x80002);
MObject WraPPDeformerNode::uScale;

WraPPDeformerNode::WraPPDeformerNode()
{

}

WraPPDeformerNode::~WraPPDeformerNode()
{

}

void* WraPPDeformerNode::creator()
{
	return new WraPPDeformerNode();
}

MStatus WraPPDeformerNode::initialize()
{
	MStatus status;
	MFnNumericAttribute nAttr;

	uScale = nAttr.create("uScale", "us", MFnNumericData::kFloat, 1);
	nAttr.setKeyable(true);
	nAttr.setWritable(true);

	status = addAttribute(uScale);
	if (status != MS::kSuccess)
	{
		status.perror("addAttribute");
		return status;
	}

	return MS::kSuccess;
}

MStatus WraPPDeformerNode::deform(MDataBlock & block, MItGeometry & iter, const MMatrix & mat, unsigned int multiIndex)
{
	MStatus status;

	MDataHandle envDH = block.inputValue(envelope, &status);
	if (status != MS::kSuccess) return status;
	float env = envDH.asFloat();

	MDataHandle uScaleDH = block.inputValue(uScale, &status);
	if (status != MS::kSuccess) return status;
	float uniScale = uScaleDH.asFloat();


	for (; !iter.isDone(); iter.next())
	{
		MPoint pt = iter.position();
		double dMtx[4][4] = { {1,0,0,0},{0,1,0,0},{0,0,1,0},{1,1,1,1} };
		
		MMatrix mtx = MMatrix(dMtx);

		MPoint dPt = pt * mtx * uniScale - pt;
		pt = pt + dPt * env;
		iter.setPosition(pt);
	}

	return MS::kSuccess;
}

WraPPGPUDeformer::WraPPGPUDeformer()
{

}

WraPPGPUDeformer::~WraPPGPUDeformer()
{
	terminate();
}

MPxGPUDeformer::DeformerStatus WraPPGPUDeformer::evaluate(
	MDataBlock & block, 
	const MEvaluationNode &, 
	const MPlug & plug, 
	unsigned int numElements, 
	const MAutoCLMem inputBuffer,
	const MAutoCLEvent inputEvent,
	MAutoCLMem outputBuffer,
	MAutoCLEvent& outputEvent)
{
	MStatus status;

	status = updateGPU(block);
	if (status != MS::kSuccess) return MPxGPUDeformer::kDeformerFailure;

	if (!clKernel.get())
	{
		/*char *maya_location = getenv("MAYA_PLUG_IN_PATH");
		MString openCLKernelFile(maya_location);
		openCLKernelFile += "/../../../clKernels/WraPP.cl";*/
		MString openCLKernelFile("C:/Users/Hampuz/Documents/GitHub/MayaPlugins/clKernels/WraPP.cl");

		MString openCLKernelName("WraPP");
		clKernel = MOpenCLInfo::getOpenCLKernel(openCLKernelFile, openCLKernelName);
		bool gg = clKernel.isNull();
		if (clKernel.isNull()) return MPxGPUDeformer::kDeformerFailure;
	}

	cl_int err = CL_SUCCESS;
	unsigned int parameterId = 0;
	err = clSetKernelArg(clKernel.get(), parameterId++, sizeof(cl_mem), (void*)outputBuffer.getReadOnlyRef());
	MOpenCLInfo::checkCLErrorStatus(err);
	err = clSetKernelArg(clKernel.get(), parameterId++, sizeof(cl_mem), (void*)inputBuffer.getReadOnlyRef());
	MOpenCLInfo::checkCLErrorStatus(err);
	err = clSetKernelArg(clKernel.get(), parameterId++, sizeof(cl_mem), (void*)clUScale.getReadOnlyRef());
	MOpenCLInfo::checkCLErrorStatus(err);
	err = clSetKernelArg(clKernel.get(), parameterId++, sizeof(cl_uint), (void*)&numElements);
	MOpenCLInfo::checkCLErrorStatus(err);

	size_t workGroupSize;
	size_t retSize;
	err = clGetKernelWorkGroupInfo(
		clKernel.get(),
		MOpenCLInfo::getOpenCLDeviceId(),
		CL_KERNEL_WORK_GROUP_SIZE,
		sizeof(size_t),
		&workGroupSize,
		&retSize);
	MOpenCLInfo::checkCLErrorStatus(err);

	size_t localWorkSize = 256;
	if (retSize > 0) localWorkSize = workGroupSize;
	size_t globalWorkSize = (localWorkSize - numElements % localWorkSize) + numElements; // global work size must be a multiple of localWorkSize

	unsigned int numInputEvents = 0;
	if (inputEvent.get())
	{
		numInputEvents = 1;
	}

	// run the kernel
	err = clEnqueueNDRangeKernel(
		MOpenCLInfo::getMayaDefaultOpenCLCommandQueue(),
		clKernel.get(),
		1,
		NULL,
		&globalWorkSize,
		&localWorkSize,
		numInputEvents,
		numInputEvents ? inputEvent.getReadOnlyRef() : 0,
		outputEvent.getReferenceForAssignment());
	
	MOpenCLInfo::checkCLErrorStatus(err);

	return MPxGPUDeformer::kDeformerSuccess;
}

MStatus WraPPGPUDeformer::updateGPU(MDataBlock & block)
{
	MStatus status = MS::kSuccess;
	MDataHandle uScaleDH = block.inputValue(WraPPDeformerNode::uScale, &status);
	if (status != MS::kSuccess) return status;
	float uniScale = uScaleDH.asFloat();

	cl_int err = CL_SUCCESS;
	if (!clUScale.get())
	{
		MHWRender::MRenderer::theRenderer()->holdGPUMemory(sizeof(float));
		clUScale.attach(clCreateBuffer(MOpenCLInfo::getOpenCLContext(), CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, sizeof(float), (void*)&uniScale, &err));
	}
	else
	{
		err = clEnqueueWriteBuffer(MOpenCLInfo::getMayaDefaultOpenCLCommandQueue(), clUScale.get(), CL_TRUE, 0, sizeof(float), (void*)&uniScale, 0, NULL, NULL);
	}

	return status;
}

void WraPPGPUDeformer::terminate()
{
	MHWRender::MRenderer::theRenderer()->releaseGPUMemory(sizeof(float));
	clUScale.reset();
	MOpenCLInfo::releaseOpenCLKernel(clKernel);
	clKernel.reset();
}

MGPUDeformerRegistrationInfo* WraPPGPUDeformer::getGPUDeformerInfo()
{
	static WraPPGPUDeformerInfo theOne;
	return &theOne;
}

bool WraPPGPUDeformer::validateNodeInGraph(MDataBlock & block, const MEvaluationNode &, const MPlug & plug, MStringArray * messages)
{
	return true;
}

bool WraPPGPUDeformer::validateNodeValues(MDataBlock & block, const MEvaluationNode &, const MPlug & plug, MStringArray * messages)
{
	return true;
}


