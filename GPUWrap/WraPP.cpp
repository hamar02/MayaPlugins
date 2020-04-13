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

	MArrayDataHandle inputArrDH = block.inputValue(input, &status);
	if (status != MS::kSuccess) return status;

	//bind data
	std::vector<Triangle> bindTriangles;


	 status = GetBindData(inputArrDH, bindTriangles);
	 if (status != MS::kSuccess) return status;



	//per object to be deformed, toBeDeformedIndex 0 is bindData
	for (int toBeDeformedIndex=1; toBeDeformedIndex < inputArrDH.elementCount(); ++toBeDeformedIndex)
	{
		inputArrDH.jumpToElement(toBeDeformedIndex);
		MDataHandle inputDH = inputArrDH.inputValue(&status);
		if (status != MS::kSuccess) return status;
		MDataHandle inputGeomDH = inputDH.child(inputGeom);
		MObject mesh = inputGeomDH.asMesh();
	
		std::vector<Triangle> triangles;
		status = GetTrianglesFromMesh(mesh, triangles);
		if (status != MS::kSuccess) return status;

		std::vector<MPoint> baryCoords;
		for (int trianglesIndex=0; trianglesIndex<triangles.size(); ++trianglesIndex)
		{
			for (int vertexIndex =0 ; vertexIndex < 3; ++vertexIndex)
			{
				for (int bindTriangleIndex = 0; bindTriangleIndex < bindTriangles.size(); ++bindTriangleIndex)
				{
					MPoint baryCoord;
					bool success = false;
					success = Barycentric(
						triangles[trianglesIndex].vertices[vertexIndex].uv,
						bindTriangles[bindTriangleIndex].vertices[0].uv,
						bindTriangles[bindTriangleIndex].vertices[1].uv,
						bindTriangles[bindTriangleIndex].vertices[2].uv,
						baryCoord.x,
						baryCoord.y,
						baryCoord.z
					);
					baryCoord.w = success;
					if (success == true)
					{
						baryCoords.push_back(baryCoord);
						continue;
					}
				}
			}
		}

		int gg = 0;
	}


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

bool WraPPDeformerNode::Barycentric(MPoint p, MPoint a, MPoint b, MPoint c, double &u, double &v, double &w)
{
	MVector v0 = b - a;
	MVector v1 = c - a;
	MVector v2 = p - a;
	double d00 = v0 * v0;
	double d01 = v0 * v1;
	double d11 = v1 * v1;
	double d20 = v2 * v0;
	double d21 = v2 * v1;
	double denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0 - v - w;


	if (v > 1.0 == true)
		return false;
	if (w > 1.0 == true)
		return false;
	if (u > 1.0 == true)
		return false;

	if (v < 0.0 == true)
		return false;
	if (w < 0.0 == true)
		return false;
	if (u < 0.0 == true)
		return false;

	if (AreSame(v + w + u, 1.0) == true)
	{
		return true;
	}

	return false;
}

bool WraPPDeformerNode::AreSame(double a, double b)
{
	bool result = fabs(a - b) < EPSILON;
	return result;
}

MStatus WraPPDeformerNode::GetBindData(MArrayDataHandle &inputArrDH, std::vector<Triangle> &bindTriangles)
{
	MStatus status;
	inputArrDH.jumpToElement(0);
	MDataHandle inputDH = inputArrDH.inputValue(&status);
	if (status != MS::kSuccess) return MS::kNotFound;
	MDataHandle inputGeomDH = inputDH.child(inputGeom);
	MObject mesh = inputGeomDH.asMesh();
	status = GetTrianglesFromMesh(mesh, bindTriangles);

	return status;
}

MStatus WraPPDeformerNode::GetTrianglesFromMesh(MObject mesh,  std::vector<Triangle> &triangles)
{
	MStatus status;
	MItMeshPolygon vIter = MItMeshPolygon(mesh, &status);
	if (status != MS::kSuccess) return status;
	//per per polygonal surface
	for (; !vIter.isDone(); vIter.next())
	{
		//per triangle
		Triangle triangle;
		int numTriangles = 0;
		vIter.numTriangles(numTriangles);
		for (int triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex) {
			std::vector<Vert> vertices;

			float2 uv = { 0, 0 };
			MPointArray points;
			MIntArray vertexIndices;
			status = vIter.getTriangle(triangleIndex, points, vertexIndices, MSpace::kObject);
			if (status != MS::kSuccess) return status;

			//per vertex
			for (int vertexIndex = 0; vertexIndex < vertexIndices.length(); ++vertexIndex)
			{
				vIter.getUV(vertexIndices[vertexIndex], uv, &MString("map1"));
				triangle.vertices[vertexIndex].point = points[vertexIndex];

				triangle.vertices[vertexIndex].uv[0] = uv[0];
				triangle.vertices[vertexIndex].uv[1] = uv[1];
			}

			triangles.push_back(triangle);
		}
	}

	return MS::kSuccess;
}

MStatus GetTrianglesFromMesh(MObject mesh, MStatus &status, std::vector<Triangle> &triangles)
{
	MItMeshPolygon vIter = MItMeshPolygon(mesh, &status);
	if (status != MS::kSuccess) return status;
	//per per polygonal surface
	for (; !vIter.isDone(); vIter.next())
	{
		//per triangle
		Triangle triangle;
		int numTriangles = 0;
		vIter.numTriangles(numTriangles);
		for (int triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex) {
			std::vector<Vert> vertices;

			float2 uv = { 0, 0 };
			MPointArray points;
			MIntArray vertexIndices;
			status = vIter.getTriangle(triangleIndex, points, vertexIndices, MSpace::kObject);
			//per vertex
			for (int vertexIndex = 0; vertexIndex < vertexIndices.length(); ++vertexIndex)
			{
				vIter.getUV(vertexIndices[vertexIndex], uv, &MString("map1"));
				triangle.vertices[vertexIndex].point = points[vertexIndex];

				triangle.vertices[vertexIndex].uv[0] = uv[0];
				triangle.vertices[vertexIndex].uv[1] = uv[1];
			}

			triangles.push_back(triangle);
		}
	}
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


