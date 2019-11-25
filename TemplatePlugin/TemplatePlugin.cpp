#include "TemplatePlugin.h"


MTypeId TemplatePlugin::id(0x80000);
MObject TemplatePlugin::input;
MObject TemplatePlugin::output;

TemplatePlugin::TemplatePlugin()
{

}

TemplatePlugin::~TemplatePlugin()
{

}

MStatus TemplatePlugin::compute(const MPlug & plug, MDataBlock & data)
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

void * TemplatePlugin::creator()
{
	return new TemplatePlugin;
}

MStatus TemplatePlugin::initialize()
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

