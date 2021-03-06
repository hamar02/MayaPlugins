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

MStatus WraPPDeformerNode::compute(const MPlug& plug, MDataBlock& dataBlock)
{
	std::string varName = std::string(plug.name().asChar());
	MStatus status;

	MDataHandle envDH = dataBlock.inputValue(envelope, &status);
	if (status != MS::kSuccess) return status;
	float env = envDH.asFloat();

	MDataHandle uScaleDH = dataBlock.inputValue(uScale, &status);
	if (status != MS::kSuccess) return status;
	float uniScale = uScaleDH.asFloat();

	MArrayDataHandle inputArrDH = dataBlock.inputValue(input, &status);
	if (status != MS::kSuccess) return status;
	//source data
	if (plug.attribute() == outputGeom) {
		unsigned int index = plug.logicalIndex();
		if (index==0 && bound == false)
		{
			inputArrDH.jumpToElement(index);
			MDataHandle inputDH = inputArrDH.inputValue(&status);
			if (status != MS::kSuccess) return MS::kNotFound;
			MDataHandle inputGeomDH = inputDH.child(inputGeom);
			MObject sourceMesh = inputGeomDH.asMesh();
			std::vector<Triangle> sourceTriangles;
			status = GetTrianglesFromMesh(sourceMesh, sourceTriangles);	 
			if (status != MS::kSuccess) return status;


			//bind data
			//per object to be deformed, geometryIndex 0 is bindData
			for (int geometryIndex=1; geometryIndex < inputArrDH.elementCount(); ++geometryIndex)
			{
				inputArrDH.jumpToElement(geometryIndex);
				MDataHandle inputDH = inputArrDH.inputValue(&status);
				if (status != MS::kSuccess) return status;
				MDataHandle inputGeomDH = inputDH.child(inputGeom);
				MObject targetMesh = inputGeomDH.asMesh();
				MPointArray targetVertices;
				MFnMesh targetMeshFn(targetMesh);
				targetMeshFn.getPoints(targetVertices, MSpace::kObject);

				std::vector<BaryCoordMatch> baryCoords;
				MItMeshVertex iter(targetMesh);
				float2 uv;
				for (;!iter.isDone(); iter.next())
				{
					bool success = false;
				
					for (int bindTriangleIndex = 0; bindTriangleIndex < sourceTriangles.size() && success==false; ++bindTriangleIndex)
					{
						BaryCoordMatch baryCoordMatch;
						iter.getUV(uv);
						MPoint pUV;
						pUV.x = uv[0];
						pUV.y = uv[1];
						success = Barycentric(
							pUV,
							sourceTriangles[bindTriangleIndex].vertices[0].uv,
							sourceTriangles[bindTriangleIndex].vertices[1].uv,
							sourceTriangles[bindTriangleIndex].vertices[2].uv,
							baryCoordMatch.baryCoord.x,
							baryCoordMatch.baryCoord.y,
							baryCoordMatch.baryCoord.z
						);
						baryCoordMatch.baryCoord.w = success;
						if (success == true)
						{
							baryCoordMatch.triangle.vertices[0] = sourceTriangles[bindTriangleIndex].vertices[0];
							baryCoordMatch.triangle.vertices[1] = sourceTriangles[bindTriangleIndex].vertices[1];
							baryCoordMatch.triangle.vertices[2] = sourceTriangles[bindTriangleIndex].vertices[2];

							baryCoords.push_back(baryCoordMatch);
						}
					}
					
				}

				baryCoordMatchedGeometries.push_back(baryCoords);

			}
			bound = true;
		}
	}


	//deform
	inputArrDH.jumpToElement(0);
	MDataHandle inputDH = inputArrDH.inputValue(&status);
	if (status != MS::kSuccess) return MS::kNotFound;
	MDataHandle inputGeomDH = inputDH.child(inputGeom);

	MObject sourceMesh = inputGeomDH.asMesh();

	MPointArray sourceVertices;
	MFnMesh meshFn(sourceMesh);
	meshFn.getPoints(sourceVertices, MSpace::kWorld);
	std::vector<MPoint> points;
	for (int i = 0; i < sourceVertices.length(); ++i) {
		points.push_back(sourceVertices[i]);
	}
	meshFn.getPoints(sourceVertices, MSpace::kObject);
	std::vector<MPoint> wpoints;
	for (int i = 0; i < sourceVertices.length(); ++i) {
		wpoints.push_back(sourceVertices[i]);
	}

	MArrayDataHandle outputGeomArrDH = dataBlock.outputValue(outputGeom, &status);
	if (status != MS::kSuccess) return status;
	for (int geometryIndex = 0; geometryIndex < baryCoordMatchedGeometries.size(); ++geometryIndex)
	{
		std::vector<MPoint> newVertices;
		MPointArray vertices;

		for (int vertexIndex = 0; vertexIndex < baryCoordMatchedGeometries[geometryIndex].size(); ++vertexIndex)
		{
			MPoint baryCoord = baryCoordMatchedGeometries[geometryIndex][vertexIndex].baryCoord;
			int triangleVertexIndex1 = baryCoordMatchedGeometries[geometryIndex][vertexIndex].triangle.vertices[0].vertexIndex;
			int triangleVertexIndex2 = baryCoordMatchedGeometries[geometryIndex][vertexIndex].triangle.vertices[1].vertexIndex;
			int triangleVertexIndex3 = baryCoordMatchedGeometries[geometryIndex][vertexIndex].triangle.vertices[2].vertexIndex;

			MPoint vertex1 = sourceVertices[triangleVertexIndex1];
			MPoint vertex2 = sourceVertices[triangleVertexIndex2];
			MPoint vertex3 = sourceVertices[triangleVertexIndex3];

			MPoint newVertex = baryCoord[0] * vertex1 + baryCoord[1] * vertex2 + baryCoord[2] * vertex3;
			//do more calculations, like normal and distance



			//6 verts, only 4 is unique, setPoints needs 4 unique not 6, map 6 back to 4
			newVertices.push_back(newVertex);
			vertices.append(newVertex);
		}

		inputArrDH.jumpToElement(geometryIndex+1);
		MDataHandle inputDH = inputArrDH.inputValue(&status);
		if (status != MS::kSuccess) return status;

		MDataHandle inputGeomDH = inputDH.child(inputGeom);
		MObject targetMesh = inputGeomDH.asMesh();
		MFnMesh newMeshFn(targetMesh);

		status = newMeshFn.setPoints(vertices, MSpace::kObject);
		if (status != MS::kSuccess) return MS::kNotFound;
		
		status = newMeshFn.setObject(targetMesh);
		if (status != MS::kSuccess) return MS::kNotFound;


		//output
		outputGeomArrDH.jumpToElement(geometryIndex+1);
		if (status != MS::kSuccess) return MS::kNotFound;

		MDataHandle outputGeomDH = outputGeomArrDH.outputValue(&status);
		if (status != MS::kSuccess) return MS::kNotFound;

		status = outputGeomDH.set(targetMesh);
		if (status != MS::kSuccess) return MS::kNotFound;

		int gg = 0;
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


MStatus WraPPDeformerNode::GetTrianglesFromMesh(MObject mesh,  std::vector<Triangle> &triangles)
{
	MFloatArray u;
	MFloatArray v;
	MIntArray triangleCount;
	MIntArray triangleVertexIndices;
	MFnMesh meshFn(mesh);
	meshFn.getTriangles(triangleCount, triangleVertexIndices);
	meshFn.getUVs(u, v, &MString("map1"));


	int vertexIndex = 0;
	for (int faceIndex = 0; faceIndex < triangleCount.length(); ++faceIndex) {
		for (int triangleIndex = 0; triangleIndex < triangleCount[faceIndex]; ++triangleIndex)
		{
			Triangle triangle;

			for (int localVertexIndex = 0; localVertexIndex < 3; ++localVertexIndex)
			{
				int triangleVertexIndex = triangleVertexIndices[vertexIndex];
				triangle.vertices[localVertexIndex].vertexIndex = triangleVertexIndex;
				triangle.vertices[localVertexIndex].uv[0] = u[triangleVertexIndex];
				triangle.vertices[localVertexIndex].uv[1] = v[triangleVertexIndex];
				++vertexIndex;
			}
			triangles.push_back(triangle);

		}
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
	MDataBlock & dataBlock, 
	const MEvaluationNode &, 
	const MPlug & plug, 
	unsigned int numElements, 
	const MAutoCLMem inputBuffer,
	const MAutoCLEvent inputEvent,
	MAutoCLMem outputBuffer,
	MAutoCLEvent& outputEvent)
{
	MStatus status;

	status = updateGPU(dataBlock);
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

MStatus WraPPGPUDeformer::updateGPU(MDataBlock & dataBlock)
{
	MStatus status = MS::kSuccess;
	MDataHandle uScaleDH = dataBlock.inputValue(WraPPDeformerNode::uScale, &status);
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

bool WraPPGPUDeformer::validateNodeInGraph(MDataBlock & dataBlock, const MEvaluationNode &, const MPlug & plug, MStringArray * messages)
{
	return true;
}

bool WraPPGPUDeformer::validateNodeValues(MDataBlock & dataBlock, const MEvaluationNode &, const MPlug & plug, MStringArray * messages)
{
	return true;
}


