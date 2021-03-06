#pragma once
#include <maya/MPxGPUDeformer.h>
#include <maya/MGPUDeformerRegistry.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MTypeId.h> 
#include <maya/MStringArray.h>
#include <maya/MDataHandle.h>
#include <maya/MItGeometry.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshFaceVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MMatrix.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MOpenCLInfo.h>
//#include <clew/clew_cl.h>
#include <maya/MFnDependencyNode.h>

#include <vector>

#define EPSILON 0.000000001

struct Vert
{
	int vertexIndex;
	MPoint uv;

};
struct Triangle {
	Vert vertices[3];
};

struct BaryCoordMatch {
	MPoint baryCoord;
	Triangle triangle;
};

class WraPPDeformerNode: public MPxDeformerNode
{
public:
	WraPPDeformerNode();
	~WraPPDeformerNode() override;
	static void* creator();
	static MStatus initialize();
	MStatus compute(const MPlug& plug, MDataBlock& dataBlock) override;

	bool Barycentric(MPoint p, MPoint a, MPoint b, MPoint c, double &u, double &v, double &w);
	bool AreSame(double a, double b);
	MStatus GetTrianglesFromMesh(MObject mesh, std::vector<Triangle> &triangles);
	

	std::vector<std::vector<BaryCoordMatch>> baryCoordMatchedGeometries;
	bool bound = false;
	static MTypeId id;
	static MObject uScale;
private:

};



class WraPPGPUDeformer: public MPxGPUDeformer
{
public:
	WraPPGPUDeformer();
	~WraPPGPUDeformer() override;


	MPxGPUDeformer::DeformerStatus evaluate(MDataBlock & dataBlock, const MEvaluationNode &, const MPlug & plug, unsigned int numElements, const MAutoCLMem inputBuffer, const MAutoCLEvent inputEvent, MAutoCLMem outputBuffer, MAutoCLEvent& outputEvent) override;
	MStatus updateGPU(MDataBlock &dataBlock);

	void terminate() override;

	static MGPUDeformerRegistrationInfo* getGPUDeformerInfo();
	static bool validateNodeInGraph(MDataBlock& dataBlock, const MEvaluationNode&, const MPlug& plug, MStringArray* messages);
	static bool validateNodeValues(MDataBlock& dataBlock, const MEvaluationNode&, const MPlug& plug, MStringArray* messages);
private:
	MAutoCLMem clUScale;
	MAutoCLKernel clKernel;
};


class WraPPGPUDeformerInfo: public MGPUDeformerRegistrationInfo
{
public:
	WraPPGPUDeformerInfo() {};
	~WraPPGPUDeformerInfo() override{};

	MPxGPUDeformer* createGPUDeformer() override
	{
		return new WraPPGPUDeformer();
	}

	bool validateNodeInGraph(MDataBlock& dataBlock, const MEvaluationNode& evaluationNode, const MPlug& plug, MStringArray* messages) override
	{
		return WraPPGPUDeformer::validateNodeInGraph(dataBlock, evaluationNode, plug, messages);
	}
	bool validateNodeValues(MDataBlock& dataBlock, const MEvaluationNode& evaluationNode, const MPlug& plug, MStringArray* messages) override
	{
		return WraPPGPUDeformer::validateNodeValues(dataBlock, evaluationNode, plug, messages);
	}
};
