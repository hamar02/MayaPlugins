//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+
// 
// DESCRIPTION:
//
// Produces the dependency graph node "offsetNode".
//
// This plug-in demonstrates how to create a user-defined weighted deformer
// with an associated shape. A deformer is a node which takes any number of
// input geometries, deforms them, and places the output into the output
// geometry attribute. This example plug-in defines a new deformer node
// that offsets vertices according to their CV's weights. The weights are set
// using the set editor or the percent command.
//
// To use this node: 
//  - create a plane or some other object
//  - type: "deformer -type offset" 
//  - a locator is created by the command, and you can use this locator
//    to control the direction of the offset. The object's CV's will be offset
//    by the value of the weights of the CV's (the default will be the weight * some constant)
//    in the direction of the y-vector of the locator 
//  - you can edit the weights using either the component editor or by using
//    the percent command (eg. percent -v .5 offset1;) 
//
// Use this script to create a simple example with the offset node:
// 
//  loadPlugin offsetNode;
//  polyTorus -r 1 -sr 0.5 -tw 0 -sx 50 -sy 50 -ax 0 1 0 -cuv 1 -ch 1;
//  deformer -type "offset";
//  setKeyframe -v 0 -at rotateZ -t 1 transform1;
//  setKeyframe -v 180 -at rotateZ -t 60 transform1;
//  select -cl;
//
#include <string.h>
#include <maya/MIOStream.h>
#include <maya/MStringArray.h>
#include <math.h>
#include <maya/MPxDeformerNode.h> 
#include <maya/MItGeometry.h>
#include <maya/MPxLocatorNode.h> 
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MDagModifier.h>
#include <maya/MPxGPUDeformer.h>
#include <maya/MGPUDeformerRegistry.h>
#include <maya/MOpenCLInfo.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFnMesh.h>
#include <clew/clew_cl.h>
#include <vector>

class MPxDeformerNodeTemplatePlugin : public MPxDeformerNode
{
public:
	MPxDeformerNodeTemplatePlugin();
	virtual ~MPxDeformerNodeTemplatePlugin();
	static  void* creator();
	static  MStatus initialize();
	// deformation function
	//
	virtual MStatus deform(MDataBlock& block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex);
	// when the accessory is deleted, this node will clean itself up
	//
	virtual MObject& accessoryAttribute() const;
	// create accessory nodes when the node is created
	//
	virtual MStatus accessoryNodeSetup(MDagModifier& cmd);
public:
	// local node attributes
	static  MObject offsetMatrix;   // offset center and axis
	static  MTypeId id;
private:
};


// the GPU override implementation of the offsetNode
// 
class offsetGPUDeformer : public MPxGPUDeformer
{
public:
	// Virtual methods from MPxGPUDeformer
	offsetGPUDeformer();
	virtual ~offsetGPUDeformer();

	virtual MPxGPUDeformer::DeformerStatus evaluate(MDataBlock& block, const MEvaluationNode&, const MPlug& plug, unsigned int numElements, const MAutoCLMem, const MAutoCLEvent, MAutoCLMem, MAutoCLEvent&);
	virtual void terminate();
	static MGPUDeformerRegistrationInfo* getGPUDeformerInfo();
	static bool validateNodeInGraph(MDataBlock& block, const MEvaluationNode&, const MPlug& plug, MStringArray* messages);
	static bool validateNodeValues(MDataBlock& block, const MEvaluationNode&, const MPlug& plug, MStringArray* messages);
private:
	// helper methods
	void extractWeightArray(MDataBlock& block, const MEvaluationNode& evaluationNode, const MPlug& plug);
	void extractOffsetMatrix(MDataBlock& block, const MEvaluationNode& evaluationNode, const MPlug& plug);
	// Storage for data on the GPU
	MAutoCLMem fCLWeights;
	MAutoCLMem fOffsetMatrix;
	unsigned int fNumElements;
	// Kernel
	MAutoCLKernel fKernel;
};
class offsetNodeGPUDeformerInfo : public MGPUDeformerRegistrationInfo
{
public:
	offsetNodeGPUDeformerInfo() {}
	virtual ~offsetNodeGPUDeformerInfo() {}
	virtual MPxGPUDeformer* createGPUDeformer()
	{
		return new offsetGPUDeformer();
	}

	virtual bool validateNodeInGraph(MDataBlock& block, const MEvaluationNode& evaluationNode, const MPlug& plug, MStringArray* messages)
	{
		return offsetGPUDeformer::validateNodeInGraph(block, evaluationNode, plug, messages);
	}
	virtual bool validateNodeValues(MDataBlock& block, const MEvaluationNode& evaluationNode, const MPlug& plug, MStringArray* messages)
	{
		return offsetGPUDeformer::validateNodeValues(block, evaluationNode, plug, messages);
	}
};