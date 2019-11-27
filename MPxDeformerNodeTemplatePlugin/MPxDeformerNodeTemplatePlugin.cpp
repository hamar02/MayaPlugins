#include "MPxDeformerNodeTemplatePlugin.h"


MTypeId MPxDeformerNodeTemplatePlugin::id(0x80002);
MObject MPxDeformerNodeTemplatePlugin::input;
MObject MPxDeformerNodeTemplatePlugin::output;

MPxDeformerNodeTemplatePlugin::MPxDeformerNodeTemplatePlugin()
{

}

MPxDeformerNodeTemplatePlugin::~MPxDeformerNodeTemplatePlugin()
{

}

MStatus MPxDeformerNodeTemplatePlugin::compute(const MPlug & plug, MDataBlock & data)
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

void * MPxDeformerNodeTemplatePlugin::creator()
{
	return new MPxDeformerNodeTemplatePlugin;
}

MStatus MPxDeformerNodeTemplatePlugin::initialize()
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

