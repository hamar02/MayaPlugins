#include "MPxGPUDeformerTemplatePlugin.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, "My plug-in", "1.0", "Any");
	status = plugin.registerNode("MPxGPUDeformerTemplatePlugin", MPxGPUDeformerTemplatePlugin::id, MPxGPUDeformerTemplatePlugin::creator, MPxGPUDeformerTemplatePlugin::initialize);
	return status;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterNode(MPxGPUDeformerTemplatePlugin::id);
	return status;
}