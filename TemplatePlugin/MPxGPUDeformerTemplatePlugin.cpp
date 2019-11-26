#include "MPxGPUDeformerTemplatePlugin.h"


MTypeId MPxGPUDeformerTemplatePlugin::id(0x80000);
MObject MPxGPUDeformerTemplatePlugin::input;
MObject MPxGPUDeformerTemplatePlugin::output;

MPxGPUDeformerTemplatePlugin::MPxGPUDeformerTemplatePlugin()
{

}

MPxGPUDeformerTemplatePlugin::~MPxGPUDeformerTemplatePlugin()
{

}

MStatus MPxGPUDeformerTemplatePlugin::compute(const MPlug & plug, MDataBlock & data)
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

void * MPxGPUDeformerTemplatePlugin::creator()
{
	return new MPxGPUDeformerTemplatePlugin;
}

MStatus MPxGPUDeformerTemplatePlugin::initialize()
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

