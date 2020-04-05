#include "WraPP.h"
#include <maya/MFnPlugin.h>

MString nodeName("WraPP");
MString registrantId("mayaPluginExample");

MStatus initializePlugin(MObject obj) {
	MStatus result;
	
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");
	result = plugin.registerNode(nodeName, WraPPDeformerNode::id, WraPPDeformerNode::creator, WraPPDeformerNode::initialize, MPxNode::kDeformerNode);

	result = MGPUDeformerRegistry::registerGPUDeformerCreator(
		nodeName,
		registrantId,
		WraPPGPUDeformer::getGPUDeformerInfo());

	return result;

}

MStatus uninitializePlugin(MObject obj)
{
	MStatus result;
	MFnPlugin plugin(obj);
	result = plugin.deregisterNode(WraPPDeformerNode::id);
	MString nodeClassName("offset");
	MString registrantId("mayaPluginExample");
	MGPUDeformerRegistry::deregisterGPUDeformerCreator(
		nodeClassName,
		registrantId);
	return result;
}