#include "MPxGPUDeformerNodeTemplatePlugin.h"


MTypeId MPxGPUDeformerNodeTemplatePlugin::id(0x80003);
MObject MPxGPUDeformerNodeTemplatePlugin::input;
MObject MPxGPUDeformerNodeTemplatePlugin::output;

MPxGPUDeformerNodeTemplatePlugin::MPxGPUDeformerNodeTemplatePlugin()
{

}

MPxGPUDeformerNodeTemplatePlugin::~MPxGPUDeformerNodeTemplatePlugin()
{

}

MStatus MPxGPUDeformerNodeTemplatePlugin::compute(const MPlug & plug, MDataBlock & data)
{
	MStatus returnStatus;
	VSOutputPrint("Test " << plug.info());

	if (plug == input)
	{
		MDataHandle inputData = data.inputValue(input, &returnStatus);
		if (returnStatus != MS::kSuccess)
			cerr << "ERROR getting data" << endl;
		else
		{
			float result = sin(inputData.asFloat());
			VSOutputPrint("result " << result);

			MDataHandle outputHandle = data.outputValue(output);
			outputHandle.setFloat(result);
			data.setClean(plug);
		}
	}

	return MS::kSuccess;
}

void * MPxGPUDeformerNodeTemplatePlugin::creator()
{
	return new MPxGPUDeformerNodeTemplatePlugin;
}

MStatus MPxGPUDeformerNodeTemplatePlugin::initialize()
{
	MFnNumericAttribute nAttr;
	output = nAttr.create("output", "out", MFnNumericData::kFloat, 0.0);
	nAttr.setWritable(false);
	nAttr.setStorable(false);
	addAttribute(output);

	input = nAttr.create("input", "in", MFnNumericData::kFloat, 0.0);
	nAttr.setStorable(true);
	addAttribute(input);

	attributeAffects(input, output);


	return MS::kSuccess;
}

