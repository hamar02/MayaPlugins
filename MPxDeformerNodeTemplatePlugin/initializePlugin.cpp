#include "MPxDeformerNodeTemplatePlugin.h"
#include <maya/MFnPlugin.h>

// standard initialization procedures
//
MStatus initializePlugin(MObject obj)
{
	MStatus result;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");
	result = plugin.registerNode("offset", MPxDeformerNodeTemplatePlugin::id, MPxDeformerNodeTemplatePlugin::creator,
		MPxDeformerNodeTemplatePlugin::initialize, MPxNode::kDeformerNode);
	MString nodeClassName("offset");
	MString registrantId("mayaPluginExample");
	MGPUDeformerRegistry::registerGPUDeformerCreator(
		nodeClassName,
		registrantId,
		offsetGPUDeformer::getGPUDeformerInfo());
	MGPUDeformerRegistry::addConditionalAttribute(
		nodeClassName,
		registrantId,
		MPxDeformerNode::envelope);
	return result;
}
MStatus uninitializePlugin(MObject obj)
{
	MStatus result;
	MFnPlugin plugin(obj);
	result = plugin.deregisterNode(MPxDeformerNodeTemplatePlugin::id);
	MString nodeClassName("offset");
	MString registrantId("mayaPluginExample");
	MGPUDeformerRegistry::deregisterGPUDeformerCreator(
		nodeClassName,
		registrantId);
	return result;
}