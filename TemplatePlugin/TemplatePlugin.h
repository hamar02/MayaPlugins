#pragma once

#include <string.h>
#include <math.h>
#include <maya/MString.h> 
#include <maya/MPxNode.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include "DebugPrinting.h"


class TemplatePlugin :
	public MPxNode
{
public:
	TemplatePlugin();
	virtual ~TemplatePlugin();


	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	static void* creator();
	static MStatus initialize();

public:
	//For local testing of nodes you can use any identifier between 0x00000000 and 0x0007ffff, 
	//but for any node that you plan to use for more permanent purposes, you should get a universally unique id from:
	//http ://mayaid.autodesk.io/. You will be assigned a unique range that you can manage on your own.
	static MTypeId id;

	//While it is not necessary, it is generally a good idea if the long name is the same as the C++ identifier for the attribute
	static MObject input;
	static MObject output;


};

